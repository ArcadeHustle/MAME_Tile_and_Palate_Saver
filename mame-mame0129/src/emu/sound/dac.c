#include "sndintrf.h"
#include "streams.h"
#include "dac.h"
#include <math.h>


/* default to 4x oversampling */
#define DEFAULT_SAMPLE_RATE (48000 * 4)


struct dac_info
{
	sound_stream	*channel;
	INT16			output;
	INT16			UnsignedVolTable[256];
	INT16			SignedVolTable[256];
};



static STREAM_UPDATE( DAC_update )
{
	struct dac_info *info = param;
	stream_sample_t *buffer = outputs[0];
	INT16 out = info->output;

	while (samples--) *(buffer++) = out;
}


void dac_data_w(int num,UINT8 data)
{
	struct dac_info *info = sndti_token(SOUND_DAC, num);
	INT16 out = info->UnsignedVolTable[data];

	if (info->output != out)
	{
		/* update the output buffer before changing the registers */
		stream_update(info->channel);
		info->output = out;
	}
}


void dac_signed_data_w(int num,UINT8 data)
{
	struct dac_info *info = sndti_token(SOUND_DAC, num);
	INT16 out = info->SignedVolTable[data];

	if (info->output != out)
	{
		/* update the output buffer before changing the registers */
		stream_update(info->channel);
		info->output = out;
	}
}


void dac_data_16_w(int num,UINT16 data)
{
	struct dac_info *info = sndti_token(SOUND_DAC, num);
	INT16 out = data >> 1;		/* range      0..32767 */

	if (info->output != out)
	{
		/* update the output buffer before changing the registers */
		stream_update(info->channel);
		info->output = out;
	}
}


void dac_signed_data_16_w(int num,UINT16 data)
{
	struct dac_info *info = sndti_token(SOUND_DAC, num);
	INT16 out = (INT32)data - (INT32)0x08000;	/* range -32768..32767 */
						/* casts avoid potential overflow on some ABIs */

	if (info->output != out)
	{
		/* update the output buffer before changing the registers */
		stream_update(info->channel);
		info->output = out;
	}
}


static void DAC_build_voltable(struct dac_info *info)
{
	int i;


	/* build volume table (linear) */
	for (i = 0;i < 256;i++)
	{
		info->UnsignedVolTable[i] = i * 0x101 / 2;	/* range      0..32767 */
		info->SignedVolTable[i] = i * 0x101 - 0x8000;	/* range -32768..32767 */
	}
}


static SND_START( dac )
{
	struct dac_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	DAC_build_voltable(info);

	info->channel = stream_create(device,0,1,clock ? clock : DEFAULT_SAMPLE_RATE,info,DAC_update);
	info->output = 0;

	state_save_register_device_item(device, 0, info->output);

	return info;
}



WRITE8_HANDLER( dac_0_data_w )
{
	dac_data_w(0,data);
}

WRITE8_HANDLER( dac_1_data_w )
{
	dac_data_w(1,data);
}

WRITE8_HANDLER( dac_2_data_w )
{
	dac_data_w(2,data);
}

WRITE8_HANDLER( dac_0_signed_data_w )
{
	dac_signed_data_w(0,data);
}

WRITE8_HANDLER( dac_1_signed_data_w )
{
	dac_signed_data_w(1,data);
}

WRITE8_HANDLER( dac_2_signed_data_w )
{
	dac_signed_data_w(2,data);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( dac )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( dac )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( dac );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( dac );		break;
		case SNDINFO_PTR_STOP:							/* nothing */								break;
		case SNDINFO_PTR_RESET:							/* nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "DAC");						break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "DAC");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");						break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);					break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

