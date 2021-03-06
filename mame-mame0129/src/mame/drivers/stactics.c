/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

Master processor - Intel 8080A

Memory Map:

0000-2fff ROM                       R
4000-47ff RAM                       R/W
5000-5fff switches/status           R
6000-6fff dips sw                   R
6000-600f Coin rjct/palette select  W
6010-601f sound triggers            W
6020-602f lamp latch                W
6030-603f speed latch               W
6040-605f shot related              W
6060-606f score display             W
60a0-60e0 sound triggers2           W
7000-7fff RNG/swit                  R     LS Nibble are a VBlank counter
                                          used as a RNG
8000-8fff swit/stat                 R
8000-8fff offset RAM                W
9000-9fff V pos reg.                R     Reads counter from an encoder wheel
a000-afff H pos reg.                R     Reads counter from an encoder wheel
b000-bfff VRAM B                    R/W   alphanumerics, bases, barrier,
                                          enemy bombs
d000-dfff VRAM D                    R/W   furthest aliens (scrolling)
e000-efff VRAM E                    R/W   middle aliens (scrolling)
f000-ffff VRAM F                    R/W   closest aliens (scrolling)

--------------------------------------------------------------------

At this time, emulation is missing:

Sound (all discrete and a 76477)
Verify Color PROM resistor values (Last 8 colors)

****************************************************************************/

#include "driver.h"
#include "cpu/i8085/i8085.h"
#include "stactics.h"
#include "stactics.lh"



/*************************************
 *
 *  Mirror motor handling
 *
 *************************************/

static CUSTOM_INPUT( get_motor_not_ready )
{
	stactics_state *state = field->port->machine->driver_data;

	/* if the motor is self-centering, but not centered yet */
    return ((*state->motor_on & 0x01) == 0) &&
    	   ((state->horiz_pos != 0) || (state->vert_pos != 0));
}


static READ8_HANDLER( vert_pos_r )
{
	stactics_state *state = space->machine->driver_data;

    return 0x70 - state->vert_pos;
}


static READ8_HANDLER( horiz_pos_r )
{
	stactics_state *state = space->machine->driver_data;

    return state->horiz_pos + 0x88;
}


static void move_motor(running_machine *machine, stactics_state *state)
{
	 /* monitor motor under joystick control */
    if (*state->motor_on & 0x01)
    {
		int ip3 = input_port_read(machine, "IN3");
		int ip4 = input_port_read(machine, "FAKE");

		/* up */
		if (((ip4 & 0x01) == 0) && (state->vert_pos > -128))
			state->vert_pos--;

		/* down */
		if (((ip4 & 0x02) == 0) && (state->vert_pos < 127))
			state->vert_pos++;

		/* left */
		if (((ip3 & 0x20) == 0) && (state->horiz_pos < 127))
			state->horiz_pos++;

		/* right */
		if (((ip3 & 0x40) == 0) && (state->horiz_pos > -128))
			state->horiz_pos--;
    }

	 /* monitor motor under self-centering control */
    else
    {
        if (state->horiz_pos > 0)
            state->horiz_pos--;
        else if (state->horiz_pos < 0)
            state->horiz_pos++;

        if (state->vert_pos > 0)
            state->vert_pos--;
        else if (state->vert_pos < 0)
            state->vert_pos++;
    }
}



/*************************************
 *
 *  Random number generator
 *
 *************************************/

static CUSTOM_INPUT( get_rng )
{
	/* this is a 555 timer, but cannot read one of the resistor values */
	return mame_rand(field->port->machine) & 0x07;
}



/*************************************
 *
 *  Coin lockout
 *
 *************************************/

static WRITE8_HANDLER( stactics_coin_lockout_w )
{
	coin_lockout_w(offset, ~data & 0x01);
}



/*************************************
 *
 *  Interrupt system
 *
 *************************************/

