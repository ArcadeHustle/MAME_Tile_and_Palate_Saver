#pragma once

#ifndef __SPC700_H__
#define __SPC700_H__

#include "cpuintrf.h"

extern CPU_GET_INFO( spc700 );
#define CPU_SPC700 CPU_GET_INFO_NAME( spc700 )

/* ======================================================================== */
/* ============================= Configuration ============================ */
/* ======================================================================== */

#ifndef INLINE
#define INLINE static
#endif

#undef uint
#define uint unsigned int

/* Turn on optimizations for SNES since it doesn't hook up the interrupt lines */
#define SPC700_OPTIMIZE_SNES 1


/* ======================================================================== */
/* ============================== PROTOTYPES ============================== */
/* ======================================================================== */

enum
{
	SPC700_PC=1, SPC700_S, SPC700_P, SPC700_A, SPC700_X, SPC700_Y
};

#define SPC700_INT_NONE			0
#define SPC700_INT_IRQ			1
#define SPC700_INT_NMI			2


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* __SPC700_H__ */
