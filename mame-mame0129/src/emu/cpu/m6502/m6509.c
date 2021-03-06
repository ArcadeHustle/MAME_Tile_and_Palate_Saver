/*****************************************************************************
 *
 *   m6509.c
 *   Portable 6509 emulator V1.0beta1
 *
 *   Copyright Peter Trauner, all rights reserved.
 *   documentation by vice emulator team
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
/*
  2000 March 10 PeT added SO input line

The basic difference is the amount of RAM these machines have been supplied with. The B128 and the CBM *10
models had 128k RAM, the others 256k. This implies some banking scheme, as the 6502 can only address 64k. And
indeed those machines use a 6509, that can address 1 MByte of RAM. It has 2 registers at addresses 0 and 1. The
indirect bank register at address 1 determines the bank (0-15) where the opcodes LDA (zp),Y and STA (zp),Y
take the data from. The exec bank register at address 0 determines the bank where all other read and write
addresses take place.

 vice writes to bank register only with zeropage operand
 0, 1 are bank register in all banks

 lda  (zp),y
 sta  (zp),y

*/

#include "debugger.h"
#include "m6509.h"

#include "ops02.h"
#include "ill02.h"
#include "ops09.h"

#define M6502_NMI_VEC	0xfffa
#define M6502_RST_VEC	0xfffc
#define M6502_IRQ_VEC	0xfffe
#define M6509_RST_VEC	M6502_RST_VEC
#define M6509_IRQ_VEC	M6502_IRQ_VEC
#define M6509_NMI_VEC	M6502_NMI_VEC

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

typedef struct _m6509_Regs m6509_Regs;
struct _m6509_Regs {
	UINT8	subtype;		/* currently selected cpu sub type */
	void	(*const *insn)(m6509_Regs *); /* pointer to the function pointer table */
	PAIR	ppc;			/* previous program counter */
	/* pc.w.h contains the current page pc_bank.w.h for better speed */
	PAIR	pc; 			/* program counter */
	PAIR	sp; 			/* stack pointer (always 100 - 1FF) */
	PAIR	zp; 			/* zero page address */
	PAIR	ea; 			/* effective address */
	UINT8	a;				/* Accumulator */
	UINT8	x;				/* X index register */
	UINT8	y;				/* Y index register */
	PAIR   pc_bank; 	   /* 4 bits, addressed over address 0 */
	PAIR   ind_bank;	   /* 4 bits, addressed over address 1 */
	UINT8	p;				/* Processor status */
	UINT8	pending_irq;	/* nonzero if an IRQ is pending */
	UINT8	after_cli;		/* pending IRQ and last insn cleared I */
	UINT8	nmi_state;
	UINT8	irq_state;
	UINT8	so_state;
	cpu_irq_callback irq_callback;
	const device_config *device;
	const address_space *space;

	int 	icount;

	m6502_read_indexed_func rdmem_id;					/* readmem callback for indexed instructions */
	m6502_write_indexed_func wrmem_id;					/* writemem callback for indexed instructions */
};


/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/

#include "t6509.c"

static READ8_HANDLER( m6509_read_00000 )
{
	m6509_Regs *cpustate = (m6509_Regs *)space->cpu->token;

	return cpustate->pc_bank.b.h2;
}

static READ8_HANDLER( m6509_read_00001 )
{
	m6509_Regs *cpustate = (m6509_Regs *)space->cpu->token;

	return cpustate->ind_bank.b.h2;
}

static WRITE8_HANDLER( m6509_write_00000 )
{
	m6509_Regs *cpustate = (m6509_Regs *)space->cpu->token;

	cpustate->pc_bank.b.h2=data&0xf;
	cpustate->pc.w.h=cpustate->pc_bank.w.h;
}

static WRITE8_HANDLER( m6509_write_00001 )
{
	m6509_Regs *cpustate = (m6509_Regs *)space->cpu->token;

	cpustate->ind_bank.b.h2=data&0xf;
}

static ADDRESS_MAP_START(m6509_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00000, 0x00000) AM_MIRROR(0xF0000) AM_READWRITE(m6509_read_00000, m6509_write_00000)
	AM_RANGE(0x00001, 0x00001) AM_MIRROR(0xF0000) AM_READWRITE(m6509_read_00001, m6509_write_00001)
ADDRESS_MAP_END

static UINT8 default_rdmem_id(const address_space *space, offs_t address) { return memory_read_byte_8le(space, address); }
static void default_wdmem_id(const address_space *space, offs_t address, UINT8 data) { memory_write_byte_8le(space, address, data); }

static CPU_INIT( m6509 )
{
	m6509_Regs *cpustate = device->token;

	cpustate->rdmem_id = default_rdmem_id;
	cpustate->wrmem_id = default_wdmem_id;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->space = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
}

