/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


UINT8 *spcforce_scrollram;


WRITE8_HANDLER( spcforce_flip_screen_w )
{
	flip_screen_set(space->machine, ~data & 0x01);
}


VIDEO_UPDATE( spcforce )
{
	int offs;


	/* draw the characters as sprites because they could be overlapping */

	bitmap_fill(bitmap,cliprect,0);


	for (offs = 0; offs < videoram_size; offs++)
	{
		int code,sx,sy,col;


		sy = 8 * (offs / 32) -  (spcforce_scrollram[offs]       & 0x0f);
		sx = 8 * (offs % 32) + ((spcforce_scrollram[offs] >> 4) & 0x0f);

		code = videoram[offs] + ((colorram[offs] & 0x01) << 8);
		col  = (~colorram[offs] >> 4) & 0x07;

		if (flip_screen_get(screen->machine))
		{
			sx = 248 - sx;
			sy = 248 - sy;
		}

		drawgfx(bitmap,screen->machine->gfx[0],
				code, col,
				flip_screen_get(screen->machine), flip_screen_get(screen->machine),
				sx, sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
	return 0;
}
