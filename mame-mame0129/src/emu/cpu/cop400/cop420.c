/***************************************************************************

    cop420.c

    National Semiconductor COP420 Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*

    TODO:

    - get rid of LBIOps/InstLen
    - when is the microbus int cleared?
    - CKO sync input
    - save internal RAM when CKO is RAM power supply pin
    - run interrupt test suite
    - run production test suite
    - run microbus test suite

*/

#include "driver.h"
#include "cpuintrf.h"
#include "debugger.h"
#include "cop400.h"

#include "420ops.c"

/* Opcode Maps */

static const s_opcode COP420_OPCODE_23_MAP[]=
{
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},
	{1, ldd		 	},{1, ldd		},{1, ldd	 	},{1, ldd	 	},{1, ldd	 	},{1, ldd 		},{1, ldd	 	},{1, ldd	 	},

	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},

	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},
	{1, xad		 	},{1, xad		},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},{1, xad	 	},

	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	}
};

static void cop420_op23(cop400_state *cop400, UINT8 opcode)
{
	UINT8 opcode23 = ROM(PC++);

	(*(COP420_OPCODE_23_MAP[opcode23].function))(cop400, opcode23);
}

static const s_opcode COP420_OPCODE_33_MAP[]=
{
	{1, illegal		},{1, skgbz0 	},{1, illegal 	},{1, skgbz2 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, skgbz1 	},{1, illegal 	},{1, skgbz3 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, skgz	 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, inin 		},{1, inil 		},{1, ing	 	},{1, illegal 	},{1, cqma	 	},{1, illegal 	},{1, inl	 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, omg	 	},{1, illegal 	},{1, camq	 	},{1, illegal 	},{1, obd	 	},{1, illegal 	},

	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, ogi	 		},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},
	{1, ogi	 		},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},{1, ogi	 	},
	{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},{1, lei 		},
	{1, lei 		},{1, lei 		},{1, lei	 	},{1, lei	 	},{1, lei	 	},{1, lei	 	},{1, lei	 	},{1, lei	 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},

	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},
	{1, lbi		 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},{1, lbi	 	},

	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},
	{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	},{1, illegal 	}
};

static void cop420_op33(cop400_state *cop400, UINT8 opcode)
{
	UINT8 opcode33 = ROM(PC++);

	(*(COP420_OPCODE_33_MAP[opcode33].function))(cop400, opcode33);
}

static const s_opcode COP420_OPCODE_MAP[]=
{
	{1, clra		},{1, skmbz0	},{1, xor		},{1, skmbz2		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, casc		},{1, skmbz1	},{1, xabr		},{1, skmbz3		},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, skc			},{1, ske		},{1, sc		},{1, cop420_op23	},{1, xis		},{1, ld		},{1, x			},{1, xds 		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},
	{1, asc			},{1, add		},{1, rc		},{1, cop420_op33  	},{1, xis		},{1, ld		},{1, x			},{1, xds		},
	{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi			},{1, lbi		},{1, lbi		},{1, lbi		},{1, lbi		},

	{1, comp		},{1, skt		},{1, rmb2		},{1, rmb3			},{1, nop		},{1, rmb1		},{1, smb2		},{1, smb1		},
	{1,	cop420_ret	},{1, retsk		},{1, adt		},{1, smb3			},{1, rmb0		},{1, smb0		},{1, cba		},{1, xas		},
	{1, cab			},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc			},{1, aisc		},{1, aisc		},{1, aisc		},{1, aisc		},
	{2, jmp			},{2, jmp		},{2, jmp		},{2, jmp			},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal   },
	{2, jsr			},{2, jsr		},{2, jsr		},{2, jsr			},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},
	{1, stii		},{1, stii		},{1, stii		},{1, stii			},{1, stii		},{1, stii		},{1, stii		},{1, stii		},

	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, lqid		},

	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{1, jp		},
	{1, jp			},{1, jp		},{1, jp		},{1, jp			},{1, jp		},{1, jp		},{1, jp		},{2, jid		}
};

/* Memory Maps */