static CPU_RESET( m6509 )
{
	m6509_Regs *cpustate = device->token;

	cpustate->insn = insn6509;

	cpustate->pc_bank.d=cpustate->ind_bank.d=0;
	cpustate->pc_bank.b.h2=cpustate->ind_bank.b.h2=0xf; /* cbm500 needs this */
	cpustate->pc.w.h=cpustate->pc_bank.w.h;
	/* wipe out the rest of the m6509 structure */
	/* read the reset vector into PC */
	PCL = RDMEM(M6509_RST_VEC|PB);
	PCH = RDMEM((M6509_RST_VEC+1)|PB);

	cpustate->sp.d = 0x01ff;
	cpustate->p = F_T|F_B|F_I|F_Z|(P&F_D);	/* set T, I and Z flags */
	cpustate->pending_irq = 0;	/* nonzero if an IRQ is pending */
	cpustate->after_cli = 0;	/* pending IRQ and last insn cleared I */
	cpustate->irq_callback = NULL;
}

static CPU_EXIT( m6509 )
{
	/* nothing to do yet */
}

INLINE void m6509_take_irq(	m6509_Regs *cpustate)
{

	if( !(P & F_I) )
	{
		EAD = M6509_IRQ_VEC;
		EAWH = PBWH;
		cpustate->icount -= 2;
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;		/* knock out D and set I flag */
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD+1);
		LOG(("M6509 '%s' takes IRQ ($%04x)\n", cpustate->device->tag, PCD));
		/* call back the cpuintrf to let it clear the line */
		if (cpustate->irq_callback) (*cpustate->irq_callback)(cpustate->device, 0);
	}
	cpustate->pending_irq = 0;
}

static CPU_EXECUTE( m6509 )
{
	m6509_Regs *cpustate = device->token;

	cpustate->icount = cycles;

	do
	{
		UINT8 op;
		PPC = PCD;

		debugger_instruction_hook(device, PCD);

		/* if an irq is pending, take it now */
		if( cpustate->pending_irq )
			m6509_take_irq(cpustate);

		op = RDOP();
		(*cpustate->insn[op])(cpustate);

		/* check if the I flag was just reset (interrupts enabled) */
		if( cpustate->after_cli )
		{
			LOG(("M6509 '%s' after_cli was >0", cpustate->device->tag));
			cpustate->after_cli = 0;
			if (cpustate->irq_state != CLEAR_LINE)
			{
				LOG((": irq line is asserted: set pending IRQ\n"));
				cpustate->pending_irq = 1;
			}
			else
			{
				LOG((": irq line is clear\n"));
			}
		}
		else
		if( cpustate->pending_irq )
			m6509_take_irq(cpustate);

	} while (cpustate->icount > 0);

	return cycles - cpustate->icount;
}

