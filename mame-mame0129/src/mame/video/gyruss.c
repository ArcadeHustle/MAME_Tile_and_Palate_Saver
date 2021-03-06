/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"


UINT8 *gyruss_videoram;
UINT8 *gyruss_colorram;
UINT8 *gyruss_spriteram;
UINT8 *gyruss_flipscreen;

static tilemap *gyruss_tilemap;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Gyruss has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
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

PALETTE_INIT( gyruss )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_rg[3], weights_b[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, resistances_rg, weights_rg, 470, 0,
			2, resistances_b,  weights_b,  470, 0,
			0, 0, 0, 0, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 32);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 32;

	/* sprites map to the lower 16 palette entries */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* characters map to the upper 16 palette entries */
	for (i = 0x100; i < 0x140; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry + 0x10);
	}
}



WRITE8_HANDLER( gyruss_spriteram_w )
{
	video_screen_update_now(space->machine->primary_screen);
	gyruss_spriteram[offset] = data;
}


static TILE_GET_INFO( gyruss_get_tile_info )
{
	int code = ((gyruss_colorram[tile_index] & 0x20) << 3) | gyruss_videoram[tile_index];
	int color = gyruss_colorram[tile_index] & 0x0f;
	int flags = TILE_FLIPYX(gyruss_colorram[tile_index] >> 6);

	tileinfo->group = (gyruss_colorram[tile_index] & 0x10) ? 0 : 1;

	SET_TILE_INFO(2, code, color, flags);
}


VIDEO_START( gyruss )
{
	gyruss_tilemap = tilemap_create(machine, gyruss_get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transmask(gyruss_tilemap, 0, 0x00, 0);	/* opaque */
	tilemap_set_transmask(gyruss_tilemap, 1, 0x0f, 0);  /* transparent */
}



READ8_HANDLER( gyruss_scanline_r )
{
	/* reads 1V - 128V */
	return video_screen_get_vpos(space->machine->primary_screen);
}


static void draw_sprites(gfx_element **gfx, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0xbc; offs >= 0; offs -= 4)
	{
		int x = gyruss_spriteram[offs];
		int y = 241 - gyruss_spriteram[offs + 3];

		int gfx_bank = gyruss_spriteram[offs + 1] & 0x01;
		int code = ((gyruss_spriteram[offs + 2] & 0x20) << 2) | ( gyruss_spriteram[offs + 1] >> 1);
		int color = gyruss_spriteram[offs + 2] & 0x0f;
		int flip_x = ~gyruss_spriteram[offs + 2] & 0x40;
		int flip_y =  gyruss_spriteram[offs + 2] & 0x80;

		drawgfx(bitmap, gfx[gfx_bank], code, color, flip_x, flip_y, x, y, cliprect, TRANSPARENCY_PEN, 0);
	}
}


VIDEO_UPDATE( gyruss )
{
	if (cliprect->min_y == video_screen_get_visible_area(screen)->min_y)
	{
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
		tilemap_set_flip(ALL_TILEMAPS, (*gyruss_flipscreen & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}

	tilemap_draw(bitmap, cliprect, gyruss_tilemap, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(screen->machine->gfx, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, gyruss_tilemap, 0, 0);

	return 0;
}
