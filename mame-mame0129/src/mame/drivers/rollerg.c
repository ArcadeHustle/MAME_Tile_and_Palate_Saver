/***************************************************************************

Rollergames (GX999) (c) 1991 Konami

driver by Nicola Salmoria

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "video/konamiic.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "machine/eeprom.h"
#include "sound/3812intf.h"
#include "sound/k053260.h"

/* prototypes */
static MACHINE_RESET( rollerg );
static KONAMI_SETLINES_CALLBACK( rollerg_banking );

VIDEO_START( rollerg );
VIDEO_UPDATE( rollerg );



static int readzoomroms;

static WRITE8_HANDLER( rollerg_0010_w )
{
logerror("%04x: write %02x to 0010\n",cpu_get_pc(space->cpu),data);

	/* bits 0/1 are coin counters */
	coin_counter_w(0,data & 0x01);
	coin_counter_w(1,data & 0x02);

	/* bit 2 enables 051316 ROM reading */
	readzoomroms = data & 0x04;

	/* bit 5 enables 051316 wraparound */
	K051316_wraparound_enable(0, data & 0x20);

	/* other bits unknown */
}

static READ8_HANDLER( rollerg_K051316_r )
{
	if (readzoomroms) return K051316_rom_0_r(space, offset);
	else return K051316_0_r(space, offset);
}

static READ8_HANDLER( rollerg_sound_r )
{
	/* If the sound CPU is running, read the status, otherwise
       just make it pass the test */
	return k053260_0_r(space, 2 + offset);
}

static WRITE8_HANDLER( soundirq_w )
{
	cpu_set_input_line_and_vector(space->machine->cpu[1],0,HOLD_LINE,0xff);
}

static TIMER_CALLBACK( nmi_callback )
{
	cpu_set_input_line(machine->cpu[1], INPUT_LINE_NMI, ASSERT_LINE);
}

static WRITE8_HANDLER( sound_arm_nmi_w )
{
	cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_NMI, CLEAR_LINE);
	timer_set(space->machine, ATTOTIME_IN_USEC(50), NULL,0,nmi_callback);	/* kludge until the K053260 is emulated correctly */
}

