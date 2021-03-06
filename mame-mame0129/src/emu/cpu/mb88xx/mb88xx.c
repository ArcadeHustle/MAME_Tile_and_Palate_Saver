/***************************************************************************

    mb88xx.c
    Core implementation for the portable Fujitsu MB88xx series MCU emulator.

    Written by Ernesto Corvi


    TODO:
    - Add support for the timer
    - Add support for the serial interface
    - Split the core to support multiple CPU types?

***************************************************************************/

#include "debugger.h"
#include "mb88xx.h"

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

typedef struct _mb88_state mb88_state;
struct _mb88_state
{
	UINT8	PC; 	/* Program Counter: 6 bits */
	UINT8	PA; 	/* Page Address: 4 bits */
	UINT16	SP[4];	/* Stack is 4*10 bit addresses deep, but we also use 3 top bits per address to store flags during irq */
	UINT8	SI;		/* Stack index: 2 bits */
	UINT8	A;		/* Accumulator: 4 bits */
	UINT8	X;		/* Index X: 4 bits */
	UINT8	Y;		/* Index Y: 4 bits */
	UINT8	st;		/* State flag: 1 bit */
	UINT8	zf;		/* Zero flag: 1 bit */
	UINT8	cf;		/* Carry flag: 1 bit */
	UINT8	vf;		/* Timer overflow flag: 1 bit */
	UINT8	sf;		/* Serial Full/Empty flag: 1 bit */
	UINT8	nf;		/* Interrupt flag: 1 bit */

    /* Peripheral Control */
    UINT8	pio; /* Peripheral enable bits: 8 bits */

    /* Timer registers */
    UINT8	TH;	/* Timer High: 4 bits */
    UINT8	TL;	/* Timer Low: 4 bits */

    /* Serial registers */
    UINT8	SB;	/* Serial buffer: 4 bits */

    /* PLA configuration */
    UINT8 *	PLA;

    /* IRQ handling */
    int	pending_interrupt;
    cpu_irq_callback irqcallback;
    const device_config *device;
    const address_space *program;
    const address_space *data;
    const address_space *io;
    int icount;
};

/***************************************************************************
    MACROS
***************************************************************************/

#define READOP(a) 			(memory_decrypted_read_byte(cpustate->program, a))

#define RDMEM(a)			(memory_read_byte_8be(cpustate->data, a))
#define WRMEM(a,v)			(memory_write_byte_8be(cpustate->data, (a), (v)))

#define READPORT(a)			(memory_read_byte_8be(cpustate->io, a))
#define WRITEPORT(a,v)		(memory_write_byte_8be(cpustate->io, (a), (v)))

#define TEST_ST()			(cpustate->st & 1)
#define TEST_ZF()			(cpustate->zf & 1)
#define TEST_CF()			(cpustate->cf & 1)
#define TEST_VF()			(cpustate->vf & 1)
#define TEST_SF()			(cpustate->sf & 1)
#define TEST_NF()			(cpustate->nf & 1)

#define UPDATE_ST_C(v)		cpustate->st=(v&0x10) ? 0 : 1
#define UPDATE_ST_Z(v)		cpustate->st=(v==0) ? 0 : 1

#define UPDATE_CF(v)		cpustate->cf=((v&0x10)==0) ? 0 : 1
#define UPDATE_ZF(v)		cpustate->zf=(v!=0) ? 0 : 1

#define CYCLES(x)			do { cpustate->icount -= (x); } while (0)

#define GETPC()				(((int)cpustate->PA << 6)+cpustate->PC)
#define GETEA()				((cpustate->X << 4)+cpustate->Y)

