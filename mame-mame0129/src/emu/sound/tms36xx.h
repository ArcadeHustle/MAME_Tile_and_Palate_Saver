#pragma once

#ifndef __TMS36XX_H__
#define __TMS36XX_H__

/* subtypes */
#define MM6221AA    21      /* Phoenix (fixed melodies) */
#define TMS3615 	15		/* Naughty Boy, Pleiads (13 notes, one output) */
#define TMS3617 	17		/* Monster Bash (13 notes, six outputs) */

/* The interface structure */
typedef struct _tms36xx_interface tms36xx_interface;
struct _tms36xx_interface
{
	int subtype;
	double decay[6];	/* decay times for the six harmonic notes */
	double speed;		/* tune speed (meaningful for the TMS3615 only) */
};

/* MM6221AA interface functions */
extern void mm6221aa_tune_w(int chip, int tune);

/* TMS3615/17 interface functions */
extern void tms36xx_note_w(int chip, int octave, int note);

/* TMS3617 interface functions */
extern void tms3617_enable_w(int chip, int enable);

SND_GET_INFO( tms36xx );

#endif /* __TMS36XX_H__ */
