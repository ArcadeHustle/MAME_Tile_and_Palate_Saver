/***************************************************************************

    Jaleco fcombat

***************************************************************************/

#include "driver.h"

int fcombat_sh;
int fcombat_sv;


#define BACKGROUND_X_START		32
#define BACKGROUND_X_START_FLIP	72

#define VISIBLE_X_MIN			(12*8)
#define VISIBLE_X_MAX			(52*8)
#define VISIBLE_Y_MIN			(2*8)
#define VISIBLE_Y_MAX			(30*8)


static tilemap *bgmap;

static TILE_GET_INFO( get_bg_tile_info )
{
	int tileno,palno;//32*16 x 32

	//palno=(tile_index-(tile_index/32*16)*32*16)/32;

	tileno = memory_region(machine, "user1")[tile_index];
	palno=0x18;//memory_region(machine, "user2")[tile_index]>>3;
	SET_TILE_INFO(2,tileno,palno,0);
}

UINT8 fcombat_cocktail_flip;

static UINT8 char_palette, sprite_palette;
static UINT8 char_bank;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( fcombat )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* fg chars/sprites */
	for (i = 0; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[(i & 0x1c0) | ((i & 3) << 4) | ((i >> 2) & 0x0f)] & 0x0f) | 0x10;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* bg chars (this is not the full story... there are four layers mixed */
	/* using another PROM */
	for (i = 0x200; i < 0x300; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}



/*************************************
 *
 *  Video system startup
 *
 *************************************/

VIDEO_START( fcombat )
{
	bgmap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows, 16, 16,32*8*2,32);
}



/*************************************
 *
 *  Video register I/O
 *
 *************************************/

WRITE8_HANDLER( fcombat_videoreg_w )
{
	/* bit 0 = flip screen and joystick input multiplexor */
	fcombat_cocktail_flip = data & 1;

	/* bits 1-2 char lookup table bank */
	char_palette = (data & 0x06) >> 1;

	/* bits 3 char bank */
	char_bank = (data & 0x08) >> 3;

	/* bits 4-5 unused */

	/* bits 6-7 sprite lookup table bank */
	sprite_palette = 0;//(data & 0xc0) >> 6;
	//popmessage("%08x",data);
}



VIDEO_UPDATE( fcombat )
{
	int sx, sy, offs, i;

	/* draw background */
	tilemap_set_scrolly( bgmap,0, fcombat_sh);
	tilemap_set_scrollx( bgmap,0, fcombat_sv-24);

tilemap_mark_all_tiles_dirty(bgmap);
	tilemap_draw(bitmap,cliprect,bgmap,0,0);
	//draw_background(bitmap, cliprect);

	/* draw sprites */
	for (i = 0; i < spriteram_size; i += 4)
	{
		int flags = spriteram[i + 0];
		int y = spriteram[i + 1] ^ 255;
		int code = spriteram[i + 2]+((flags&0x20)<<3);
		int x = spriteram[i + 3] * 2 + 72;

		int xflip = flags & 0x80;
		int yflip = flags & 0x40;
		int doubled =0;// flags & 0x10;
		int wide = flags & 0x08;
		int code2 = code;

		int color = ((flags >> 1) & 0x03) | ((code >> 5) & 0x04) | (code & 0x08) | (sprite_palette * 16);
				const gfx_element *gfx =  screen->machine->gfx[1];

		if (fcombat_cocktail_flip)
		{
			x = 64*8 - gfx->width - x;
			y = 32*8 - gfx->height - y;
			if (wide) y -= gfx->height;
			xflip = !xflip;
			yflip = !yflip;
		}

		if (wide)
		{
			if (yflip)
				code |= 0x10, code2 &= ~0x10;
			else
				code &= ~0x10, code2 |= 0x10;

			drawgfx(bitmap, gfx, code2, color, xflip, yflip, x, y + gfx->height,
			        cliprect, TRANSPARENCY_PEN, 0);
		}

		if(flags&0x10)
		{



			drawgfx(bitmap, gfx, code2+16, color, xflip, yflip, x, y + gfx->height,
			        cliprect, TRANSPARENCY_PEN, 0);
			drawgfx(bitmap, gfx, code2+16*2, color, xflip, yflip, x, y + 2*gfx->height,
			        cliprect, TRANSPARENCY_PEN, 0);
			drawgfx(bitmap, gfx, code2+16*3, color, xflip, yflip, x, y + 3*gfx->height,
			        cliprect, TRANSPARENCY_PEN, 0);

		}



		drawgfx(bitmap, gfx, code, color, xflip, yflip, x, y,
		        cliprect, TRANSPARENCY_PEN, 0);

		if (doubled) i += 4;
	}

	/* draw the visible text layer */
	for (sy = VISIBLE_Y_MIN/8; sy < VISIBLE_Y_MAX/8; sy++)
		for (sx = VISIBLE_X_MIN/8; sx < VISIBLE_X_MAX/8; sx++)
		{
			int x = fcombat_cocktail_flip ? (63*8 - 8*sx) : 8*sx;
			int y = fcombat_cocktail_flip ? (31*8 - 8*sy) : 8*sy;

			offs = sx + sy * 64;
			drawgfx(bitmap, screen->machine->gfx[0],
				videoram[offs] + 256 * char_bank,
				((videoram[offs] & 0xf0) >> 4) + char_palette * 16,
				fcombat_cocktail_flip, fcombat_cocktail_flip, x, y,
				cliprect, TRANSPARENCY_PEN, 0);
		}
	return 0;
}