#define INCPC()				do { cpustate->PC++; if ( cpustate->PC >= 0x40 ) { cpustate->PC = 0; cpustate->PA++; } } while (0)


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static CPU_INIT( mb88 )
{
	mb88_state *cpustate = device->token;

	if ( device->static_config )
	{
		const mb88_cpu_core *_config = (const mb88_cpu_core*)device->static_config;
		cpustate->PLA = _config->PLA_config;
	}

	cpustate->irqcallback = irqcallback;
	cpustate->device = device;
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	state_save_register_device_item(device, 0, cpustate->PC);
	state_save_register_device_item(device, 0, cpustate->PA);
	state_save_register_device_item(device, 0, cpustate->SP[0]);
	state_save_register_device_item(device, 0, cpustate->SP[1]);
	state_save_register_device_item(device, 0, cpustate->SP[2]);
	state_save_register_device_item(device, 0, cpustate->SP[3]);
	state_save_register_device_item(device, 0, cpustate->SI);
	state_save_register_device_item(device, 0, cpustate->A);
	state_save_register_device_item(device, 0, cpustate->X);
	state_save_register_device_item(device, 0, cpustate->Y);
	state_save_register_device_item(device, 0, cpustate->st);
	state_save_register_device_item(device, 0, cpustate->zf);
	state_save_register_device_item(device, 0, cpustate->cf);
	state_save_register_device_item(device, 0, cpustate->vf);
	state_save_register_device_item(device, 0, cpustate->sf);
	state_save_register_device_item(device, 0, cpustate->nf);
	state_save_register_device_item(device, 0, cpustate->pio);
	state_save_register_device_item(device, 0, cpustate->TH);
	state_save_register_device_item(device, 0, cpustate->TL);
	state_save_register_device_item(device, 0, cpustate->SB);
	state_save_register_device_item(device, 0, cpustate->pending_interrupt);
}

static CPU_RESET( mb88 )
{
	mb88_state *cpustate = device->token;

	/* zero registers and flags */
	cpustate->PC = 0;
	cpustate->PA = 0;
	cpustate->PA = 0;
	cpustate->SP[0] = cpustate->SP[1] = cpustate->SP[2] = cpustate->SP[3] = 0;
	cpustate->SI = 0;
	cpustate->A = 0;
	cpustate->X = 0;
	cpustate->Y = 0;
	cpustate->st = 1;	/* start off with st=1 */
	cpustate->zf = 0;
	cpustate->cf = 0;
	cpustate->vf = 0;
	cpustate->sf = 0;
	cpustate->nf = 0;
	cpustate->pio = 0;
	cpustate->TH = 0;
	cpustate->TL = 0;
	cpustate->SB = 0;
	cpustate->pending_interrupt = 0;
}

/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static int pla( mb88_state *cpustate, int inA, int inB )
{
	int index = ((inB&1) << 4) | (inA&0x0f);

	if ( cpustate->PLA )
		return cpustate->PLA[index];

	return index;
}

static void set_irq_line(mb88_state *cpustate, int state)
{
	/* on falling edge trigger interrupt */
	if ( (cpustate->pio & 0x04) && cpustate->nf && state == CLEAR_LINE )
	{
		cpustate->pending_interrupt = 1;
	}

	cpustate->nf = (state != CLEAR_LINE) ? 1 : 0;
}

static void update_pio( mb88_state *cpustate )
{
	/* update interrupts, serial and timer flags */

	if ( (cpustate->pio & 0x04) && cpustate->pending_interrupt )
	{
		/* no vectors supported, just do the callback to clear irq_state if needed */
		if (cpustate->irqcallback)
			(*cpustate->irqcallback)(cpustate->device, 0);

		cpustate->SP[cpustate->SI] = GETPC();
		cpustate->SP[cpustate->SI] |= TEST_CF() << 15;
		cpustate->SP[cpustate->SI] |= TEST_ZF() << 14;
		cpustate->SP[cpustate->SI] |= TEST_ST() << 13;
		cpustate->SI = ( cpustate->SI + 1 ) & 3;
		cpustate->PC = 0x02;
		cpustate->PA = 0x00;
		cpustate->st = 1;

		cpustate->pending_interrupt = 0;

		CYCLES(3); /* ? */
	}

	/* TODO: add support for serial and timer */
}

