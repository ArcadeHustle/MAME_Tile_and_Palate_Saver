#pragma once

#ifndef __Z80_H__
#define __Z80_H__

#include "cpuintrf.h"

enum
{
	Z80_PC, Z80_SP,
	Z80_A, Z80_B, Z80_C, Z80_D, Z80_E, Z80_H, Z80_L,
	Z80_AF, Z80_BC, Z80_DE, Z80_HL,
	Z80_IX, Z80_IY,	Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2,
	Z80_R, Z80_I, Z80_IM, Z80_IFF1, Z80_IFF2, Z80_HALT,
	Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3, Z80_MEMPTR,

	Z80_GENPC = REG_GENPC,
	Z80_GENSP = REG_GENSP,
	Z80_GENPCBASE = REG_GENPCBASE
};

enum
{
	Z80_TABLE_op,
	Z80_TABLE_cb,
	Z80_TABLE_ed,
	Z80_TABLE_xy,
	Z80_TABLE_xycb,
	Z80_TABLE_ex	/* cycles counts for taken jr/jp/call and interrupt latency (rst opcodes) */
};

enum
{
	CPUINFO_PTR_Z80_CYCLE_TABLE = CPUINFO_PTR_CPU_SPECIFIC,
	CPUINFO_PTR_Z80_CYCLE_TABLE_LAST = CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_ex
};

extern CPU_GET_INFO( z80 );
#define CPU_Z80 CPU_GET_INFO_NAME( z80 )

extern CPU_DISASSEMBLE( z80 );

#endif /* __Z80_H__ */