static INTERRUPT_GEN( stactics_interrupt )
{
	stactics_state *state = device->machine->driver_data;

	move_motor(device->machine, state);

    cpu_set_input_line(device, 0, HOLD_LINE);
}



/*************************************
 *
 *  Data CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
    AM_RANGE(0x0000, 0x2fff) AM_ROM
    AM_RANGE(0x4000, 0x47ff) AM_RAM
    AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_READ_PORT("IN0")
    AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_READ_PORT("IN1")
    AM_RANGE(0x6000, 0x6001) AM_MIRROR(0x0f08) AM_WRITE(stactics_coin_lockout_w)
    AM_RANGE(0x6002, 0x6005) AM_MIRROR(0x0f08) AM_WRITE(SMH_NOP)
    AM_RANGE(0x6006, 0x6007) AM_MIRROR(0x0f08) AM_WRITE(SMH_RAM) AM_BASE_MEMBER(stactics_state, palette)
 /* AM_RANGE(0x6010, 0x6017) AM_MIRROR(0x0f08) AM_WRITE(stactics_sound_w) */
    AM_RANGE(0x6016, 0x6016) AM_MIRROR(0x0f08) AM_WRITE(SMH_RAM) AM_BASE_MEMBER(stactics_state, motor_on)  /* Note: This overlaps rocket sound */
    AM_RANGE(0x6020, 0x6027) AM_MIRROR(0x0f08) AM_WRITE(SMH_RAM) AM_BASE_MEMBER(stactics_state, lamps)
    AM_RANGE(0x6030, 0x6030) AM_MIRROR(0x0f0f) AM_WRITE(stactics_speed_latch_w)
    AM_RANGE(0x6040, 0x6040) AM_MIRROR(0x0f0f) AM_WRITE(stactics_shot_trigger_w)
    AM_RANGE(0x6050, 0x6050) AM_MIRROR(0x0f0f) AM_WRITE(stactics_shot_flag_clear_w)
    AM_RANGE(0x6060, 0x606f) AM_MIRROR(0x0f00) AM_WRITE(SMH_RAM) AM_BASE_MEMBER(stactics_state, display_buffer)
    AM_RANGE(0x6070, 0x609f) AM_MIRROR(0x0f00) AM_WRITE(SMH_NOP)
 /* AM_RANGE(0x60a0, 0x60ef) AM_MIRROR(0x0f00) AM_WRITE(stactics_sound2_w) */
    AM_RANGE(0x60f0, 0x60ff) AM_MIRROR(0x0f00) AM_WRITE(SMH_NOP)
    AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0fff) AM_READ_PORT("IN2")
    AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x0fff) AM_READ_PORT("IN3")
    AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x0800) AM_WRITE(stactics_scroll_ram_w)
    AM_RANGE(0x9000, 0x9000) AM_MIRROR(0x0fff) AM_READ(vert_pos_r)
    AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x0fff) AM_READ(horiz_pos_r)
    AM_RANGE(0xb000, 0xbfff) AM_RAM AM_BASE_MEMBER(stactics_state, videoram_b)
    AM_RANGE(0xc000, 0xcfff) AM_NOP
    AM_RANGE(0xd000, 0xdfff) AM_RAM AM_BASE_MEMBER(stactics_state, videoram_d)
    AM_RANGE(0xe000, 0xefff) AM_RAM AM_BASE_MEMBER(stactics_state, videoram_e)
    AM_RANGE(0xf000, 0xffff) AM_RAM AM_BASE_MEMBER(stactics_state, videoram_f)
ADDRESS_MAP_END



/*************************************
 *
 *  Input port definitions
 *
 *************************************/