static CPU_EXECUTE( mb88 )
{
	mb88_state *cpustate = device->token;

	cpustate->icount = cycles;

	while (cpustate->icount > 0)
	{
		UINT8 opcode, arg, oc;

		/* fetch the opcode */
		debugger_instruction_hook(device, GETPC());
		opcode = READOP(GETPC());

		/* increment the PC */
		INCPC();

		/* start with instruction doing 1 cycle */
		oc = 1;

		switch (opcode)
		{
			case 0x00: /* nop ZCS:...*/
				cpustate->st = 1;
				break;

			case 0x01: /* outO ZCS:...*/
				WRITEPORT( MB88_PORTO, pla(cpustate, cpustate->A,TEST_CF()) );
				cpustate->st = 1;
				break;

			case 0x02: /* outP ZCS:... */
				WRITEPORT( MB88_PORTP, cpustate->A );
				cpustate->st = 1;
				break;

			case 0x03: /* outR ZCS:... */
				arg = cpustate->Y;
				WRITEPORT( MB88_PORTR0+(arg&3), cpustate->A );
				cpustate->st = 1;
				break;

			case 0x04: /* tay ZCS:... */
				cpustate->Y = cpustate->A;
				cpustate->st = 1;
				break;

			case 0x05: /* tath ZCS:... */
				cpustate->TH = cpustate->A;
				cpustate->st = 1;
				break;

			case 0x06: /* tatl ZCS:... */
				cpustate->TL = cpustate->A;
				cpustate->st = 1;
				break;

			case 0x07: /* tas ZCS:... */
				cpustate->SB = cpustate->A;
				cpustate->st = 1;
				break;

			case 0x08: /* icy ZCS:x.x */
				cpustate->Y++;
				UPDATE_ST_C(cpustate->Y);
				cpustate->Y &= 0x0f;
				UPDATE_ZF(cpustate->Y);
				break;

			case 0x09: /* icm ZCS:x.x */
				arg=RDMEM(GETEA());
				arg++;
				UPDATE_ST_C(arg);
				arg &= 0x0f;
				UPDATE_ZF(arg);
				WRMEM(GETEA(),arg);
				break;

			case 0x0a: /* stic ZCS:x.x */
				WRMEM(GETEA(),cpustate->A);
				cpustate->Y++;
				UPDATE_ST_C(cpustate->Y);
				cpustate->Y &= 0x0f;
				UPDATE_ZF(cpustate->Y);
				break;

			case 0x0b: /* x ZCS:x.. */
				arg = RDMEM(GETEA());
				WRMEM(GETEA(),cpustate->A);
				cpustate->A = arg;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x0c: /* rol ZCS:xxx */
				cpustate->A <<= 1;
				cpustate->A |= TEST_CF();
				UPDATE_ST_C(cpustate->A);
				cpustate->cf = cpustate->st ^ 1;
				cpustate->A &= 0x0f;
				UPDATE_ZF(cpustate->A);
				break;

			case 0x0d: /* l ZCS:x.. */
				cpustate->A = RDMEM(GETEA());
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x0e: /* adc ZCS:xxx */
				arg = RDMEM(GETEA());
				arg += cpustate->A;
				arg += TEST_CF();
				UPDATE_ST_C(arg);
				cpustate->cf = cpustate->st ^ 1;
				cpustate->A = arg & 0x0f;
				UPDATE_ZF(cpustate->A);
				break;

			case 0x0f: /* and ZCS:x.x */
				cpustate->A &= RDMEM(GETEA());
				UPDATE_ZF(cpustate->A);
				cpustate->st = cpustate->zf ^ 1;
				break;

			case 0x10: /* daa ZCS:.xx */
				if ( TEST_CF() || cpustate->A > 9 ) cpustate->A += 6;
				UPDATE_ST_C(cpustate->A);
				cpustate->cf = cpustate->st ^ 1;
				cpustate->A &= 0x0f;
				break;

			case 0x11: /* das ZCS:.xx */
				if ( TEST_CF() || cpustate->A > 9 ) cpustate->A += 10;
				UPDATE_ST_C(cpustate->A);
				cpustate->cf = cpustate->st ^ 1;
				cpustate->A &= 0x0f;
				break;

			case 0x12: /* inK ZCS:x.. */
				cpustate->A = READPORT( MB88_PORTK ) & 0x0f;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x13: /* inR ZCS:x.. */
				arg = cpustate->Y;
				cpustate->A = READPORT( MB88_PORTR0+(arg&3) ) & 0x0f;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x14: /* tya ZCS:x.. */
				cpustate->A = cpustate->Y;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x15: /* ttha ZCS:x.. */
				cpustate->A = cpustate->TH;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x16: /* ttla ZCS:x.. */
				cpustate->A = cpustate->TL;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x17: /* tsa ZCS:x.. */
				cpustate->A = cpustate->SB;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x18: /* dcy ZCS:..x */
				cpustate->Y--;
				UPDATE_ST_C(cpustate->Y);
				cpustate->Y &= 0x0f;
				break;

			case 0x19: /* dcm ZCS:x.x */
				arg=RDMEM(GETEA());
				arg--;
				UPDATE_ST_C(arg);
				arg &= 0x0f;
				UPDATE_ZF(arg);
				WRMEM(GETEA(),arg);
				break;

			case 0x1a: /* stdc ZCS:x.x */
				WRMEM(GETEA(),cpustate->A);
				cpustate->Y--;
				UPDATE_ST_C(cpustate->Y);
				cpustate->Y &= 0x0f;
				UPDATE_ZF(cpustate->Y);
				break;

			case 0x1b: /* xx ZCS:x.. */
				arg = cpustate->X;
				cpustate->X = cpustate->A;
				cpustate->A = arg;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x1c: /* ror ZCS:xxx */
				cpustate->A |= TEST_CF() << 4;
				UPDATE_ST_C(cpustate->A << 4);
				cpustate->cf = cpustate->st ^ 1;
				cpustate->A >>= 1;
				cpustate->A &= 0x0f;
				UPDATE_ZF(cpustate->A);
				break;

			case 0x1d: /* st ZCS:x.. */
				WRMEM(GETEA(),cpustate->A);
				cpustate->st = 1;
				break;

			case 0x1e: /* sbc ZCS:xxx */
				arg = RDMEM(GETEA());
				arg -= cpustate->A;
				arg -= TEST_CF();
				UPDATE_ST_C(arg);
				cpustate->cf = cpustate->st ^ 1;
				cpustate->A = arg & 0x0f;
				UPDATE_ZF(cpustate->A);
				break;

			case 0x1f: /* or ZCS:x.x */
				cpustate->A |= RDMEM(GETEA());
				UPDATE_ZF(cpustate->A);
				cpustate->st = cpustate->zf ^ 1;
				break;

			case 0x20: /* setR ZCS:... */
				arg = READPORT( MB88_PORTR0+(cpustate->Y/4) );
				WRITEPORT( MB88_PORTR0+(cpustate->Y/4), arg | ( 1 << (cpustate->Y%4) ) );
				cpustate->st = 1;
				break;

			case 0x21: /* setc ZCS:.xx */
				cpustate->cf = 1;
				cpustate->st = 1;
				break;

			case 0x22: /* rstR ZCS:... */
				arg = READPORT( MB88_PORTR0+(cpustate->Y/4) );
				WRITEPORT( MB88_PORTR0+(cpustate->Y/4), arg & ~( 1 << (cpustate->Y%4) ) );
				cpustate->st = 1;
				break;

			case 0x23: /* rstc ZCS:.xx */
				cpustate->cf = 0;
				cpustate->st = 1;
				break;

			case 0x24: /* tstr ZCS:..x */
				arg = READPORT( MB88_PORTR0+(cpustate->Y/4) );
				cpustate->st = ( arg & ( 1 << (cpustate->Y%4) ) ) ? 1 : 0;
				break;

			case 0x25: /* tsti ZCS:..x */
				cpustate->st = cpustate->nf ^ 1;
				break;

			case 0x26: /* tstv ZCS:..x */
				cpustate->st = cpustate->vf ^ 1;
				break;

			case 0x27: /* tsts ZCS:..x */
				cpustate->st = cpustate->sf ^ 1;
				break;

			case 0x28: /* tstc ZCS:..x */
				cpustate->st = cpustate->cf ^ 1;
				break;

			case 0x29: /* tstz ZCS:..x */
				cpustate->st = cpustate->zf ^ 1;
				break;

			case 0x2a: /* sts ZCS:x.. */
				WRMEM(GETEA(),cpustate->SB);
				UPDATE_ZF(cpustate->SB);
				cpustate->st = 1;
				break;

			case 0x2b: /* ls ZCS:x.. */
				cpustate->SB = RDMEM(GETEA());
				UPDATE_ZF(cpustate->SB);
				cpustate->st = 1;
				break;

			case 0x2c: /* rts ZCS:... */
				cpustate->SI = ( cpustate->SI - 1 ) & 3;
				cpustate->PC = cpustate->SP[cpustate->SI] & 0x3f;
				cpustate->PA = cpustate->SP[cpustate->SI] >> 6;
				cpustate->st = 1;
				break;

			case 0x2d: /* neg ZCS: ..x */
				cpustate->A = (~cpustate->A)+1;
				cpustate->A &= 0x0f;
				UPDATE_ST_Z(cpustate->A);
				break;

			case 0x2e: /* c ZCS:xxx */
				arg = RDMEM(GETEA());
				arg -= cpustate->A;
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				cpustate->zf = cpustate->st ^ 1;
				break;

			case 0x2f: /* eor ZCS:x.x */
				cpustate->A ^= RDMEM(GETEA());
				UPDATE_ST_Z(cpustate->A);
				cpustate->zf = cpustate->st ^ 1;
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: /* sbit ZCS:... */
				arg = RDMEM(GETEA());
				WRMEM(GETEA(), arg | (1 << (opcode&3)));
				cpustate->st = 1;
				break;

			case 0x34: case 0x35: case 0x36: case 0x37: /* rbit ZCS:... */
				arg = RDMEM(GETEA());
				WRMEM(GETEA(), arg & ~(1 << (opcode&3)));
				cpustate->st = 1;
				break;

			case 0x38: case 0x39: case 0x3a: case 0x3b: /* tbit ZCS:... */
				arg = RDMEM(GETEA());
				cpustate->st = ( arg & (1 << (opcode&3) ) ) ? 1 : 0;
				break;

			case 0x3c: /* rti ZCS:... */
				/* restore address and saved state flags on the top bits of the stack */
				cpustate->SI = ( cpustate->SI - 1 ) & 3;
				cpustate->PC = cpustate->SP[cpustate->SI] & 0x3f;
				cpustate->PA = cpustate->SP[cpustate->SI] >> 6;
				cpustate->st = (cpustate->SP[cpustate->SI] >> 13)&1;
				cpustate->zf = (cpustate->SP[cpustate->SI] >> 14)&1;
				cpustate->cf = (cpustate->SP[cpustate->SI] >> 15)&1;
				break;

			case 0x3d: /* jpa imm ZCS:..x */
				cpustate->PA = READOP(GETPC()) & 0x1f;
				cpustate->PC = cpustate->A * 4;
				oc = 2;
				cpustate->st = 1;
				break;

			case 0x3e: /* en imm ZCS:... */
				cpustate->pio |= READOP(GETPC());
				INCPC();
				oc = 2;
				cpustate->st = 1;
				break;

			case 0x3f: /* dis imm ZCS:... */
				cpustate->pio &= ~(READOP(GETPC()));
				INCPC();
				oc = 2;
				cpustate->st = 1;
				break;

			case 0x40:	case 0x41:	case 0x42:	case 0x43: /* setD ZCS:... */
				arg = READPORT(MB88_PORTR0);
				arg |= (1 << (opcode&3));
				WRITEPORT(MB88_PORTR0,arg);
				cpustate->st = 1;
				break;

			case 0x44:	case 0x45:	case 0x46:	case 0x47: /* rstD ZCS:... */
				arg = READPORT(MB88_PORTR0);
				arg &= ~(1 << (opcode&3));
				WRITEPORT(MB88_PORTR0,arg);
				cpustate->st = 1;
				break;

			case 0x48:	case 0x49:	case 0x4a:	case 0x4b: /* tstD ZCS:..x */
				arg = READPORT(MB88_PORTR2);
				cpustate->st = (arg & (1 << (opcode&3))) ? 1 : 0;
				break;

			case 0x4c:	case 0x4d:	case 0x4e:	case 0x4f: /* tba ZCS:..x */
				cpustate->st = (cpustate->A & (1 << (opcode&3))) ? 1 : 0;
				break;

			case 0x50:	case 0x51:	case 0x52:	case 0x53: /* xd ZCS:x.. */
				arg = RDMEM(opcode&3);
				WRMEM((opcode&3),cpustate->A);
				cpustate->A = arg;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0x54:	case 0x55:	case 0x56:	case 0x57: /* xyd ZCS:x.. */
				arg = RDMEM((opcode&3)+4);
				WRMEM((opcode&3)+4,cpustate->Y);
				cpustate->Y = arg;
				UPDATE_ZF(cpustate->Y);
				cpustate->st = 1;
				break;

			case 0x58:	case 0x59:	case 0x5a:	case 0x5b:
			case 0x5c:	case 0x5d:	case 0x5e:	case 0x5f: /* lxi ZCS:x.. */
				cpustate->X = opcode & 7;
				UPDATE_ZF(cpustate->X);
				cpustate->st = 1;
				break;

			case 0x60:	case 0x61:	case 0x62:	case 0x63:
			case 0x64:	case 0x65:	case 0x66:	case 0x67: /* call imm ZCS:..x */
				arg = READOP(GETPC());
				INCPC();
				oc = 2;
				if ( TEST_ST() )
				{
					cpustate->SP[cpustate->SI] = GETPC();
					cpustate->SI = ( cpustate->SI + 1 ) & 3;
					cpustate->PC = arg & 0x3f;
					cpustate->PA = ( ( opcode & 7 ) << 2 ) | ( arg >> 6 );
				}
				cpustate->st = 1;
				break;

			case 0x68:	case 0x69:	case 0x6a:	case 0x6b:
			case 0x6c:	case 0x6d:	case 0x6e:	case 0x6f: /* jpl imm ZCS:..x */
				arg = READOP(GETPC());
				INCPC();
				oc = 2;
				if ( TEST_ST() )
				{
					cpustate->PC = arg & 0x3f;
					cpustate->PA = ( ( opcode & 7 ) << 2 ) | ( arg >> 6 );
				}
				cpustate->st = 1;
				break;

			case 0x70:	case 0x71:	case 0x72:	case 0x73:
			case 0x74:	case 0x75:	case 0x76:	case 0x77:
			case 0x78:	case 0x79:	case 0x7a:	case 0x7b:
			case 0x7c:	case 0x7d:	case 0x7e:	case 0x7f: /* ai ZCS:xxx */
				arg = opcode & 0x0f;
				arg += cpustate->A;
				UPDATE_ST_C(arg);
				cpustate->cf = cpustate->st ^ 1;
				cpustate->A = arg & 0x0f;
				UPDATE_ZF(cpustate->A);
				break;

			case 0x80:	case 0x81:	case 0x82:	case 0x83:
			case 0x84:	case 0x85:	case 0x86:	case 0x87:
			case 0x88:	case 0x89:	case 0x8a:	case 0x8b:
			case 0x8c:	case 0x8d:	case 0x8e:	case 0x8f: /* lxi ZCS:x.. */
				cpustate->Y = opcode & 0x0f;
				UPDATE_ZF(cpustate->Y);
				cpustate->st = 1;
				break;

			case 0x90:	case 0x91:	case 0x92:	case 0x93:
			case 0x94:	case 0x95:	case 0x96:	case 0x97:
			case 0x98:	case 0x99:	case 0x9a:	case 0x9b:
			case 0x9c:	case 0x9d:	case 0x9e:	case 0x9f: /* li ZCS:x.. */
				cpustate->A = opcode & 0x0f;
				UPDATE_ZF(cpustate->A);
				cpustate->st = 1;
				break;

			case 0xa0:	case 0xa1:	case 0xa2:	case 0xa3:
			case 0xa4:	case 0xa5:	case 0xa6:	case 0xa7:
			case 0xa8:	case 0xa9:	case 0xaa:	case 0xab:
			case 0xac:	case 0xad:	case 0xae:	case 0xaf: /* cyi ZCS:xxx */
				arg = cpustate->Y - (opcode & 0x0f);
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				cpustate->zf = cpustate->st ^ 1;
				break;

			case 0xb0:	case 0xb1:	case 0xb2:	case 0xb3:
			case 0xb4:	case 0xb5:	case 0xb6:	case 0xb7:
			case 0xb8:	case 0xb9:	case 0xba:	case 0xbb:
			case 0xbc:	case 0xbd:	case 0xbe:	case 0xbf: /* ci ZCS:xxx */
				arg = cpustate->A - (opcode & 0x0f);
				UPDATE_CF(arg);
				arg &= 0x0f;
				UPDATE_ST_Z(arg);
				cpustate->zf = cpustate->st ^ 1;
				break;

			default: /* jmp ZCS:..x */
				if ( TEST_ST() )
				{
					cpustate->PC = opcode & 0x3f;
				}
				cpustate->st = 1;
				break;
		}

		/* update cycle counts */
		CYCLES( oc );

		/* update interrupts, serial and timer flags */
		update_pio(cpustate);
	}

	return cycles - cpustate->icount;
}

