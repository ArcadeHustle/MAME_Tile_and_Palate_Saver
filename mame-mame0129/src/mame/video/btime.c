/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

This file is also used by scregg.c

***************************************************************************/

#include "driver.h"
#include "includes/btime.h"


UINT8 *btime_videoram;
size_t btime_videoram_size;
UINT8 *btime_colorram;
UINT8 *lnc_charbank;
UINT8 *bnj_backgroundram;
UINT8 *zoar_scrollram;
UINT8 *deco_charram;
size_t bnj_backgroundram_size;

static UINT8 btime_palette = 0;
static UINT8 bnj_scroll1 = 0;
static UINT8 bnj_scroll2 = 0;
static bitmap_t *background_bitmap;

static UINT8 *sprite_dirty, *char_dirty;

/***************************************************************************

    Burger Time doesn't have a color PROM. It uses RAM to dynamically
    create the palette.
    The palette RAM is connected to the RGB output this way:

    bit 7 -- 15 kohm resistor  -- BLUE (inverted)
          -- 33 kohm resistor  -- BLUE (inverted)
          -- 15 kohm resistor  -- GREEN (inverted)
          -- 33 kohm resistor  -- GREEN (inverted)
          -- 47 kohm resistor  -- GREEN (inverted)
          -- 15 kohm resistor  -- RED (inverted)
          -- 33 kohm resistor  -- RED (inverted)
    bit 0 -- 47 kohm resistor  -- RED (inverted)

***************************************************************************/
PALETTE_INIT( btime )
{
	int i;


	/* Burger Time doesn't have a color PROM, but Hamburge has. */
	/* This function is also used by Eggs. */
	if (color_prom == 0) return;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

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

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

/***************************************************************************

    Convert the color PROMs into a more useable format.

    The PROM is connected to the RGB output this way:

    bit 7 -- 47 kohm resistor  -- RED
          -- 33 kohm resistor  -- RED
          -- 15 kohm resistor  -- RED
          -- 47 kohm resistor  -- GREEN
          -- 33 kohm resistor  -- GREEN
          -- 15 kohm resistor  -- GREEN
          -- 33 kohm resistor  -- BLUE
    bit 0 -- 15 kohm resistor  -- BLUE

***************************************************************************/
PALETTE_INIT( lnc )
{
	int i;


	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 7) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 3) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 0) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}


MACHINE_RESET( lnc )
{
    *lnc_charbank = 1;
}


/***************************************************************************

Start the video hardware emulation.

***************************************************************************/

VIDEO_START( btime )
{
    bnj_scroll1 = 0;
    bnj_scroll2 = 0;
    btime_palette = 0;

	sprite_dirty = auto_malloc(256 * sizeof(*sprite_dirty));
	memset(sprite_dirty, 1, 256 * sizeof(*sprite_dirty));

	char_dirty = auto_malloc(1024 * sizeof(*char_dirty));
	memset(char_dirty, 1, 1024 * sizeof(*char_dirty));
}


VIDEO_START( bnj )
{
    /* the background area is twice as wide as the screen */
    int width = 256;
    int height = 256;
    bitmap_format format = video_screen_get_format(machine->primary_screen);
    background_bitmap = auto_bitmap_alloc(2*width, height, format);

    VIDEO_START_CALL(btime);
}


WRITE8_HANDLER( btime_paletteram_w )
{
    /* RGB output is inverted */
    paletteram_BBGGGRRR_w(space,offset,~data);
}

WRITE8_HANDLER( lnc_videoram_w )
{
	btime_videoram[offset] = data;
	btime_colorram[offset] = *lnc_charbank;
}

READ8_HANDLER( btime_mirrorvideoram_r )
{
    int x,y;

    /* swap x and y coordinates */
    x = offset / 32;
    y = offset % 32;
    offset = 32 * y + x;

    return btime_videoram[offset];
}

READ8_HANDLER( btime_mirrorcolorram_r )
{
    int x,y;

    /* swap x and y coordinates */
    x = offset / 32;
    y = offset % 32;
    offset = 32 * y + x;

    return btime_colorram[offset];
}