static void m6509_set_irq_line(m6509_Regs *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state) return;
		cpustate->nmi_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6509 '%s' set_nmi_line(ASSERT)\n", cpustate->device->tag));
			EAD = M6509_NMI_VEC;
			EAWH = PBWH;
			cpustate->icount -= 2;
			PUSH(PCH);
			PUSH(PCL);
			PUSH(P & ~F_B);
			P |= F_I;		/* knock out D and set I flag */
			PCL = RDMEM(EAD);
			PCH = RDMEM(EAD+1);
			LOG(("M6509 '%s' takes NMI ($%04x)\n", cpustate->device->tag, PCD));
		}
	}
	else
	{
		if( irqline == M6509_SET_OVERFLOW )
		{
			if( cpustate->so_state && !state )
			{
				LOG(( "M6509 '%s' set overflow\n", cpustate->device->tag));
				P|=F_V;
			}
			cpustate->so_state=state;
			return;
		}
		cpustate->irq_state = state;
		if( state != CLEAR_LINE )
		{
			LOG(( "M6509 '%s' set_irq_line(ASSERT)\n", cpustate->device->tag));
			cpustate->pending_irq = 1;
		}
	}
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m6509 )
{
	m6509_Regs *cpustate = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + M6509_IRQ_LINE:	m6509_set_irq_line(cpustate, M6509_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + M6509_SET_OVERFLOW:m6509_set_irq_line(cpustate, M6509_SET_OVERFLOW, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	m6509_set_irq_line(cpustate, INPUT_LINE_NMI, info->i); break;

		case CPUINFO_INT_PC:							PCW = info->i; 							break;
		case CPUINFO_INT_REGISTER + M6509_PC:			cpustate->pc.w.l = info->i;					break;
		case CPUINFO_INT_SP:							S = info->i;							break;
		case CPUINFO_INT_REGISTER + M6509_S:			cpustate->sp.b.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6509_P:			cpustate->p = info->i;						break;
		case CPUINFO_INT_REGISTER + M6509_A:			cpustate->a = info->i;						break;
		case CPUINFO_INT_REGISTER + M6509_X:			cpustate->x = info->i;						break;
		case CPUINFO_INT_REGISTER + M6509_Y:			cpustate->y = info->i;						break;
		case CPUINFO_INT_REGISTER + M6509_PC_BANK:		cpustate->pc_bank.b.h2 = info->i;			break;
		case CPUINFO_INT_REGISTER + M6509_IND_BANK:		cpustate->ind_bank.b.h2 = info->i;			break;
		case CPUINFO_INT_REGISTER + M6509_EA:			cpustate->ea.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + M6509_ZP:			cpustate->zp.w.l = info->i;					break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_FCT_M6502_READINDEXED_CALLBACK:	cpustate->rdmem_id = (m6502_read_indexed_func) info->f; break;
		case CPUINFO_FCT_M6502_WRITEINDEXED_CALLBACK:	cpustate->wrmem_id = (m6502_write_indexed_func) info->f; break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m6509 )
{
	m6509_Regs *cpustate = (device != NULL) ? device->token : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m6509_Regs);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 10;							break;

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + M6509_IRQ_LINE:	info->i = cpustate->irq_state;				break;
		case CPUINFO_INT_INPUT_STATE + M6509_SET_OVERFLOW:info->i = cpustate->so_state;				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc.w.l;				break;

		case CPUINFO_INT_PC:							info->i = PCD;							break;
		case CPUINFO_INT_REGISTER + M6509_PC:			info->i = cpustate->pc.w.l;					break;
		case CPUINFO_INT_SP:							info->i = S;							break;
		case CPUINFO_INT_REGISTER + M6509_S:			info->i = cpustate->sp.b.l;					break;
		case CPUINFO_INT_REGISTER + M6509_P:			info->i = cpustate->p;						break;
		case CPUINFO_INT_REGISTER + M6509_A:			info->i = cpustate->a;						break;
		case CPUINFO_INT_REGISTER + M6509_X:			info->i = cpustate->x;						break;
		case CPUINFO_INT_REGISTER + M6509_Y:			info->i = cpustate->y;						break;
		case CPUINFO_INT_REGISTER + M6509_PC_BANK:		info->i = cpustate->pc_bank.b.h2;			break;
		case CPUINFO_INT_REGISTER + M6509_IND_BANK:		info->i = cpustate->ind_bank.b.h2;			break;
		case CPUINFO_INT_REGISTER + M6509_EA:			info->i = cpustate->ea.w.l;					break;
		case CPUINFO_INT_REGISTER + M6509_ZP:			info->i = cpustate->zp.w.l;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m6509);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(m6509);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(m6509);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(m6509);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m6509);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m6502);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP:			info->internal_map8 = ADDRESS_MAP_NAME(m6509_mem); break;
		case CPUINFO_FCT_M6502_READINDEXED_CALLBACK:	info->f = (genf *) cpustate->rdmem_id;		break;
		case CPUINFO_FCT_M6502_WRITEINDEXED_CALLBACK:	info->f = (genf *) cpustate->wrmem_id;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "M6509");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "MOS Technology 6509"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0beta");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller\nCopyright Peter Trauner\nall rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->p & 0x80 ? 'N':'.',
				cpustate->p & 0x40 ? 'V':'.',
				cpustate->p & 0x20 ? 'R':'.',
				cpustate->p & 0x10 ? 'B':'.',
				cpustate->p & 0x08 ? 'D':'.',
				cpustate->p & 0x04 ? 'I':'.',
				cpustate->p & 0x02 ? 'Z':'.',
				cpustate->p & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M6509_PC:			sprintf(info->s, "PC:%04X", cpustate->pc.w.l); break;
		case CPUINFO_STR_REGISTER + M6509_S:			sprintf(info->s, "S:%02X", cpustate->sp.b.l); break;
		case CPUINFO_STR_REGISTER + M6509_P:			sprintf(info->s, "P:%02X", cpustate->p); break;
		case CPUINFO_STR_REGISTER + M6509_A:			sprintf(info->s, "A:%02X", cpustate->a); break;
		case CPUINFO_STR_REGISTER + M6509_X:			sprintf(info->s, "X:%02X", cpustate->x); break;
		case CPUINFO_STR_REGISTER + M6509_Y:			sprintf(info->s, "Y:%02X", cpustate->y); break;
		case CPUINFO_STR_REGISTER + M6509_PC_BANK:		sprintf(info->s, "M0:%01X", cpustate->pc_bank.b.h2); break;
		case CPUINFO_STR_REGISTER + M6509_IND_BANK:		sprintf(info->s, "M1:%01X", cpustate->ind_bank.b.h2); break;
		case CPUINFO_STR_REGISTER + M6509_EA:			sprintf(info->s, "EA:%04X", cpustate->ea.w.l); break;
		case CPUINFO_STR_REGISTER + M6509_ZP:			sprintf(info->s, "ZP:%03X", cpustate->zp.w.l); break;
	}
}