/***************************************************************************
    INFORMATION SETTERS
***************************************************************************/

static CPU_SET_INFO( mb88 )
{
	mb88_state *cpustate = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + MB88_IRQ_LINE:	set_irq_line(cpustate, info->i);					break;

		case CPUINFO_INT_PC:
				cpustate->PC = info->i & 0x3f;
				cpustate->PA = (info->i >> 6) & 0x1f;
				break;
		case CPUINFO_INT_REGISTER + MB88_PC:			cpustate->PC = info->i;						break;
		case CPUINFO_INT_REGISTER + MB88_PA:			cpustate->PA = info->i;						break;
		case CPUINFO_INT_REGISTER + MB88_FLAGS:
				cpustate->st = (info->i & 0x01) ? 1 : 0;
				cpustate->zf = (info->i & 0x02) ? 1 : 0;
				cpustate->cf = (info->i & 0x04) ? 1 : 0;
				cpustate->vf = (info->i & 0x08) ? 1 : 0;
				cpustate->sf = (info->i & 0x10) ? 1 : 0;
				cpustate->nf = (info->i & 0x20) ? 1 : 0;
				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MB88_SI:			cpustate->SI = info->i & 0x03;				break;
		case CPUINFO_INT_REGISTER + MB88_A:				cpustate->A = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_X:				cpustate->X = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_Y:				cpustate->Y = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_PIO:			cpustate->pio = info->i & 0xff;				break;
		case CPUINFO_INT_REGISTER + MB88_TH:			cpustate->TH = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_TL:			cpustate->TL = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + MB88_SB:			cpustate->SB = info->i & 0x0f;				break;
	}
}

