#include "sndintrf.h"
#include "streams.h"
#include "tms3615.h"

#define VMIN	0x0000
#define VMAX	0x7fff

#define TONES 13

static const int divisor[TONES] = { 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239 };

struct TMS3615 {
	sound_stream *channel;	/* returned by stream_create() */
	int samplerate; 		/* output sample rate */
	int basefreq;			/* chip's base frequency */
	int counter8[TONES];	/* tone frequency counter for 8' */
	int counter16[TONES];	/* tone frequency counter for 16'*/
	int output8; 			/* output signal bits for 8' */
	int output16; 			/* output signal bits for 16' */
	int enable; 			/* mask which tones to play */
};

static STREAM_UPDATE( tms3615_sound_update )
{
	struct TMS3615 *tms = param;
	int samplerate = tms->samplerate;
	stream_sample_t *buffer8 = outputs[TMS3615_FOOTAGE_8];
	stream_sample_t *buffer16 = outputs[TMS3615_FOOTAGE_16];

	while( samples-- > 0 )
	{
		int sum8 = 0, sum16 = 0, tone = 0;

		for (tone = 0; tone < TONES; tone++)
		{
			// 8'

			tms->counter8[tone] -= (tms->basefreq / divisor[tone]);

			while( tms->counter8[tone] <= 0 )
			{
				tms->counter8[tone] += samplerate;
				tms->output8 ^= 1 << tone;
			}

			if (tms->output8 & tms->enable & (1 << tone))
			{
				sum8 += VMAX;
			}

			// 16'

			tms->counter16[tone] -= (tms->basefreq / divisor[tone] / 2);

			while( tms->counter16[tone] <= 0 )
			{
				tms->counter16[tone] += samplerate;
				tms->output16 ^= 1 << tone;
			}

			if (tms->output16 & tms->enable & (1 << tone))
			{
				sum16 += VMAX;
			}
		}

        *buffer8++ = sum8 / TONES;
        *buffer16++ = sum16 / TONES;
	}

	tms->enable = 0;
}

void tms3615_enable_w(int chip, int enable)
{
	struct TMS3615 *tms = sndti_token(SOUND_TMS3615, chip);
	tms->enable = enable;
}

static SND_START( tms3615 )
{
	struct TMS3615 *tms;

	tms = auto_malloc(sizeof(*tms));
	memset(tms, 0, sizeof(*tms));

	tms->channel = stream_create(device, 0, 2, clock/8, tms, tms3615_sound_update);
	tms->samplerate = clock/8;
	tms->basefreq = clock;

    return tms;
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( tms3615 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}

SND_GET_INFO( tms3615 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( tms3615 );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tms3615 );		break;
		case SNDINFO_PTR_STOP:							/* Nothing */									break;
		case SNDINFO_PTR_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "TMS3615");						break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "TI PSG");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");							break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);						break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
