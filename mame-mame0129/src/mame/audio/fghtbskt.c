/*

Fighting Basketball PCM unsigned 8 bit mono samples

*/

#include "driver.h"
#include "sound/samples.h"

static INT16 *samplebuf;

WRITE8_HANDLER( fghtbskt_samples_w )
{
	if( data & 1 )
		sample_start_raw(0, samplebuf + ((data & 0xf0) << 8), 0x2000, 8000, 0);
}

SAMPLES_START( fghtbskt_sh_start )
{
	running_machine *machine = device->machine;
	int i, len = memory_region_length(machine, "samples");
	UINT8 *ROM = memory_region(machine, "samples");

	samplebuf = auto_malloc(len * 2);

	for(i=0;i<len;i++)
		samplebuf[i] = ((INT8)(ROM[i] ^ 0x80)) * 256;
}