/***************************************************************************
    INFORMATION GETTERS
***************************************************************************/

CPU_GET_INFO( mb88 )
{
	mb88_state *cpustate = (device != NULL) ? device->token : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(mb88_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 3;							break;

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 11;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 7;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 		info->i = 3;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + MB88_IRQ_LINE:	info->i = cpustate->pending_interrupt ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:							info->i = GETPC();						break;
		case CPUINFO_INT_REGISTER + MB88_PC: 			info->i = cpustate->PC;						break;
		case CPUINFO_INT_REGISTER + MB88_PA: 			info->i = cpustate->PA;						break;
		case CPUINFO_INT_REGISTER + MB88_FLAGS:			info->i = 0;
				if (TEST_ST()) info->i |= 0x01;
				if (TEST_ZF()) info->i |= 0x02;
				if (TEST_CF()) info->i |= 0x04;
				if (TEST_VF()) info->i |= 0x08;
				if (TEST_SF()) info->i |= 0x10;
				if (TEST_NF()) info->i |= 0x20;
				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MB88_SI: 			info->i = cpustate->SI;						break;
		case CPUINFO_INT_REGISTER + MB88_A: 			info->i = cpustate->A;						break;
		case CPUINFO_INT_REGISTER + MB88_X: 			info->i = cpustate->X;						break;
		case CPUINFO_INT_REGISTER + MB88_Y: 			info->i = cpustate->Y;						break;
		case CPUINFO_INT_REGISTER + MB88_PIO: 			info->i = cpustate->pio;						break;
		case CPUINFO_INT_REGISTER + MB88_TH: 			info->i = cpustate->TH;						break;
		case CPUINFO_INT_REGISTER + MB88_TL: 			info->i = cpustate->TL;						break;
		case CPUINFO_INT_REGISTER + MB88_SB: 			info->i = cpustate->SB;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(mb88);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(mb88);					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(mb88);				break;
		case CPUINFO_FCT_EXIT:							info->exit = NULL;						break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(mb88);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(mb88);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB88xx");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Fujitsu MB88xx");	break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Ernesto Corvi"); break;

		case CPUINFO_STR_FLAGS:
    		sprintf(info->s, "%c%c%c%c%c%c",
	        		TEST_ST() ? 'T' : 't',
	        		TEST_ZF() ? 'Z' : 'z',
	        		TEST_CF() ? 'C' : 'c',
	        		TEST_VF() ? 'V' : 'v',
	        		TEST_SF() ? 'S' : 's',
	        		TEST_NF() ? 'I' : 'i');
	        break;

        case CPUINFO_STR_REGISTER + MB88_FLAGS:
    		sprintf(info->s, "FL:%c%c%c%c%c%c",
	        		TEST_ST() ? 'T' : 't',
	        		TEST_ZF() ? 'Z' : 'z',
	        		TEST_CF() ? 'C' : 'c',
	        		TEST_VF() ? 'V' : 'v',
	        		TEST_SF() ? 'S' : 's',
	        		TEST_NF() ? 'I' : 'i');
	        break;

        case CPUINFO_STR_REGISTER + MB88_PC:			sprintf(info->s, "PC:%02X", cpustate->PC);	break;
        case CPUINFO_STR_REGISTER + MB88_PA:			sprintf(info->s, "PA:%02X", cpustate->PA);	break;
        case CPUINFO_STR_REGISTER + MB88_SI:			sprintf(info->s, "SI:%1X", cpustate->SI);	break;
		case CPUINFO_STR_REGISTER + MB88_A:				sprintf(info->s, "A:%1X", cpustate->A);		break;
		case CPUINFO_STR_REGISTER + MB88_X:				sprintf(info->s, "X:%1X", cpustate->X);		break;
		case CPUINFO_STR_REGISTER + MB88_Y:				sprintf(info->s, "Y:%1X", cpustate->Y);		break;
		case CPUINFO_STR_REGISTER + MB88_PIO:			sprintf(info->s, "PIO:%02X", cpustate->pio);	break;
        case CPUINFO_STR_REGISTER + MB88_TH:			sprintf(info->s, "TH:%1X", cpustate->TH);	break;
		case CPUINFO_STR_REGISTER + MB88_TL:			sprintf(info->s, "TL:%1X", cpustate->TL);	break;
		case CPUINFO_STR_REGISTER + MB88_SB:			sprintf(info->s, "SB:%1X", cpustate->SB);	break;
	}
}

CPU_GET_INFO( mb8841 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 11;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 7;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB8841");				break;

		default:										CPU_GET_INFO_CALL(mb88);			break;
	}
}

CPU_GET_INFO( mb8842 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 11;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 7;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB8842");				break;

		default:										CPU_GET_INFO_CALL(mb88);			break;
	}
}

CPU_GET_INFO( mb8843 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 10;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 6;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB8843");				break;

		default:										CPU_GET_INFO_CALL(mb88);			break;
	}
}

CPU_GET_INFO( mb8844 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 10;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 6;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MB8844");				break;

		default:										CPU_GET_INFO_CALL(mb88);			break;
	}
}