WRITE8_HANDLER( btime_mirrorvideoram_w )
{
    int x,y;

    /* swap x and y coordinates */
    x = offset / 32;
    y = offset % 32;
    offset = 32 * y + x;

    btime_videoram[offset] = data;
}

WRITE8_HANDLER( lnc_mirrorvideoram_w )
{
    int x,y;

    /* swap x and y coordinates */
    x = offset / 32;
    y = offset % 32;
    offset = 32 * y + x;

    lnc_videoram_w(space,offset,data);
}

WRITE8_HANDLER( btime_mirrorcolorram_w )
{
    int x,y;

    /* swap x and y coordinates */
    x = offset / 32;
    y = offset % 32;
    offset = 32 * y + x;

    btime_colorram[offset] = data;
}

WRITE8_HANDLER( deco_charram_w )
{
    if (deco_charram[offset] == data)  return;

    deco_charram[offset] = data;

    offset &= 0x1fff;

    /* dirty sprite */
    sprite_dirty[offset >> 5] = 1;

    /* diry char */
    char_dirty  [offset >> 3] = 1;
}

WRITE8_HANDLER( bnj_background_w )
{
	bnj_backgroundram[offset] = data;
}

WRITE8_HANDLER( bnj_scroll1_w )
{
    bnj_scroll1 = data;
}

WRITE8_HANDLER( bnj_scroll2_w )
{
    bnj_scroll2 = data;
}

WRITE8_HANDLER( zoar_video_control_w )
{
    // Zoar video control
    //
    // Bit 0-2 = Unknown (always 0). Marked as MCOL on schematics
    // Bit 3-4 = Palette
    // Bit 7   = Flip Screen

	btime_palette = (data & 0x30) >> 3;
	flip_screen_set(space->machine, data & 0x80);
}

WRITE8_HANDLER( btime_video_control_w )
{
    // Btime video control
    //
    // Bit 0   = Flip screen
    // Bit 1-7 = Unknown

	flip_screen_set(space->machine, data & 0x01);
}

WRITE8_HANDLER( bnj_video_control_w )
{
    /* Bnj/Lnc works a little differently than the btime/eggs (apparently). */
    /* According to the information at: */
    /* http://www.davesclassics.com/arcade/Switch_Settings/BumpNJump.sw */
    /* SW8 is used for cocktail video selection (as opposed to controls), */
    /* but bit 7 of the input port is used for vblank input. */
    /* My guess is that this switch open circuits some connection to */
    /* the monitor hardware. */
    /* For now we just check 0x40 in DSW1, and ignore the write if we */
    /* are in upright controls mode. */

    if (input_port_read(space->machine, "DSW1") & 0x40) /* cocktail mode */
        btime_video_control_w(space, offset, data);
}

WRITE8_HANDLER( disco_video_control_w )
{
	btime_palette = (data >> 2) & 0x03;

	if (!(input_port_read(space->machine, "DSW1") & 0x40)) /* cocktail mode */
	{
		flip_screen_set(space->machine, data & 0x01);
	}
}


