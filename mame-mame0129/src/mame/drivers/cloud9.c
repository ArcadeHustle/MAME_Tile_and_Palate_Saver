/***************************************************************************

    Atari Cloud 9 (prototype) hardware

    driver by Mike Balfour

    Games supported:
        * Cloud 9
        * Firebeast

    Known issues:
        * none at this time

****************************************************************************

    Horizontal sync chain:

        Appears to be the same as Crystal Castles. See ccastles.c for
        details.

        Pixel clock = 5MHz
        HBLANK ends at H = 0
        HBLANK begins at H = 256
        HSYNC begins at H = 260 (? unconfirmed)
        HSYNC ends at H = 288 (? unconfirmed)
        HTOTAL = 320

    Vertical sync chain:

        Appears to be similar to Crystal Castles. The PROM at 10E seems
        to have a similar layout to the SYNC PROM used by Crystal
        Castles. The standard PROM maps as follows:

        VBLANK ends at V = 23
        VBLANK begins at V = 255
        VSYNC begins at V = 3
        VSYNC ends at V = 6
        VTOTAL = 256

    Interrupts:

        IRQ is clocked by /32V, so IRQs are generated a V = 0,64,128,192.

****************************************************************************

    This hardware is very similar to Crystal Castles. The primary
    difference is the lack of banked ROM and the mapping of the bitmap
    layer into the lower 20k instead of the lower 32k. In order to do
    this, they split the bitmap into two banks. Bank 0 holds pixels
    0,1,4,5,... while bank 1 holds pixels 2,3,6,7,... This is all handled
    transparently by bitmode.

    The lower 24 lines of video RAM are used for working RAM. This amounts
    to $600 bytes at $0000. In order to provide more work RAM, the write
    protect logic selects bank 1 for accesses to VRAM at $4000, so the
    other $600 bytes can be accessed there. Only Firebeast seems to use
    this RAM.

    0000        R/W     Write to bit mode X latch (and through to RAM)
    0001        R/W     Write to bit mode Y latch (and through to RAM)
    0002        R/W     Access the bitmap via bit mode
    0000-3FFF   R/W     Video RAM bank 0 (or 1 or both, depending on video control)
    4000-4FFF   R/W     Video RAM bank 1
    5000-53FF   R/W     Motion Object RAM
    5400        W       Watchdog
    5480        W       IRQ Acknowledge
    5500-557F   W       Color RAM (9 bits, 4 banks, LSB of Blue is addr&$40)
    5580        W       Auto-increment X bitmap index (~D7)
    5581        W       Auto-increment Y bitmap index (~D7)
    5584        W       VRAM Both Banks - (D7) seems to allow writing to both banks
    5585        W       Invert screen?
    5586        W       VRAM Bank select?
    5587        W       Color bank select
    5600        W       Coin Counter 1 (D7)
    5601        W       Coin Counter 2 (D7)
    5602        W       Start1 LED (~D7)
    5603        W       Start2 LED (~D7)
    5700        W       EAROM recall
    5800        R       IN0 (D7=Vblank, D6=Right Coin, D5=Left Coin, D4=Aux, D3=Self Test)
    5801        R       IN1 (D7=Start1, D6=Start2, D5=Fire, D4=Zap)
    5900        R       Trackball Vert
    5901        R       Trackball Horiz
    5A00-5A0F   R/W     Pokey 1
    5B00-5B0F   R/W     Pokey 2
    5C00-5CFF   W       EAROM
    6000-FFFF   R       Program ROM

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "sound/pokey.h"
#include "cloud9.h"


#define MASTER_CLOCK	(10000000)

#define PIXEL_CLOCK		(MASTER_CLOCK/2)
#define HTOTAL			(320)
#define VTOTAL			(256)



/*************************************
 *
 *  Globals
 *
 *************************************/

static UINT8 *nvram_stage;
static const UINT8 *syncprom;
static UINT8 irq_state;
static emu_timer *irq_timer;

int cloud9_vblank_start;
int cloud9_vblank_end;



/*************************************
 *
 *  VBLANK and IRQ generation
 *
 *************************************/

INLINE void schedule_next_irq(running_machine *machine, int curscanline)
{
	/* IRQ is clocked by /32V, so every 64 scanlines */
	curscanline = (curscanline + 64) & 255;

	/* next one at the start of this scanline */
	timer_adjust_oneshot(irq_timer, video_screen_get_time_until_pos(machine->primary_screen, curscanline, 0), curscanline);
}


