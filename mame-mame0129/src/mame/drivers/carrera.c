/*

This is a simple 'Pairs' game called
Carrera by BS Electronics



PCB Layout
----------

|----------------------------------------------|
|            22.1184MHz  Z80                   |
|                                     ROM.IC1  |
|_  BATTERY              ROM.IC22              |
  |                                            |
 _|                      6116         ROM.IC2  |
|                                              |
|                                              |
|J  AY-3-8910                         ROM.IC3  |
|A                       DSW1(8)               |
|M                                             |
|M                       DSW2(8)      ROM.IC4  |
|A                                             |
|                        DSW3(8)               |
|                                     ROM.IC5  |
|_  PROM.IC39            DSW4(8)               |
  |                                    6116    |
 _|                                            |
|                   HD6845             6116    |
|----------------------------------------------|
Notes:
      Z80 @ 3.6864MHz [22.1184/6]
      AY-3-8910 @ 1.8432MHz [22.1184/12]


Emulation Notes:
 Corrupt Tile on the first R in Carrera? (unlikely to be a bug, HW is very simple..)
 There is also a 'Bomberman' title in the GFX roms, unused from what I can see.

*/

#define MASTER_CLOCK	XTAL_22_1184MHz

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

static UINT8* carrera_tileram;



static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x4fff) AM_READ(SMH_ROM)
	AM_RANGE(0xe000, 0xe7ff) AM_READ(SMH_RAM)
	AM_RANGE(0xf000, 0xffff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x4fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xe800, 0xe800) AM_DEVWRITE(MC6845, "crtc", mc6845_address_w)
	AM_RANGE(0xe801, 0xe801) AM_DEVWRITE(MC6845, "crtc", mc6845_register_w)
	AM_RANGE(0xf000, 0xffff) AM_WRITE(SMH_RAM) AM_BASE(&carrera_tileram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("IN3")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN4")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("IN5")
	AM_RANGE(0x06, 0x06) AM_WRITE(SMH_NOP) // ?
	AM_RANGE(0x08, 0x08) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(ay8910_write_port_0_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( carrera )
	PORT_START("IN0")	/* Port 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	/* unused / unknown inputs, not dips */
	PORT_DIPNAME( 0x20, 0x20, "0" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")	/* Port 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	/* unused / unknown inputs, not dips */
	PORT_DIPNAME( 0x04, 0x04, "1" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* I suspect the 4 below are the 4xDSWs */
	PORT_START("IN2")	/* Port 2 */
	PORT_DIPNAME( 0x01, 0x01, "2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")	/* Port 3 */
	PORT_DIPNAME( 0x01, 0x01, "3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")	/* Port 4 */
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN5")	/* Port 5 */
	PORT_DIPNAME( 0x01, 0x01, "5" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Playing Graphics" )
	PORT_DIPSETTING(    0x08, "Bricks" )
	PORT_DIPSETTING(    0x00, "Fruits" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug?" ) // displays numbers over the game area
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5),RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( carrera )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static VIDEO_UPDATE(carrera)
{

	int x,y;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = carrera_tileram[count&0x7ff] | carrera_tileram[(count&0x7ff)+0x800]<<8;

			drawgfx(bitmap,screen->machine->gfx[0],tile,0,0,0,x*8,y*8,cliprect,TRANSPARENCY_NONE,0);
			count++;
		}
	}
	return 0;
}

static READ8_HANDLER( unknown_r )
{
	return mame_rand(space->machine);
}

/* these are set as input, but I have no idea which input port it uses is for the AY */
static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	unknown_r,
	unknown_r,
	NULL,
	NULL
};

static PALETTE_INIT(carrera)
{
	int x;
	UINT8 *src = memory_region ( machine, "proms" );

	for (x=0;x<32;x++)
		palette_set_color_rgb(machine, x, pal3bit(src[x] >> 0), pal3bit(src[x] >> 3), pal2bit(src[x] >> 6));
}


static const mc6845_interface mc6845_intf =
{
	"main",		/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	NULL,		/* callback for display state changes */
	NULL,		/* HSYNC callback */
	NULL		/* VSYNC callback */
};


static MACHINE_DRIVER_START( carrera )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, MASTER_CLOCK / 6)
	MDRV_CPU_PROGRAM_MAP(readmem, writemem)
	MDRV_CPU_IO_MAP(io_map, 0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_MC6845_ADD("crtc", MC6845, MASTER_CLOCK / 16, mc6845_intf)

	MDRV_GFXDECODE(carrera)
	MDRV_PALETTE_LENGTH(32)
	MDRV_PALETTE_INIT(carrera)

	MDRV_VIDEO_UPDATE(carrera)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, MASTER_CLOCK/12)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END


ROM_START( carrera )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "27512.ic22", 0x00000, 0x10000, CRC(2385b9c8) SHA1(12d4397779e074096fbb23b114985f104366b79c) )

	ROM_REGION( 0x50000, "gfx1", 0 )
	ROM_LOAD( "27512.ic1", 0x00000, 0x10000, CRC(a16e914e) SHA1(09f2271f193a7bffd62ef6e428ecbf9aa1154860) )
	ROM_LOAD( "27512.ic2", 0x10000, 0x10000, CRC(147036a5) SHA1(34b4818fe61c5b13220b0a2001987b68b655b2cb) )
	ROM_LOAD( "27512.ic3", 0x20000, 0x10000, CRC(920eee0e) SHA1(85e6d5292b751c57c64d17858bd00292356599e3) )
	ROM_LOAD( "27512.ic4", 0x30000, 0x10000, CRC(97433f36) SHA1(39f3c6b76ad540693682832aba6e4fc400ca3753) )
	ROM_LOAD( "27512.ic5", 0x40000, 0x10000, CRC(ffa75920) SHA1(aa5619f5aabcdfa250bb24bcad101a8c512a1776) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.ic39", 0x00, 0x20, CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) )
ROM_END


GAME( 19??, carrera, 0, carrera, carrera,0, ROT0, "BS Electronics", "Carrera (Version 6.7)", GAME_WRONG_COLORS )