static void draw_chars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 transparency, UINT8 color, int priority)
{
    offs_t offs;

    for (offs = 0; offs < btime_videoram_size; offs++)
    {
        UINT8 x = 31 - (offs / 32);
        UINT8 y = offs % 32;

        UINT16 code = btime_videoram[offs] + 256 * (btime_colorram[offs] & 3);

        /* check priority */
        if ((priority != -1) && (priority != ((code >> 7) & 0x01)))  continue;

        if (flip_screen_get(machine))
        {
            x = 31 - x;
            y = 33 - y;
        }

        drawgfx(bitmap,machine->gfx[0],
                code,
                color,
                flip_screen_get(machine),flip_screen_get(machine),
                8*x,8*y,
                cliprect,transparency,0);
    }
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 color,
                         UINT8 sprite_y_adjust, UINT8 sprite_y_adjust_flip_screen,
                         UINT8 *sprite_ram, offs_t interleave)
{
    int i;
    offs_t offs;

    /* draw the sprites */
    for (i = 0, offs = 0; i < 8; i++, offs += 4*interleave)
    {
        int x, y;
        UINT8 flipx,flipy;

        if (!(sprite_ram[offs + 0] & 0x01)) continue;

        x = 240 - sprite_ram[offs + 3*interleave];
        y = 240 - sprite_ram[offs + 2*interleave];

        flipx = sprite_ram[offs + 0] & 0x04;
        flipy = sprite_ram[offs + 0] & 0x02;

        if (flip_screen_get(machine))
        {
            x = 240 - x;
            y = 256 - y + sprite_y_adjust_flip_screen;

            flipx = !flipx;
            flipy = !flipy;
        }

        y = y - sprite_y_adjust;

        drawgfx(bitmap,machine->gfx[1],
                sprite_ram[offs + interleave],
                color,
                flipx,flipy,
                x, y,
                cliprect,TRANSPARENCY_PEN,0);

        y = y + (flip_screen_get(machine) ? -256 : 256);

        // Wrap around
        drawgfx(bitmap,machine->gfx[1],
                sprite_ram[offs + interleave],
                color,
                flipx,flipy,
                x,y,
                cliprect,TRANSPARENCY_PEN,0);
    }
}


static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8* tmap, UINT8 color)
{
    int i;
    const UINT8 *gfx = memory_region(machine, "bg_map");
    int scroll = -(bnj_scroll2 | ((bnj_scroll1 & 0x03) << 8));

    // One extra iteration for wrap around
    for (i = 0; i < 5; i++, scroll += 256)
    {
		offs_t offs;
        offs_t tileoffset = tmap[i & 3] * 0x100;

        // Skip if this title is completely off the screen
        if (scroll > 256)  break;
        if (scroll < -256) continue;

        for (offs = 0; offs < 0x100; offs++)
        {
            int x = 240 - (16 * (offs / 16) + scroll);
            int y = 16 * (offs % 16);

            if (flip_screen_get(machine))
            {
                x = 240 - x;
                y = 256 - y;
            }

            drawgfx(bitmap, machine->gfx[2],
                    gfx[tileoffset + offs],
                    color,
                    flip_screen_get(machine),flip_screen_get(machine),
                    x,y,
                    cliprect,TRANSPARENCY_NONE,0);
        }
    }
}


static void decode_modified(running_machine *machine, UINT8 *sprite_ram, int interleave)
{
    int i,offs;


    /* decode dirty characters */
    for (offs = btime_videoram_size - 1;offs >= 0;offs--)
    {
        int code;

        code = btime_videoram[offs] + 256 * (btime_colorram[offs] & 3);

        if (char_dirty[code])
        {
            decodechar(machine->gfx[0],code,deco_charram);

            char_dirty[code] = 0;
        }
    }

    /* decode dirty sprites */
    for (i = 0, offs = 0;i < 8; i++, offs += 4*interleave)
    {
        int code;

        code  = sprite_ram[offs + interleave];

        if (sprite_dirty[code])
        {
            decodechar(machine->gfx[1],code,deco_charram);

            sprite_dirty[code] = 0;
        }
    }
}