static INPUT_PORTS_START( stactics )
    PORT_START("IN0")	/*  IN0 */
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 )
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 )
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 )
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_motor_not_ready, NULL)

    PORT_START("IN1")	/* IN1 */
    PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
    PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
    PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
    PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
    PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(    0x28, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
    PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
    PORT_DIPNAME( 0x40, 0x00, "High Score Initial Entry" )
    PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )

    PORT_START("IN2")	/* IN2 */
    PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_rng, NULL)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(stactics_get_frame_count_d3, NULL)
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )
    PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

    PORT_START("IN3")	/* IN3 */
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(stactics_get_shot_standby, NULL)
    PORT_DIPNAME( 0x04, 0x04, "Number of Barriers" )
    PORT_DIPSETTING(    0x04, "4" )
    PORT_DIPSETTING(    0x00, "6" )
    PORT_DIPNAME( 0x08, 0x08, "Bonus Barriers" )
    PORT_DIPSETTING(    0x08, "1" )
    PORT_DIPSETTING(    0x00, "2" )
    PORT_DIPNAME( 0x10, 0x00, "Extended Play" )
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(stactics_get_not_shot_arrive, NULL)

	PORT_START("FAKE")	/* FAKE */
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
INPUT_PORTS_END



/*************************************
 *
 *  Start
 *
 *************************************/

static MACHINE_START( stactics )
{
	stactics_state *state = machine->driver_data;

	state->vert_pos = 0;
	state->horiz_pos = 0;
	*state->motor_on = 0;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( stactics )

	MDRV_DRIVER_DATA(stactics_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("main", 8080, 1933560)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT("main", stactics_interrupt)

	MDRV_MACHINE_START(stactics)

	/* video hardware */
	MDRV_IMPORT_FROM(stactics_video)

	/* audio hardware */
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( stactics )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "epr-218x",     0x0000, 0x0800, CRC(b1186ad2) SHA1(88929a183ac0499619b3e07241f3b5a0c89bdab1) )
	ROM_LOAD( "epr-219x",     0x0800, 0x0800, CRC(3b86036d) SHA1(6ad5e14dcfdbc6d2a0a32ae7f18ce41ab4b51eec) )
	ROM_LOAD( "epr-220x",     0x1000, 0x0800, CRC(c58702da) SHA1(93936c46810722d435f9ddb0641defb741743dee) )
	ROM_LOAD( "epr-221x",     0x1800, 0x0800, CRC(e327639e) SHA1(024929b65c71eaeb6d234a14d7535a7d5b98b8d3) )
	ROM_LOAD( "epr-222y",     0x2000, 0x0800, CRC(24dd2bcc) SHA1(f77c59beccc1a77e3bfc2928ff532d6e221ff42d) )
	ROM_LOAD( "epr-223x",     0x2800, 0x0800, CRC(7fef0940) SHA1(5b2af55f75ef0130f9202b6a916a96dbd601fcfa) )

	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "pr54",         0x0000, 0x0800, CRC(9640bd6e) SHA1(dd12952a6591f2056ac1b5688dca0a3a2ef69f2d) )      /* color/priority PROM */
	ROM_LOAD( "pr55",         0x0800, 0x0800, CRC(f162673b) SHA1(83743780b6c1f8014df24fa0650000b7cb137d92) )      /* timing PROM (unused)    */
	ROM_LOAD( "pr65",         0x1000, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )      /* timing PROM (unused)    */
	ROM_LOAD( "pr66",         0x1020, 0x0020, CRC(78dcf300) SHA1(37034cc0cfa4a8ec47937a2a34b77ec56b387a9b) )      /* timing PROM (unused)    */

	ROM_REGION( 0x0820, "user1", 0 )
	ROM_LOAD( "epr-217",      0x0000, 0x0800, CRC(38259f5f) SHA1(1f4182ffc2d78fca22711526bb2ae2cfe040173c) )      /* LED fire beam data      */
	ROM_LOAD( "pr67",         0x0800, 0x0020, CRC(b27874e7) SHA1(c24bc78c4b2ae01aaed5d994ce2e7c5e0f2eece8) )      /* LED timing ROM (unused) */
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAMEL( 1981, stactics, 0, stactics, stactics, 0, ORIENTATION_FLIP_X, "Sega", "Space Tactics", GAME_NO_SOUND, layout_stactics )
