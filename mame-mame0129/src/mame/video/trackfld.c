/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"

UINT8 *trackfld_scroll;
UINT8 *trackfld_scroll2;

static tilemap *bg_tilemap;
static int bg_bank = 0, sprite_bank1 = 0, sprite_bank2 = 0;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Track 'n Field has one 32x8 palette PROM and two 256x4 lookup table PROMs
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
PALETTE_INIT( trackfld )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances_rg[0], rweights, 1000, 0,
			3, &resistances_rg[0], gweights, 1000, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

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
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* sprites */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* characters */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( trackfld_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( trackfld_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( trackfld_flipscreen_w )
{
	if (flip_screen_get(space->machine) != data)
	{
		flip_screen_set(space->machine, data);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( atlantol_gfxbank_w )
{
	static int old = 0;

	if( data & 1 )
	{
		/* male / female sprites switch */
		if( (old == 1 && (data & 1) == 1) ||
			(old == 0 && (data & 1) == 1) )
			sprite_bank2 = 0x200;
		else
			sprite_bank2 = 0;

		sprite_bank1 = 0;

		old = data & 1;
	}
	else
	{
		/* male / female sprites switch */
		if( (old == 0 && (data & 1) == 0) ||
			(old == 1 && (data & 1) == 0) )
			sprite_bank2 = 0;
		else
			sprite_bank2 = 0x200;

		sprite_bank1 = 0;

		old = data & 1;
	}

	if( (data & 3) == 3 )
	{
		if( sprite_bank2 )
			sprite_bank1 = 0x500;
		else
			sprite_bank1 = 0x300;
	}
	else if( (data & 3) == 2 )
	{
		if( sprite_bank2 )
			sprite_bank1 = 0x300;
		else
			sprite_bank1 = 0x100;
	}

	if( bg_bank != (data & 0x8) )
	{
		bg_bank = data & 0x8;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + 4 * (attr & 0xc0);
	int color = attr & 0x0f;
	int flags = ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x20) ? TILE_FLIPY : 0);

	if( bg_bank )
		code |= 0x400;

	SET_TILE_INFO(1, code, color, flags);
}

VIDEO_START( trackfld )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 64, 32);

	tilemap_set_scroll_rows(bg_tilemap, 32);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size - 2; offs >= 0; offs -= 2)
	{
		int attr = spriteram_2[offs];
		int code = spriteram[offs + 1];
		int color = attr & 0x0f;
		int flipx = ~attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs] - 1;
		int sy = 240 - spriteram_2[offs + 1];

		if (flip_screen_get(machine))
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		/* Note that this adjustement must be done AFTER handling flip screen, thus */
		/* proving that this is a hardware related "feature" */
		sy += 1;

		drawgfx(bitmap, machine->gfx[0],
			code + sprite_bank1 + sprite_bank2, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PENS,
			colortable_get_transpen_mask(machine->colortable, machine->gfx[0], color, 0));

		/* redraw with wraparound */
		drawgfx(bitmap,machine->gfx[0],
			code + sprite_bank1 + sprite_bank2, color,
			flipx, flipy,
			sx - 256, sy,
			cliprect,
			TRANSPARENCY_PENS,
			colortable_get_transpen_mask(machine->colortable, machine->gfx[0], color, 0));
	}
}

VIDEO_UPDATE( trackfld )
{
	int row, scrollx;

	for (row = 0; row < 32; row++)
	{
		scrollx = trackfld_scroll[row] + 256 * (trackfld_scroll2[row] & 0x01);
		if (flip_screen_get(screen->machine)) scrollx = -scrollx;
		tilemap_set_scrollx(bg_tilemap, row, scrollx);
	}

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