static ADDRESS_MAP_START( cop420_internal_rom, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cop420_internal_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

/****************************************************************************
 * Initialize emulation
 ****************************************************************************/
static CPU_INIT( cop420 )
{
	cop400_state *cop400 = device->token;
	int i;

	cop400->device = device;
	cop400->intf = (cop400_interface *) device->static_config;

	/* get address spaces */

	cop400->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cop400->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	cop400->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	/* set output pin masks */

	cop400->g_mask = 0x0f;	// G0-G3 available
	cop400->d_mask = 0x0f;	// D0-D3 available
	cop400->in_mask = 0x0f;	// IN0-IN3 available

	/* set clock divider */

	device_set_info_int(device, CPUINFO_INT_CLOCK_DIVIDER, cop400->intf->cki);

	/* allocate serial timer */

	cop400->serial_timer = timer_alloc(device->machine, cop400_serial_tick, cop400);
	timer_adjust_periodic(cop400->serial_timer, attotime_zero, 0, ATTOTIME_IN_HZ(device->clock / 16));

	/* allocate counter timer */

	cop400->counter_timer = timer_alloc(device->machine, cop400_counter_tick, cop400);
	timer_adjust_periodic(cop400->counter_timer, attotime_zero, 0, ATTOTIME_IN_HZ(device->clock / 16 / 4));

	/* allocate IN latch timer */

	cop400->inil_timer = timer_alloc(device->machine, cop400_inil_tick, cop400);
	timer_adjust_periodic(cop400->inil_timer, attotime_zero, 0, ATTOTIME_IN_HZ(device->clock / 16));

	/* allocate Microbus timer */

	if (cop400->intf->microbus == COP400_MICROBUS_ENABLED)
	{
		cop400->microbus_timer = timer_alloc(device->machine, cop400_microbus_tick, cop400);
		timer_adjust_periodic(cop400->microbus_timer, attotime_zero, 0, ATTOTIME_IN_HZ(device->clock / 16));
	}

	/* initialize instruction length array */

	for (i=0; i<256; i++) cop400->InstLen[i]=1;

	cop400->InstLen[0x60] = cop400->InstLen[0x61] = cop400->InstLen[0x62] = cop400->InstLen[0x63] =
	cop400->InstLen[0x68] = cop400->InstLen[0x69] = cop400->InstLen[0x6a] = cop400->InstLen[0x6b] =
	cop400->InstLen[0x33] = cop400->InstLen[0x23] = 2;

	/* initialize LBI opcode array */

	for (i=0; i<256; i++) cop400->LBIops[i] = 0;
	for (i=0x08; i<0x10; i++) cop400->LBIops[i] = 1;
	for (i=0x18; i<0x20; i++) cop400->LBIops[i] = 1;
	for (i=0x28; i<0x30; i++) cop400->LBIops[i] = 1;
	for (i=0x38; i<0x40; i++) cop400->LBIops[i] = 1;

	for (i=0; i<256; i++) cop400->LBIops33[i] = 0;
	for (i=0x80; i<0xc0; i++) cop400->LBIops33[i] = 1;

	/* register for state saving */

	state_save_register_device_item(device, 0, cop400->pc);
	state_save_register_device_item(device, 0, cop400->prevpc);
	state_save_register_device_item(device, 0, cop400->a);
	state_save_register_device_item(device, 0, cop400->b);
	state_save_register_device_item(device, 0, cop400->c);
	state_save_register_device_item(device, 0, cop400->en);
	state_save_register_device_item(device, 0, cop400->g);
	state_save_register_device_item(device, 0, cop400->q);
	state_save_register_device_item(device, 0, cop400->sa);
	state_save_register_device_item(device, 0, cop400->sb);
	state_save_register_device_item(device, 0, cop400->sc);
	state_save_register_device_item(device, 0, cop400->sio);
	state_save_register_device_item(device, 0, cop400->skl);
	state_save_register_device_item(device, 0, cop400->skip);
	state_save_register_device_item(device, 0, cop400->skip_lbi);
	state_save_register_device_item(device, 0, cop400->t);
	state_save_register_device_item(device, 0, cop400->skt_latch);
	state_save_register_device_item(device, 0, cop400->g_mask);
	state_save_register_device_item(device, 0, cop400->d_mask);
	state_save_register_device_item(device, 0, cop400->in_mask);
	state_save_register_device_item(device, 0, cop400->si);
	state_save_register_device_item(device, 0, cop400->last_skip);
	state_save_register_device_item_array(device, 0, cop400->in);
	state_save_register_device_item(device, 0, cop400->microbus_int);
	state_save_register_device_item(device, 0, cop400->halt);
}

static CPU_INIT( cop421 )
{
	cop400_state *cop400 = device->token;

	CPU_INIT_CALL(cop420);

	/* set output pin masks */
	cop400->in_mask = 0; // IN lines not available
}

static CPU_INIT( cop422 )
{
	cop400_state *cop400 = device->token;

	CPU_INIT_CALL(cop420);

	/* set output pin masks */
	cop400->g_mask = 0x0e; // only G2, G3 available
	cop400->d_mask = 0x0e; // only D2, D3 available
	cop400->in_mask = 0; // IN lines not available
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
static CPU_RESET( cop420 )
{
	cop400_state *cop400 = device->token;

	PC = 0;
	A = 0;
	B = 0;
	C = 0;
	OUT_D(0);
	EN = 0;
	WRITE_G(cop400, 0);
	SKL = 1;

	T = 0;
	cop400->skt_latch = 1;
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/
static CPU_EXECUTE( cop420 )
{
	cop400_state *cop400 = device->token;

	UINT8 opcode;

	cop400->icount = cycles;

	do
	{
		prevPC = PC;

		debugger_instruction_hook(device, PC);

		opcode = ROM(PC);

		if (cop400->skip_lbi)
		{
			int is_lbi = 0;

			if (opcode == 0x33)
			{
				is_lbi = cop400->LBIops33[ROM(PC+1)];
			}
			else
			{
				is_lbi = cop400->LBIops[opcode];
			}

			if (is_lbi)
			{
				cop400->icount -= COP420_OPCODE_MAP[opcode].cycles;

				PC += cop400->InstLen[opcode];
			}
			else
			{
				cop400->skip_lbi = 0;
			}
		}

		if (!cop400->skip_lbi)
		{
			int inst_cycles = COP420_OPCODE_MAP[opcode].cycles;
			PC++;
			(*(COP420_OPCODE_MAP[opcode].function))(cop400, opcode);
			cop400->icount -= inst_cycles;

			// check for interrupt

			if (BIT(EN, 1) && BIT(IL, 1))
			{
				void *function = COP420_OPCODE_MAP[ROM(PC)].function;

				if ((function != jp) &&	(function != jmp) && (function != jsr))
				{
					// store skip logic
					cop400->last_skip = cop400->skip;
					cop400->skip = 0;

					// push next PC
					PUSH(PC);

					// jump to interrupt service routine
					PC = 0x0ff;

					// disable interrupt
					EN &= ~0x02;
				}

				IL &= ~2;
			}

			// skip next instruction?

			if (cop400->skip)
			{
				void *function = COP420_OPCODE_MAP[ROM(PC)].function;

				opcode = ROM(PC);

				if ((function == lqid) || (function == jid))
				{
					cop400->icount -= 1;
				}
				else
				{
					cop400->icount -= COP420_OPCODE_MAP[opcode].cycles;
				}

				PC += cop400->InstLen[opcode];

				cop400->skip = 0;
			}
		}
	} while (cop400->icount > 0);

	return cycles - cop400->icount;
}

/**************************************************************************
 * Validity check
 **************************************************************************/
static CPU_VALIDITY_CHECK( cop420 )
{
	int error = FALSE;
	const cop400_interface *intf = (const cop400_interface *) config;

	if ((intf == NULL) || (intf->cki <= 0))
	{
		mame_printf_error("%s: %s has an invalid CPU configuration\n", driver->source_file, driver->name);
		error = TRUE;
	}

	return error;
}

static CPU_VALIDITY_CHECK( cop421 )
{
	int error = FALSE;
	const cop400_interface *intf = (const cop400_interface *) config;

	if ((intf == NULL) || (intf->cki <= 0) || (intf->microbus == COP400_MICROBUS_ENABLED))
	{
		mame_printf_error("%s: %s has an invalid CPU configuration\n", driver->source_file, driver->name);
		error = TRUE;
	}

	return error;
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( cop420 )
{
	cop400_state *cop400 = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + COP400_PC:			PC = info->i;							break;

	    case CPUINFO_INT_REGISTER + COP400_A:			A = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_B:			B = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_C:			C = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_EN:			EN = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_G:			G = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_Q:			Q = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SA:			SA = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SB:			SB = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SC:			SC = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SIO:			SIO = info->i;							break;
		case CPUINFO_INT_REGISTER + COP400_SKL:			SKL = info->i;							break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( cop420 )
{
	cop400_state *cop400 = (device != NULL) ? device->token : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cop400_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 16;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: info->i = 10;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_DATA: 	info->i = 6;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 		info->i = 9;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;						    break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = prevPC;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + COP400_PC:			info->i = PC;							break;
		case CPUINFO_INT_REGISTER + COP400_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + COP400_B:			info->i = B;							break;
		case CPUINFO_INT_REGISTER + COP400_C:			info->i = C;							break;
		case CPUINFO_INT_REGISTER + COP400_EN:			info->i = EN;							break;
		case CPUINFO_INT_REGISTER + COP400_G:			info->i = G;							break;
		case CPUINFO_INT_REGISTER + COP400_Q:			info->i = Q;							break;
		case CPUINFO_INT_REGISTER + COP400_SA:			info->i = SA;							break;
		case CPUINFO_INT_REGISTER + COP400_SB:			info->i = SB;							break;
		case CPUINFO_INT_REGISTER + COP400_SC:			info->i = SC;							break;
		case CPUINFO_INT_REGISTER + COP400_SIO:			info->i = SIO;							break;
		case CPUINFO_INT_REGISTER + COP400_SKL:			info->i = SKL;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(cop420);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop420);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(cop420);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(cop420);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cop420);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cop400->icount;			break;
		case CPUINFO_FCT_VALIDITY_CHECK:				info->validity_check = CPU_VALIDITY_CHECK_NAME(cop420);	break;

        case CPUINFO_PTR_INTERNAL_MEMORY_MAP_PROGRAM:
            info->internal_map8 = ADDRESS_MAP_NAME(cop420_internal_rom);                              break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP_DATA:
 			info->internal_map8 = ADDRESS_MAP_NAME(cop420_internal_ram);								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP420");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright MAME Team"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, " ");
			break;

		case CPUINFO_STR_REGISTER + COP400_PC:			sprintf(info->s, "PC:%04X", PC);		break;
		case CPUINFO_STR_REGISTER + COP400_A:			sprintf(info->s, "A:%01X", A );			break;
		case CPUINFO_STR_REGISTER + COP400_B:			sprintf(info->s, "B:%02X", B );			break;
		case CPUINFO_STR_REGISTER + COP400_C:			sprintf(info->s, "C:%01X", C);			break;
		case CPUINFO_STR_REGISTER + COP400_EN:			sprintf(info->s, "EN:%01X", EN);		break;
		case CPUINFO_STR_REGISTER + COP400_G:			sprintf(info->s, "G:%01X", G);			break;
		case CPUINFO_STR_REGISTER + COP400_Q:			sprintf(info->s, "Q:%02X", Q);			break;
		case CPUINFO_STR_REGISTER + COP400_SA:			sprintf(info->s, "SA:%04X", SA);		break;
		case CPUINFO_STR_REGISTER + COP400_SB:			sprintf(info->s, "SB:%04X", SB);		break;
		case CPUINFO_STR_REGISTER + COP400_SC:			sprintf(info->s, "SC:%04X", SC);		break;
		case CPUINFO_STR_REGISTER + COP400_SIO:			sprintf(info->s, "SIO:%01X", SIO);		break;
		case CPUINFO_STR_REGISTER + COP400_SKL:			sprintf(info->s, "SKL:%01X", SKL);		break;
	}
}

CPU_GET_INFO( cop421 )
{
	// COP421 is a 24-pin package version of the COP420, lacking the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop421);				break;
		case CPUINFO_FCT_VALIDITY_CHECK:				info->validity_check = CPU_VALIDITY_CHECK_NAME(cop421);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP421");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: CPU_GET_INFO_CALL(cop420); break;
	}
}

CPU_GET_INFO( cop422 )
{
	// COP422 is a 20-pin package version of the COP420, lacking G0/G1, D0/D1, and the IN ports

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cop422);				break;
		case CPUINFO_FCT_VALIDITY_CHECK:				info->validity_check = CPU_VALIDITY_CHECK_NAME(cop421);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP422");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: CPU_GET_INFO_CALL(cop420); break;
	}
}

CPU_GET_INFO( cop402 )
{
	// COP402 is a ROMless version of the COP420

	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP_PROGRAM:
 			info->internal_map8 = NULL;															break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "COP402");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "National Semiconductor COPS"); break;

		default: CPU_GET_INFO_CALL(cop420); break;
	}
}
