/***************************************************************************

    Taito Crazy Balloon hardware

    Analog emulation - Jan 2006, Derrick Renaud

***************************************************************************/

#include "driver.h"
#include "crbaloon.h"
#include "sound/sn76477.h"
#include "sound/discrete.h"


/* timing sources */
#define CRBALOON_16H			(CRBALOON_MASTER_XTAL/2/2/16)

/* enables */
#define CRBALOON_LAUGH_EN		NODE_01
#define CRBALOON_MUSIC_EN		NODE_02
#define CRBALOON_MUSIC_DATA		NODE_03

/* nodes - adjusters */
#define CRBALOON_VR2			NODE_05
#define CRBALOON_VR3			NODE_06

/* nodes - sounds */
#define CRBALOON_LAUGH_SND		NODE_80
#define CRBALOON_MUSIC_SND		NODE_81



void crbaloon_audio_set_music_freq(const address_space *space, UINT8 freq)
{
	discrete_sound_w(space, CRBALOON_MUSIC_DATA, freq);
}


void crbaloon_audio_set_music_enable(const address_space *space, int enabled)
{
	discrete_sound_w(space, CRBALOON_MUSIC_EN, enabled);
}


void crbaloon_audio_set_explosion_enable(int enabled)
{
	sn76477_enable_w(0, enabled);
}


void crbaloon_audio_set_breath_enable(int enabled)
{
	/* changes slf_res to 10k (middle of two 10k resistors)
       it also puts a tantal capacitor against GND on the output,
       but this section of the schematics is not readable. */
	sn76477_slf_res_w(0, enabled ? RES_K(10) : RES_K(20) );
}


void crbaloon_audio_set_appear_enable(int enabled)
{
	/* APPEAR is connected to MIXER B */
	sn76477_mixer_b_w(0, enabled);
}


void crbaloon_audio_set_laugh_enable(const address_space *space, int enabled)
{
	discrete_sound_w(space, CRBALOON_LAUGH_EN, enabled);
}



static const discrete_555_desc desc_crbaloon_laugh_osc =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,				// B+ voltage of 555
	DEFAULT_555_VALUES
};


static const discrete_dac_r1_ladder desc_crbaloon_music_dac =
{
	3,
	{0, RES_K(470), RES_K(120)},
	5, RES_K(470), 0, 0
};



static DISCRETE_SOUND_START(crbaloon)

	/************************************************
    * Input register mapping
    ************************************************/
	DISCRETE_INPUT_LOGIC(CRBALOON_LAUGH_EN)
	DISCRETE_INPUT_LOGIC(CRBALOON_MUSIC_EN)
	DISCRETE_INPUT_DATA (CRBALOON_MUSIC_DATA)

	DISCRETE_ADJUSTMENT_TAG(CRBALOON_VR2, 0, 0.5, DISC_LINADJ, "VR2")
	DISCRETE_ADJUSTMENT_TAG(CRBALOON_VR3, 0, 1,   DISC_LINADJ, "VR3")

	/************************************************
    * Laugh is a VCO modulated by a constant
    * square wave.
    ************************************************/
	DISCRETE_555_ASTABLE(NODE_10, 1, RES_K(10), RES_K(100), CAP_U(1), &desc_crbaloon_laugh_osc)
	DISCRETE_CRFILTER_VREF(NODE_11, 1, NODE_10,
		1.0/(1.0/RES_K(5) + 1.0/RES_K(10) + 1.0/RES_K(100)), // 5k & 10k are 555 internal
		CAP_U(10),
		/* The following will calculate the reference control voltage with no signal applied to the cap. */
		5.0* (1.0/(1.0/RES_K(10) + 1.0/RES_K(100))) / (RES_K(5)+(1.0/(1.0/RES_K(10) + 1.0/RES_K(100)))) )
	DISCRETE_555_ASTABLE_CV(NODE_12, CRBALOON_LAUGH_EN, RES_K(1), RES_K(22), CAP_U(.1), NODE_11, &desc_crbaloon_laugh_osc)
	DISCRETE_MULTIPLY(NODE_13, 1, NODE_12, CRBALOON_VR2)
	DISCRETE_CRFILTER(CRBALOON_LAUGH_SND, 1, NODE_13, RES_K(20), CAP_U(1))

	/************************************************
    * Music Generator is amplitude modulated by a
    * linear ramp.
    ************************************************/
	DISCRETE_NOTE(NODE_20, 1, CRBALOON_16H / 2, CRBALOON_MUSIC_DATA, 255, 7, DISC_CLK_IS_FREQ)
	DISCRETE_DAC_R1(NODE_21, CRBALOON_MUSIC_EN, NODE_20, DEFAULT_TTL_V_LOGIC_1, &desc_crbaloon_music_dac)
//  DISCRETE_RAMP(NODE_21, 1, RAMP, GRAD, MIN, MAX, CLAMP)
	DISCRETE_MULTIPLY(NODE_22, 1, NODE_21, CRBALOON_VR3)
	DISCRETE_CRFILTER(CRBALOON_MUSIC_SND, 1, NODE_22, RES_K(50), CAP_U(1))

	/************************************************
    * Final mix and output.
    ************************************************/
	DISCRETE_ADDER2(NODE_90, 1, CRBALOON_LAUGH_SND, CRBALOON_MUSIC_SND)
	DISCRETE_CRFILTER(NODE_91, 1, NODE_90, RES_K(100), CAP_U(1))

	DISCRETE_OUTPUT(NODE_91, 65000.0/12)

DISCRETE_SOUND_END



static const sn76477_interface crbaloon_sn76477_interface =
{
	RES_K( 47),	/*  4 noise_res          */
	RES_K(330),	/*  5 filter_res         */
	CAP_P(470),	/*  6 filter_cap         */
	RES_K(220),	/*  7 decay_res          */
	CAP_U(1.0),	/*  8 attack_decay_cap   */
	RES_K(4.7),	/* 10 attack_res         */
	RES_M(  1),	/* 11 amplitude_res      */
	RES_K(200),	/* 12 feedback_res       */
	5.0,		/* 16 vco_voltage        */
	CAP_P(470),	/* 17 vco_cap            */
	RES_K(330),	/* 18 vco_res            */
	5.0,		/* 19 pitch_voltage      */
	RES_K( 20),	/* 20 slf_res (variable) */
	CAP_P(420),	/* 21 slf_cap            */
	CAP_U(1.0),	/* 23 oneshot_cap        */
	RES_K( 47),	/* 24 oneshot_res        */
	0,			/* 22 vco                */
	0,			/* 26 mixer A            */
	0,			/* 25 mixer B (variable) */
	1,			/* 27 mixer C            */
	1,			/* 1  envelope 1         */
	0,			/* 28 envelope 2         */
	0			/* 9  enable (variable)  */
};



MACHINE_DRIVER_START( crbaloon_audio )

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn", SN76477, 0)
	MDRV_SOUND_CONFIG(crbaloon_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(crbaloon)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END