static READ8_HANDLER( pip_r )
{
	return 0x7f;
}

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0020, 0x0020) AM_READ(watchdog_reset_r)
	AM_RANGE(0x0030, 0x0031) AM_READ(rollerg_sound_r)	/* K053260 */
	AM_RANGE(0x0050, 0x0050) AM_READ_PORT("P1")
	AM_RANGE(0x0051, 0x0051) AM_READ_PORT("P2")
	AM_RANGE(0x0052, 0x0052) AM_READ_PORT("DSW3")
	AM_RANGE(0x0053, 0x0053) AM_READ_PORT("DSW1")
	AM_RANGE(0x0060, 0x0060) AM_READ_PORT("DSW2")
	AM_RANGE(0x0061, 0x0061) AM_READ(pip_r)				/* ????? */
	AM_RANGE(0x0300, 0x030f) AM_READ(K053244_r)
	AM_RANGE(0x0800, 0x0fff) AM_READ(rollerg_K051316_r)
	AM_RANGE(0x1000, 0x17ff) AM_READ(K053245_r)
	AM_RANGE(0x1800, 0x1fff) AM_READ(SMH_RAM)
	AM_RANGE(0x2000, 0x3aff) AM_READ(SMH_RAM)
	AM_RANGE(0x4000, 0x7fff) AM_READ(SMH_BANK1)
	AM_RANGE(0x8000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0010, 0x0010) AM_WRITE(rollerg_0010_w)
	AM_RANGE(0x0020, 0x0020) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x0030, 0x0031) AM_WRITE(k053260_0_w)
	AM_RANGE(0x0040, 0x0040) AM_WRITE(soundirq_w)
	AM_RANGE(0x0100, 0x010f) AM_WRITE(SMH_NOP)	/* 053252? */
	AM_RANGE(0x0200, 0x020f) AM_WRITE(K051316_ctrl_0_w)
	AM_RANGE(0x0300, 0x030f) AM_WRITE(K053244_w)
	AM_RANGE(0x0800, 0x0fff) AM_WRITE(K051316_0_w)
	AM_RANGE(0x1000, 0x17ff) AM_WRITE(K053245_w)
	AM_RANGE(0x1800, 0x1fff) AM_WRITE(paletteram_xBBBBBGGGGGRRRRR_be_w) AM_BASE(&paletteram)
	AM_RANGE(0x2000, 0x3aff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x4000, 0xffff) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(SMH_RAM)
	AM_RANGE(0xa000, 0xa02f) AM_READ(k053260_0_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(ym3812_status_port_0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xa000, 0xa02f) AM_WRITE(k053260_0_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(ym3812_control_port_0_w)
	AM_RANGE(0xc001, 0xc001) AM_WRITE(ym3812_write_port_0_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(sound_arm_nmi_w)
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( rollerg )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
//  PORT_DIPSETTING(    0x00, "Disabled" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, "Bonus Energy" )
	PORT_DIPSETTING(    0x00, "1/2 for Stage Winner" )
	PORT_DIPSETTING(    0x08, "1/4 for Stage Winner" )
	PORT_DIPSETTING(    0x10, "1/4 for Cycle Winner" )
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END



/***************************************************************************

    Machine Driver

***************************************************************************/

static MACHINE_DRIVER_START( rollerg )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", KONAMI, 3000000)		/* ? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("audio", Z80, 3579545)
	MDRV_CPU_PROGRAM_MAP(readmem_sound,writemem_sound)
								/* NMIs are generated by the 053260 */
	MDRV_MACHINE_RESET(rollerg)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(rollerg)
	MDRV_VIDEO_UPDATE(rollerg)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM3812, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("konami", K053260, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_DRIVER_END



/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( rollerg )
	ROM_REGION( 0x28000, "main", 0 ) /* code + banked roms */
	ROM_LOAD( "999m02.g7",  0x10000, 0x18000, CRC(3df8db93) SHA1(10c46d53d11b12b8f7cc6417601baef4638c1efe) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "999m01.e11", 0x0000, 0x8000, CRC(1fcfb22f) SHA1(ef058a7de6ba7cf310b91975345113acc6078f8a) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "999h06.k2",  0x000000, 0x100000, CRC(eda05130) SHA1(b52073a4a4651035d5f1e112601ceb2d004b2143) ) /* sprites */
	ROM_LOAD( "999h05.k8",  0x100000, 0x100000, CRC(5f321c7d) SHA1(d60a3480891b83ac109f2fecfe2b958bac310c15) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "999h03.d23", 0x000000, 0x040000, CRC(ea1edbd2) SHA1(a17d19f873384287e1e47222d46274e7408b40d4) ) /* zoom */
	ROM_LOAD( "999h04.f23", 0x040000, 0x040000, CRC(c1a35355) SHA1(615606d30500a8f2be19171893e985b085fff2fc) )

	ROM_REGION( 0x80000, "konami", 0 )	/* samples for 053260 */
	ROM_LOAD( "999h09.c5",  0x000000, 0x080000, CRC(c5188783) SHA1(d9ab69e4197ba2b42e3b0bb713236c8037fc2ab3) )
ROM_END

ROM_START( rollergj )
	ROM_REGION( 0x28000, "main", 0 ) /* code + banked roms */
	ROM_LOAD( "999v02.bin", 0x10000, 0x18000, CRC(0dd8c3ac) SHA1(4c3d5514dec317c6640ceaaa06411766632f4412) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "999m01.e11", 0x0000, 0x8000, CRC(1fcfb22f) SHA1(ef058a7de6ba7cf310b91975345113acc6078f8a) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "999h06.k2",  0x000000, 0x100000, CRC(eda05130) SHA1(b52073a4a4651035d5f1e112601ceb2d004b2143) ) /* sprites */
	ROM_LOAD( "999h05.k8",  0x100000, 0x100000, CRC(5f321c7d) SHA1(d60a3480891b83ac109f2fecfe2b958bac310c15) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "999h03.d23", 0x000000, 0x040000, CRC(ea1edbd2) SHA1(a17d19f873384287e1e47222d46274e7408b40d4) ) /* zoom */
	ROM_LOAD( "999h04.f23", 0x040000, 0x040000, CRC(c1a35355) SHA1(615606d30500a8f2be19171893e985b085fff2fc) )

	ROM_REGION( 0x80000, "konami", 0 )	/* samples for 053260 */
	ROM_LOAD( "999h09.c5",  0x000000, 0x080000, CRC(c5188783) SHA1(d9ab69e4197ba2b42e3b0bb713236c8037fc2ab3) )
ROM_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

static KONAMI_SETLINES_CALLBACK( rollerg_banking )
{
	UINT8 *RAM = memory_region(device->machine, "main");
	int offs = 0;


	offs = 0x10000 + ((lines & 0x07) * 0x4000);
	if (offs >= 0x28000) offs -= 0x20000;
	memory_set_bankptr(device->machine, 1,&RAM[offs]);
}

static MACHINE_RESET( rollerg )
{
	konami_configure_set_lines(machine->cpu[0], rollerg_banking);

	readzoomroms = 0;
}

static DRIVER_INIT( rollerg )
{
	konami_rom_deinterleave_2(machine, "gfx1");
}



GAME( 1991, rollerg,  0,       rollerg, rollerg, rollerg, ROT0, "Konami", "Rollergames (US)", 0 )
GAME( 1991, rollergj, rollerg, rollerg, rollerg, rollerg, ROT0, "Konami", "Rollergames (Japan)", 0 )