static TIMER_CALLBACK( clock_irq )
{
	/* assert the IRQ if not already asserted */
	if (!irq_state)
	{
		cpu_set_input_line(machine->cpu[0], 0, ASSERT_LINE);
		irq_state = 1;
	}

	/* force an update now */
	video_screen_update_partial(machine->primary_screen, video_screen_get_vpos(machine->primary_screen));

	/* find the next edge */
	schedule_next_irq(machine, param);
}


static CUSTOM_INPUT( get_vblank )
{
	int scanline = video_screen_get_vpos(field->port->machine->primary_screen);
	return (~syncprom[scanline & 0xff] >> 1) & 1;
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static MACHINE_START( cloud9 )
{
	rectangle visarea;

	/* initialize globals */
	syncprom = memory_region(machine, "proms") + 0x000;

	/* find the start of VBLANK in the SYNC PROM */
	for (cloud9_vblank_start = 0; cloud9_vblank_start < 256; cloud9_vblank_start++)
		if ((syncprom[(cloud9_vblank_start - 1) & 0xff] & 2) != 0 && (syncprom[cloud9_vblank_start] & 2) == 0)
			break;
	if (cloud9_vblank_start == 0)
		cloud9_vblank_start = 256;

	/* find the end of VBLANK in the SYNC PROM */
	for (cloud9_vblank_end = 0; cloud9_vblank_end < 256; cloud9_vblank_end++)
		if ((syncprom[(cloud9_vblank_end - 1) & 0xff] & 2) == 0 && (syncprom[cloud9_vblank_end] & 2) != 0)
			break;

	/* can't handle the wrapping case */
	assert(cloud9_vblank_end < cloud9_vblank_start);

	/* reconfigure the visible area to match */
	visarea.min_x = 0;
	visarea.max_x = 255;
	visarea.min_y = cloud9_vblank_end + 1;
	visarea.max_y = cloud9_vblank_start;
	video_screen_configure(machine->primary_screen, 320, 256, &visarea, HZ_TO_ATTOSECONDS(PIXEL_CLOCK) * VTOTAL * HTOTAL);

	/* create a timer for IRQs and set up the first callback */
	irq_timer = timer_alloc(machine, clock_irq, NULL);
	irq_state = 0;
	schedule_next_irq(machine, 0-64);

	/* allocate backing memory for the NVRAM */
	generic_nvram = auto_malloc(generic_nvram_size);

	/* setup for save states */
	state_save_register_global(machine, irq_state);
	state_save_register_global_pointer(machine, generic_nvram, generic_nvram_size);
}


static MACHINE_RESET( cloud9 )
{
	cpu_set_input_line(machine->cpu[0], 0, CLEAR_LINE);
	irq_state = 0;
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

static WRITE8_HANDLER( irq_ack_w )
{
	if (irq_state)
	{
		cpu_set_input_line(space->machine->cpu[0], 0, CLEAR_LINE);
		irq_state = 0;
	}
}


static WRITE8_HANDLER( cloud9_led_w )
{
	set_led_status(offset, ~data & 0x80);
}


static WRITE8_HANDLER( cloud9_coin_counter_w )
{
	coin_counter_w(offset, data & 0x80);
}


static READ8_HANDLER( leta_r )
{
	return input_port_read(space->machine, offset ? "TRACKX" : "TRACKY");
}



/*************************************
 *
 *  NVRAM handling
 *
 *************************************/

static NVRAM_HANDLER( cloud9 )
{
	if (read_or_write)
	{
		/* on power down, the EAROM is implicitly stored */
		memcpy(generic_nvram, nvram_stage, generic_nvram_size);
		mame_fwrite(file, generic_nvram, generic_nvram_size);
	}
	else if (file)
		mame_fread(file, generic_nvram, generic_nvram_size);
	else
		memset(generic_nvram, 0, generic_nvram_size);
}


static WRITE8_HANDLER( nvram_recall_w )
{
	memcpy(nvram_stage, generic_nvram, generic_nvram_size);
}


static WRITE8_HANDLER( nvram_store_w )
{
	memcpy(generic_nvram, nvram_stage, generic_nvram_size);
}


static READ8_HANDLER( nvram_r )
{
	/* only a single XD2212 for 4 bits of NVRAM */
	return nvram_stage[offset] | 0xf0;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( cloud9_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0001) AM_WRITE(cloud9_bitmode_addr_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(cloud9_bitmode_r, cloud9_bitmode_w)
	AM_RANGE(0x0000, 0x4fff) AM_READWRITE(SMH_BANK1, cloud9_videoram_w)
	AM_RANGE(0x5000, 0x53ff) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0x5400, 0x547f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5480, 0x54ff) AM_WRITE(irq_ack_w)
	AM_RANGE(0x5500, 0x557f) AM_RAM_WRITE(cloud9_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0x5580, 0x5587) AM_MIRROR(0x0078) AM_WRITE(cloud9_video_control_w)
	AM_RANGE(0x5600, 0x5601) AM_MIRROR(0x0078) AM_WRITE(cloud9_coin_counter_w)
	AM_RANGE(0x5602, 0x5603) AM_MIRROR(0x0078) AM_WRITE(cloud9_led_w)
	AM_RANGE(0x5680, 0x56ff) AM_WRITE(nvram_store_w)
	AM_RANGE(0x5700, 0x577f) AM_WRITE(nvram_recall_w)
	AM_RANGE(0x5800, 0x5800) AM_MIRROR(0x007e) AM_READ_PORT("IN0")
	AM_RANGE(0x5801, 0x5801) AM_MIRROR(0x007e) AM_READ_PORT("IN1")
	AM_RANGE(0x5900, 0x5903) AM_MIRROR(0x007c) AM_READ(leta_r)
	AM_RANGE(0x5a00, 0x5a0f) AM_MIRROR(0x00f0) AM_READWRITE(pokey1_r, pokey1_w)
	AM_RANGE(0x5b00, 0x5b0f) AM_MIRROR(0x00f0) AM_READWRITE(pokey2_r, pokey2_w)
	AM_RANGE(0x5c00, 0x5cff) AM_MIRROR(0x0300) AM_READWRITE(nvram_r, SMH_RAM) AM_BASE(&nvram_stage) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cloud9 )
	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(get_vblank, NULL)

	PORT_START("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xfe, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x22, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(	0x1e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(	0x1a, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(	0x16, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(	0x12, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	0x0e, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("TRACKX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)
INPUT_PORTS_END


static INPUT_PORTS_START( firebeas )
	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(get_vblank, NULL)

	PORT_START("IN1")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_REVERSE

	PORT_START("TRACKX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_REVERSE
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( cloud9 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_16x16x4_planar, 0, 4 )
GFXDECODE_END



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const pokey_interface pokey_config =
{
	{ 0 },
	input_port_2_r
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( cloud9 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", M6502, MASTER_CLOCK/8)
	MDRV_CPU_PROGRAM_MAP(cloud9_map,0)

	MDRV_MACHINE_START(cloud9)
	MDRV_MACHINE_RESET(cloud9)
	MDRV_NVRAM_HANDLER(cloud9)
	MDRV_WATCHDOG_VBLANK_INIT(8)

	/* video hardware */
	MDRV_GFXDECODE(cloud9)
	MDRV_PALETTE_LENGTH(64)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE((float)PIXEL_CLOCK / (float)VTOTAL / (float)HTOTAL)
	MDRV_SCREEN_SIZE(HTOTAL, VTOTAL)
	MDRV_SCREEN_VBLANK_TIME(0)			/* VBLANK is handled manually */
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 231)

	MDRV_VIDEO_START(cloud9)
	MDRV_VIDEO_UPDATE(cloud9)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("pokey1", POKEY, MASTER_CLOCK/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("pokey2", POKEY, MASTER_CLOCK/8)
	MDRV_SOUND_CONFIG(pokey_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( cloud9 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "c9_6000.bin", 0x6000, 0x2000, CRC(b5d95d98) SHA1(9a347e5fc6e9e753e5c6972341725b5f4412e451) )
	ROM_LOAD( "c9_8000.bin", 0x8000, 0x2000, CRC(49af8f22) SHA1(c118372bec0c428c2b60d29df95f358b302d5e66) )
	ROM_LOAD( "c9_a000.bin", 0xa000, 0x2000, CRC(7cf404a6) SHA1(d20b662102f8426af51b1ca4ed8e18b00d711365) )
	ROM_LOAD( "c9_c000.bin", 0xc000, 0x2000, CRC(26a4d7df) SHA1(8eef0a5f5d1ff13eec75d0c50f5a5dea28486ae5) )
	ROM_LOAD( "c9_e000.bin", 0xe000, 0x2000, CRC(6e663bce) SHA1(4f4a5dc57ba6bc38a17973a6644849f6f5a2dfd1) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "c9_gfx0.bin", 0x0000, 0x1000, CRC(d01a8019) SHA1(a77d6125b116ab4bf9446e3b99469dad2719f7e5) )
	ROM_LOAD( "c9_gfx1.bin", 0x1000, 0x1000, CRC(514ac009) SHA1(f05081d8da47e650b0bd12cd00460c98a4f745b1) )
	ROM_LOAD( "c9_gfx2.bin", 0x2000, 0x1000, CRC(930c1ade) SHA1(ba22cb7b105da2ab8c40574e70f18d594d833452) )
	ROM_LOAD( "c9_gfx3.bin", 0x3000, 0x1000, CRC(27e9b88d) SHA1(a1d27e62eea9cdff662a3c160f650bbdb32b7f47) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "63s141.e10",  0x0000, 0x0100, BAD_DUMP CRC(8e98083f) SHA1(ed29c7ed2226613ed5d09ecef4e645e3b53f7f8d) )	/* Sync PROM */
	ROM_LOAD( "63s141.m10",  0x0100, 0x0100, BAD_DUMP CRC(b0b039c0) SHA1(724fa88f3f3c62b3c9345cdb13e114a10b7bbdb0) )	/* ??? PROM */
	ROM_LOAD( "82s129.p3",   0x0200, 0x0100, BAD_DUMP CRC(615d784d) SHA1(e7e6397ae45d6ae8b3670b457ede79c42d18d71f) )	/* VRAM Write Protect PROM */
	ROM_LOAD( "63s141.m8",   0x0300, 0x0100, BAD_DUMP CRC(6d7479ec) SHA1(7a7c30f5846b98afaaca2af9aab82416ebafe4cc) )	/* ??? PROM */
ROM_END


ROM_START( firebeas )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "6000.j2",  0x6000, 0x2000, CRC(dbd04782) SHA1(1237511b00a4121fae01a07bca05a76202d74058) )
	ROM_LOAD( "8000.kl2", 0x8000, 0x2000, CRC(96231ca4) SHA1(e865d4396968f94a284fe9993f066d55f9c225a4) )
	ROM_LOAD( "a000.lm2", 0xa000, 0x2000, CRC(f7e0bf25) SHA1(d116cbc7643a3328f7a1fe4ff03d8a087b8c7648) )
	ROM_LOAD( "c000.n2",  0xc000, 0x2000, CRC(43a91b74) SHA1(6b38703e793ebcbee7b060053485072d9dac6b8b) )
	ROM_LOAD( "e000.p2",  0xe000, 0x1000, CRC(8e5045ab) SHA1(8e5e8dd7710dc5ec68602f069dfc30e1bcaf7411) )
	ROM_RELOAD(           0xf000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mo0000.r12", 0x0000, 0x2000, CRC(5246c979) SHA1(a975ede0a6c2c91f4373ecba1e2f61f1aedcee62) )
	ROM_LOAD( "mo2000.p12", 0x2000, 0x2000, CRC(1a3b6d57) SHA1(d9015140e69fdc3f73113f0afc3be2579197017a) )
	ROM_LOAD( "mo4000.n12", 0x4000, 0x2000, CRC(69e18319) SHA1(3bf3cbe4ea06ea71928eff8a57c2ef7dc6e6716a) )
	ROM_LOAD( "mo6000.m12", 0x6000, 0x2000, CRC(b722997f) SHA1(65a2618ecd8b4923f30f59c1fb95124cf0391964) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "63s141.e10", 0x0000, 0x0100, CRC(8e98083f) SHA1(ed29c7ed2226613ed5d09ecef4e645e3b53f7f8d) )	/* Sync PROM */
	ROM_LOAD( "63s141.m10", 0x0100, 0x0100, CRC(b0b039c0) SHA1(724fa88f3f3c62b3c9345cdb13e114a10b7bbdb0) )	/* ??? PROM */
	ROM_LOAD( "82s129.p3",  0x0200, 0x0100, CRC(615d784d) SHA1(e7e6397ae45d6ae8b3670b457ede79c42d18d71f) )	/* VRAM Write Protect PROM */
	ROM_LOAD( "63s141.m8",  0x0300, 0x0100, CRC(6d7479ec) SHA1(7a7c30f5846b98afaaca2af9aab82416ebafe4cc) )	/* ??? PROM */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, cloud9,   0, cloud9, cloud9,   0, ROT0, "Atari", "Cloud 9 (prototype)", GAME_SUPPORTS_SAVE )
GAME( 1983, firebeas, 0, cloud9, firebeas, 0, ROT0, "Atari", "Firebeast (prototype)", GAME_SUPPORTS_SAVE )