VIDEO_UPDATE( btime )
{
    if (bnj_scroll1 & 0x10)
    {
        int i, start;

        // Generate tile map
        static UINT8 btime_tilemap[4];

        if (flip_screen_get(screen->machine))
            start = 0;
        else
            start = 1;

        for (i = 0; i < 4; i++)
        {
            btime_tilemap[i] = start | (bnj_scroll1 & 0x04);
            start = (start + 1) & 0x03;
        }

        draw_background(screen->machine, bitmap, cliprect, btime_tilemap, 0);
        draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_PEN, 0, -1);
    }
    else
        draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_NONE, 0, -1);

    draw_sprites(screen->machine, bitmap, cliprect, 0, 1, 0, btime_videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( eggs )
{
    draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_NONE, 0, -1);
    draw_sprites(screen->machine, bitmap, cliprect, 0, 0, 0, btime_videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( lnc )
{
    draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_NONE, 0, -1);
    draw_sprites(screen->machine, bitmap, cliprect, 0, 1, 2, btime_videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( zoar )
{
    if (bnj_scroll1 & 0x04)
    {
        draw_background(screen->machine, bitmap, cliprect, zoar_scrollram, btime_palette);
        draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_PEN, btime_palette + 1, -1);
    }
    else
        draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_NONE, btime_palette + 1, -1);

    /* The order is important for correct priorities */
    draw_sprites(screen->machine, bitmap, cliprect, btime_palette + 1, 1, 2, btime_videoram + 0x1f, 0x20);
    draw_sprites(screen->machine, bitmap, cliprect, btime_palette + 1, 1, 2, btime_videoram,        0x20);

	return 0;
}


VIDEO_UPDATE( bnj )
{
    if (bnj_scroll1)
    {
        int scroll, offs;

        for (offs = bnj_backgroundram_size-1; offs >=0; offs--)
        {
            int sx,sy;

            sx = 16 * ((offs < 0x100) ? ((offs % 0x80) / 8) : ((offs % 0x80) / 8) + 16);
            sy = 16 * (((offs % 0x100) < 0x80) ? offs % 8 : (offs % 8) + 8);
            sx = 496 - sx;

            if (flip_screen_get(screen->machine))
            {
                sx = 496 - sx;
                sy = 256 - sy;
            }

            drawgfx(background_bitmap, screen->machine->gfx[2],
                    (bnj_backgroundram[offs] >> 4) + ((offs & 0x80) >> 3) + 32,
                    0,
                    flip_screen_get(screen->machine), flip_screen_get(screen->machine),
                    sx, sy,
                    0, TRANSPARENCY_NONE, 0);
        }

        /* copy the background bitmap to the screen */
        scroll = (bnj_scroll1 & 0x02) * 128 + 511 - bnj_scroll2;
        if (!flip_screen_get(screen->machine))
            scroll = 767-scroll;
        copyscrollbitmap(bitmap, background_bitmap, 1, &scroll, 0, 0, cliprect);

        /* copy the low priority characters followed by the sprites
           then the high priority characters */
        draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_PEN, 0, 1);
        draw_sprites(screen->machine, bitmap, cliprect, 0, 0, 0, btime_videoram, 0x20);
        draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_PEN, 0, 0);
    }
    else
    {
        draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_NONE, 0, -1);
        draw_sprites(screen->machine, bitmap, cliprect, 0, 0, 0, btime_videoram, 0x20);
    }

	return 0;
}


VIDEO_UPDATE( cookrace )
{
    int offs;

    for (offs = bnj_backgroundram_size-1; offs >=0; offs--)
    {
        int sx,sy;

        sx = 31 - (offs / 32);
        sy = offs % 32;

        if (flip_screen_get(screen->machine))
        {
            sx = 31 - sx;
            sy = 33 - sy;
        }

        drawgfx(bitmap, screen->machine->gfx[2],
                bnj_backgroundram[offs],
                0,
                flip_screen_get(screen->machine), flip_screen_get(screen->machine),
                8*sx,8*sy,
                cliprect, TRANSPARENCY_NONE, 0);
    }

    draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_PEN, 0, -1);
    draw_sprites(screen->machine, bitmap, cliprect, 0, 1, 0, btime_videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( disco )
{
    decode_modified(screen->machine, spriteram, 1);
    draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_NONE, btime_palette, -1);
    draw_sprites(screen->machine, bitmap, cliprect, btime_palette, 0, 0, spriteram, 1);

	return 0;
}


VIDEO_UPDATE( progolf )
{
	decode_modified(screen->machine, spriteram, 1);
	draw_chars(screen->machine, bitmap, cliprect, TRANSPARENCY_NONE, /*btime_palette*/0, -1);
//  draw_sprites(screen->machine, bitmap, cliprect, 0/*btime_palette*/, 0, 0, spriteram, 1);

	return 0;
}
