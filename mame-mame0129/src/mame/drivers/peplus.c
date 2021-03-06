/**********************************************************************************


    PLAYER'S EDGE PLUS (PE+)

    Driver by Jim Stolis.
    Layouts by Stephh.

    Special thanks to smf for I2C EEPROM support.

    --- Technical Notes ---

    Name:    Player's Edge Plus (PP0516) Double Bonus Draw Poker.
    Company: IGT - International Gaming Technology
    Year:    1987

    Hardware:

    CPU =  INTEL 83c02       ; I8052 compatible
    VIDEO = ROCKWELL 6545    ; CRTC6845 compatible
    SND =  AY-3-8912         ; AY8910 compatible

    History:

    This form of video poker machine has the ability to use different game roms.  The operator
    changes the game by placing the rom at U68 on the motherboard.  This driver is currently valid
    for the PP0516 game rom, but should work with all other compatible game roms as cpu, video,
    sound, and inputs is concerned.  Some games can share the same color prom and graphic roms,
    but this is not always the case.  It is best to confirm the game, color and graphic combinations.

    The game code runs in two different modes, game mode and operator mode.  Game mode is what a
    normal player would see when playing.  Operator mode is for the machine operator to adjust
    machine settings and view coin counts.  The upper door must be open in order to enter operator
    mode and so it should be mapped to an input bank if you wish to support it.  The operator
    has two additional inputs (jackpot reset and self-test) to navigate with, along with the
    normal buttons available to the player.

    A normal machine keeps all coin counts and settings in a battery-backed ram, and will
    periodically update an external eeprom for an even more secure backup.  This eeprom
    also holds the current game state in order to recover the player from a full power failure.


Additional notes
================

1) What are "set chips" ?

    They are meant to be used after you have already sucessfully put a new game in your machine.
    Lets say you have 'pepp0516' installed and you go through the setup. In a real machine,
    you may want to add a bill validator. The only way to do that is to un-socket the 'pepp0516'
    chip and put in the 'peset038' chip and then reboot the machine. Then this chip's program
    runs and you set the options and put the 'pepp0516' chip back in.

    The only way to simulate this is to fire up the 'pepp0516' game and set it up. Then exit the
    game and copy the pepp0516.nv file to peset038.nv, and then run the 'peset038' program.
    This is because they have to have the same eeprom and cmos data in memory to work. When you
    are done with the 'peset038' program, you copy the peset038.nv file back over the pepp0516.nv .
    'peset038' is just a utility program with one screen and 3 tested inputs.


2) Initialisation

  - Method 1 :
      * be sure the door is opened (if not, press 'O' by default)
      * "CMOS DATA" will be displayed
      * press the self-test button (default is 'K')
      * be sure the door is opened (if not, press 'O' by default)
      * "EEPROM DATA" will be displayed
      * press the self-test button (default is 'K')
      * be sure the door is closed (if not, press 'O' by default)

  - Method 2 :
      * be sure the door is opened (if not, press 'O' by default)
      * "CMOS DATA" will be displayed
      * press the self-test button (default is 'K') until a "beep" is heard
      * be sure the door is closed (if not, press 'O' by default)
      * press the jackpot reset button (default is 'L')
      * be sure the door is opened (if not, press 'O' by default)
      * "EEPROM DATA" will be displayed
      * press the self-test button (default is 'K')
      * be sure the door is closed (if not, press 'O' by default)


          gamename  method
          --------  ------
          pepp0065     1
          pepp0158     2
          pepp0188     1
          pepp0250     1
          pepp0447     2
          pepp0516     1
          peps0014     1
          peps0022     1
          peps0043     1
          peps0045     1
          peps0308     1
          pebe0014     1
          peke1012     1
          peps0615     2
          peps0716     2
          pex2069p     2
          pexp0019     2
          pexp0112     2
          pexs0006     2
          pexmp006     2


2) Configuration

  - To configure a game :
      * be sure the door is opened (if not, press 'O' by default)
      * press the self-test button (default is 'K')
      * cycle through the screens with the self-test button (default is 'K')
      * close the door (default is 'O') to go back to the game and save the settings

2a) What are "set chips" ?

    They are meant to be used after you have already sucessfully put a new game in your machine.
    Lets say you have 'pepp0516' installed and you go through the setup. In a real machine,
    you may want to add a bill validator. The only way to do that is to un-socket the 'pepp0516'
    chip and put in the 'peset038' chip and then reboot the machine. Then this chip's program
    runs and you set the options and put the 'pepp0516' chip back in.

    The only way to simulate this is to fire up the 'pepp0516' game and set it up. Then exit the
    game and copy the pepp0516.nv file to peset038.nv, and then run the 'peset038' program.
    This is because they have to have the same eeprom and cmos data in memory to work. When you
    are done with the peset038 program, you copy the peset038.nv file back over the pepp0516.nv .
    'peset038' is just a utility program with one screen.

2b) About the "autohold" feature

    Depending on laws which vary from cities/country, this feature can available or not in the
    "operator mode". By default, it isn't available. To have this feature available in the
    "operator mode", a new chip has to be burnt with a bit set and a new checksum (game ID
    doesn't change though). Once the feature is available, it can be decided to turn the
    "autohold" option ON or OFF (default is OFF).
    To avoid having too many clones in MAME, a fake "Enable Autohold Feature" configuration
    option has been added for (poker) games that support it. If you change this option,
    you must leave the game, delete the .nv file, initialise and configure the game again.



Stephh's log (2007.11.28) :
  - split old peplus.c to peplus.c and pe_drvr.c (same as it was done for the NEOGEO)
  - Renamed sets :
      * 'peplus'   -> 'pepp0516' (so we get the game ID as for the other games)
  - added MACHINE_RESET definition (needed for fake "Enable Autohold Feature" configuration)
  - added generic/default layout, inputs and outputs
  - for each kind of game (poker, bjack, keno, slots) :
      * added two layouts (default is the "Bezel Lamps" for players, the other is "Debug Lamps")
      * added one INPUT_PORT definition
  - added fake "Enable Autohold Feature" configuration option for poker games that allow it
    and added a specific INPUT_PORT definition
  - for "set chips" :
      * added one fake layout
      * added one fake INPUT_PORT definition

***********************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "video/mc6845.h"

#include "peplus.lh"
#include "pe_schip.lh"
#include "pe_poker.lh"
#include "pe_bjack.lh"
#include "pe_keno.lh"
#include "pe_slots.lh"

#define MASTER_CLOCK 		XTAL_20MHz
#define CPU_CLOCK			((MASTER_CLOCK)/2)		/* divided by 2 - 7474 */
#define MC6845_CLOCK		((MASTER_CLOCK)/8/3)

static UINT16 autohold_addr; /* address to patch in program RAM to enable autohold feature */

static tilemap *bg_tilemap;

/* Pointers to External RAM */
static UINT8 *program_ram;
static UINT8 *cmos_ram;
static UINT8 *s3000_ram;
static UINT8 *s5000_ram;
static UINT8 *s7000_ram;
static UINT8 *sb000_ram;
static UINT8 *sd000_ram;
static UINT8 *sf000_ram;

/* Variables used instead of CRTC6845 system */
static UINT16 vid_address = 0;

/* Holds upper video address and palette number */
static UINT8 *palette_ram;

/* IO Ports */
static UINT8 *io_port;

/* Coin, Door, Hopper and EEPROM States */
static UINT64 last_cycles;
static UINT8 coin_state = 0;
static UINT64 last_door;
static UINT8 door_open = 0;
static UINT64 last_coin_out;
static UINT8 coin_out_state = 0;
static int sda_dir = 0;

/* Static Variables */
#define CMOS_NVRAM_SIZE     0x2000
#define eeprom_NVRAM_SIZE   0x200 // 4k Bit

/* prototypes */
static MC6845_ON_VSYNC_CHANGED(crtc_vsync);
static MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);

static const mc6845_interface mc6845_intf =
{
	"main",					/* screen we are acting on */
	8,						/* number of pixels per video memory address */
	NULL,					/* before pixel update callback */
	NULL,					/* row update callback */
	NULL,					/* after pixel update callback */
	NULL,					/* callback for display state changes */
	NULL,					/* HSYNC callback */
	crtc_vsync,				/* VSYNC callback */
	crtc_addr				/* update address callback */
};

/*****************
* NVRAM Handlers *
******************/

static NVRAM_HANDLER( peplus )
{
	if (read_or_write)
	{
		mame_fwrite(file, cmos_ram, CMOS_NVRAM_SIZE);
	}
	else
	{
		if (file)
		{
			mame_fread(file, cmos_ram, CMOS_NVRAM_SIZE);
		}
		else
		{
			memset(cmos_ram, 0, CMOS_NVRAM_SIZE);
		}
	}

	NVRAM_HANDLER_CALL(i2cmem_0);
}

/*****************
* Write Handlers *
******************/

static WRITE8_HANDLER( peplus_bgcolor_w )
{
	int i;

	for (i = 0; i < 16; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (~data >> 0) & 0x01;
		bit1 = (~data >> 1) & 0x01;
		bit2 = (~data >> 2) & 0x01;
		r = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* green component */
		bit0 = (~data >> 3) & 0x01;
		bit1 = (~data >> 4) & 0x01;
		bit2 = (~data >> 5) & 0x01;
		g = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* blue component */
		bit0 = (~data >> 6) & 0x01;
		bit1 = (~data >> 7) & 0x01;
		bit2 = 0;
		b = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		palette_set_color(space->machine, (15 + (i*16)), MAKE_RGB(r, g, b));
	}
}

/*
    ROCKWELL 6545 - Transparent Memory Addressing
    The current CRTC6845 driver does not support these
    additional registers (R18, R19, R31)
*/

static MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr)
{
	vid_address = address;
}


static WRITE8_DEVICE_HANDLER( peplus_crtc_mode_w )
{
	/* Reset timing logic */
}

static TIMER_CALLBACK(assert_lp_cb)
{
	mc6845_assert_light_pen_input((const device_config *) ptr);
}

static void handle_lightpen( const device_config *device )
{
    int x_val = input_port_read_safe(device->machine, "TOUCH_X",0x00);
    int y_val = input_port_read_safe(device->machine, "TOUCH_Y",0x00);
    const rectangle *vis_area = video_screen_get_visible_area(device->machine->primary_screen);
    int xt, yt;

    xt = x_val * (vis_area->max_x - vis_area->min_x) / 1024 + vis_area->min_x;
    yt = y_val * (vis_area->max_y - vis_area->min_y) / 1024 + vis_area->min_y;

     timer_set(device->machine, video_screen_get_time_until_pos(device->machine->primary_screen, yt, xt), (void *) device, 0, assert_lp_cb);
 }

static MC6845_ON_VSYNC_CHANGED(crtc_vsync)
{
	cputag_set_input_line(device->machine, "main", 0, vsync ? ASSERT_LINE : CLEAR_LINE);
	handle_lightpen(device);
}


static WRITE8_DEVICE_HANDLER( peplus_crtc_display_w )
{
	videoram[vid_address] = data;
	palette_ram[vid_address] = io_port[1];
	tilemap_mark_tile_dirty(bg_tilemap, vid_address);

	/* An access here triggers a device read !*/
	(void) mc6845_register_r(device, 0);
}

static WRITE8_HANDLER( peplus_io_w )
{
	io_port[offset] = data;
}

static WRITE8_HANDLER( peplus_duart_w )
{
	// Used for Slot Accounting System Communication
}

static WRITE8_HANDLER( peplus_cmos_w )
{
	cmos_ram[offset] = data;
}

static WRITE8_HANDLER( peplus_s3000_w )
{
	s3000_ram[offset] = data;
}

static WRITE8_HANDLER( peplus_s5000_w )
{
	s5000_ram[offset] = data;
}

static WRITE8_HANDLER( peplus_s7000_w )
{
	s7000_ram[offset] = data;
}

static WRITE8_HANDLER( peplus_sb000_w )
{
	sb000_ram[offset] = data;
}

static WRITE8_HANDLER( peplus_sd000_w )
{
	sd000_ram[offset] = data;
}

static WRITE8_HANDLER( peplus_sf000_w )
{
	sf000_ram[offset] = data;
}

static WRITE8_HANDLER( peplus_output_bank_a_w )
{
	output_set_value("pe_bnka0",(data >> 0) & 1); /* Coin Lockout */
	output_set_value("pe_bnka1",(data >> 1) & 1); /* Diverter */
	output_set_value("pe_bnka2",(data >> 2) & 1); /* Bell */
	output_set_value("pe_bnka3",(data >> 3) & 1); /* N/A */
	output_set_value("pe_bnka4",(data >> 4) & 1); /* Hopper 1 */
	output_set_value("pe_bnka5",(data >> 5) & 1); /* Hopper 2 */
	output_set_value("pe_bnka6",(data >> 6) & 1); /* specific to a kind of machine */
	output_set_value("pe_bnka7",(data >> 7) & 1); /* specific to a kind of machine */

    coin_out_state = 0;
    if(((data >> 4) & 1) || ((data >> 5) & 1))
        coin_out_state = 3;
}

static WRITE8_HANDLER( peplus_output_bank_b_w )
{
	output_set_value("pe_bnkb0",(data >> 0) & 1); /* specific to a kind of machine */
	output_set_value("pe_bnkb1",(data >> 1) & 1); /* Deal Spin Start */
	output_set_value("pe_bnkb2",(data >> 2) & 1); /* Cash Out */
	output_set_value("pe_bnkb3",(data >> 3) & 1); /* specific to a kind of machine */
	output_set_value("pe_bnkb4",(data >> 4) & 1); /* Bet 1 / Bet Max */
	output_set_value("pe_bnkb5",(data >> 5) & 1); /* Change Request */
	output_set_value("pe_bnkb6",(data >> 6) & 1); /* Door Open */
	output_set_value("pe_bnkb7",(data >> 7) & 1); /* specific to a kind of machine */
}

static WRITE8_HANDLER( peplus_output_bank_c_w )
{
	output_set_value("pe_bnkc0",(data >> 0) & 1); /* Coin In Meter */
	output_set_value("pe_bnkc1",(data >> 1) & 1); /* Coin Out Meter */
	output_set_value("pe_bnkc2",(data >> 2) & 1); /* Coin Drop Meter */
	output_set_value("pe_bnkc3",(data >> 3) & 1); /* Jackpot Meter */
	output_set_value("pe_bnkc4",(data >> 4) & 1); /* Bill Acceptor Enabled */
	output_set_value("pe_bnkc5",(data >> 5) & 1); /* SDS Out */
	output_set_value("pe_bnkc6",(data >> 6) & 1); /* N/A */
	output_set_value("pe_bnkc7",(data >> 7) & 1); /* Game Meter */
}

static WRITE8_HANDLER(i2c_nvram_w)
{
	i2cmem_write(space->machine, 0, I2CMEM_SCL, BIT(data, 2));
	sda_dir = BIT(data, 1);
	i2cmem_write(space->machine, 0, I2CMEM_SDA, BIT(data, 0));
}


/****************
* Read Handlers *
****************/

static READ8_HANDLER( peplus_io_r )
{
    return io_port[offset];
}

static READ8_HANDLER( peplus_duart_r )
{
	// Used for Slot Accounting System Communication
	return 0x00;
}

static READ8_HANDLER( peplus_cmos_r )
{
	return cmos_ram[offset];
}

static READ8_HANDLER( peplus_s3000_r )
{
	return s3000_ram[offset];
}

static READ8_HANDLER( peplus_s5000_r )
{
	return s5000_ram[offset];
}

static READ8_HANDLER( peplus_s7000_r )
{
	return s7000_ram[offset];
}

static READ8_HANDLER( peplus_sb000_r )
{
	return sb000_ram[offset];
}

static READ8_HANDLER( peplus_sd000_r )
{
	return sd000_ram[offset];
}

static READ8_HANDLER( peplus_sf000_r )
{
	return sf000_ram[offset];
}

/* Last Color in Every Palette is bgcolor */
static READ8_HANDLER( peplus_bgcolor_r )
{
	return palette_get_color(space->machine, 15); // Return bgcolor from First Palette
}

static READ8_HANDLER( peplus_dropdoor_r )
{
	return 0x00; // Drop Door 0x00=Closed 0x02=Open
}

static READ8_HANDLER( peplus_watchdog_r )
{
	return 0x00; // Watchdog
}

static READ8_HANDLER( peplus_input_bank_a_r )
{
/*
        Bit 0 = COIN DETECTOR A
        Bit 1 = COIN DETECTOR B
        Bit 2 = COIN DETECTOR C
        Bit 3 = COIN OUT
        Bit 4 = HOPPER FULL
        Bit 5 = DOOR OPEN
        Bit 6 = LOW BATTERY
        Bit 7 = I2C EEPROM SDA
*/
	UINT8 bank_a = 0x50; // Turn Off Low Battery and Hopper Full Statuses
	UINT8 coin_optics = 0x00;
    UINT8 coin_out = 0x00;
	UINT64 curr_cycles = cpu_get_total_cycles(space->cpu);

	UINT8 sda = 0;
	if(!sda_dir)
	{
		sda = i2cmem_read(space->machine, 0, I2CMEM_SDA);
	}

	if ((input_port_read_safe(space->machine, "SENSOR",0x00) & 0x01) == 0x01 && coin_state == 0) {
		coin_state = 1; // Start Coin Cycle
		last_cycles = cpu_get_total_cycles(space->cpu);
	} else {
		/* Process Next Coin Optic State */
		if (curr_cycles - last_cycles > 600000/6 && coin_state != 0) {
			coin_state++;
			if (coin_state > 5)
				coin_state = 0;
			last_cycles = cpu_get_total_cycles(space->cpu);
		}
	}

	switch (coin_state)
	{
		case 0x00: // No Coin
			coin_optics = 0x00;
			break;
		case 0x01: // Optic A
			coin_optics = 0x01;
			break;
		case 0x02: // Optic AB
			coin_optics = 0x03;
			break;
		case 0x03: // Optic ABC
			coin_optics = 0x07;
			break;
		case 0x04: // Optic BC
			coin_optics = 0x06;
			break;
		case 0x05: // Optic C
			coin_optics = 0x04;
			break;
	}

	if (curr_cycles - last_door > 6000/12) { // Guessing with 6000
		if ((input_port_read_safe(space->machine, "DOOR",0xff) & 0x01) == 0x01) {
			door_open = (!door_open & 0x01);
		} else {
			door_open = 1;
		}
		last_door = cpu_get_total_cycles(space->cpu);
	}

	if (curr_cycles - last_coin_out > 600000/12 && coin_out_state != 0) { // Guessing with 600000
		if (coin_out_state != 2) {
            coin_out_state = 2; // Coin-Out Off
        } else {
            coin_out_state = 3; // Coin-Out On
        }

		last_coin_out = cpu_get_total_cycles(space->cpu);
	}

    switch (coin_out_state)
    {
        case 0x00: // No Coin-Out
	        coin_out = 0x00;
	        break;
        case 0x01: // First Coin-Out On
	        coin_out = 0x08;
	        break;
        case 0x02: // Coin-Out Off
	        coin_out = 0x00;
	        break;
        case 0x03: // Additional Coin-Out On
	        coin_out = 0x08;
	        break;
    }

	bank_a = (sda<<7) | bank_a | (door_open<<5) | coin_optics | coin_out;

	return bank_a;
}


/****************************
* Video/Character functions *
****************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int pr = palette_ram[tile_index];
	int vr = videoram[tile_index];

	int code = ((pr & 0x0f)*256) | vr;
	int color = (pr>>4) & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START( peplus )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 40, 25);
	palette_ram = auto_malloc(0x3000);
	memset(palette_ram, 0, 0x3000);
}

static VIDEO_UPDATE( peplus )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	return 0;
}

static PALETTE_INIT( peplus )
{
/*  prom bits
    7654 3210
    ---- -xxx   red component.
    --xx x---   green component.
    xx-- ----   blue component.
*/
	int i;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (~color_prom[i] >> 0) & 0x01;
		bit1 = (~color_prom[i] >> 1) & 0x01;
		bit2 = (~color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* green component */
		bit0 = (~color_prom[i] >> 3) & 0x01;
		bit1 = (~color_prom[i] >> 4) & 0x01;
		bit2 = (~color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* blue component */
		bit0 = (~color_prom[i] >> 6) & 0x01;
		bit1 = (~color_prom[i] >> 7) & 0x01;
		bit2 = 0;
		b = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8x8 characters */
	0x1000, /* 4096 characters */
	4,  /* 4 bitplanes */
	{ 0x1000*8*8*3, 0x1000*8*8*2, 0x1000*8*8*1, 0x1000*8*8*0 }, /* bitplane offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( peplus )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 0, 16 )
GFXDECODE_END


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( peplus_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_BASE(&program_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( peplus_iomap, ADDRESS_SPACE_IO, 8 )
	// Battery-backed RAM (0x1000-0x01fff Extended RAM for Superboards Only)
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(peplus_cmos_r, peplus_cmos_w) AM_BASE(&cmos_ram)

	// CRT Controller
	AM_RANGE(0x2008, 0x2008) AM_DEVWRITE(R6545_1, "crtc", peplus_crtc_mode_w)
	AM_RANGE(0x2080, 0x2080) AM_DEVREADWRITE(R6545_1, "crtc", mc6845_status_r, mc6845_address_w)
	AM_RANGE(0x2081, 0x2081) AM_DEVREADWRITE(R6545_1, "crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x2083, 0x2083) AM_DEVREADWRITE(R6545_1, "crtc", mc6845_register_r, peplus_crtc_display_w)

    // Superboard Data
	AM_RANGE(0x3000, 0x3fff) AM_READWRITE(peplus_s3000_r, peplus_s3000_w) AM_BASE(&s3000_ram)

	// Sound and Dipswitches
	AM_RANGE(0x4000, 0x4000) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0x4004, 0x4004) AM_READ_PORT("SW1") AM_WRITE(ay8910_write_port_0_w)

    // Superboard Data
	AM_RANGE(0x5000, 0x5fff) AM_READWRITE(peplus_s5000_r, peplus_s5000_w) AM_BASE(&s5000_ram)

	// Background Color Latch
	AM_RANGE(0x6000, 0x6000) AM_READ(peplus_bgcolor_r) AM_WRITE(peplus_bgcolor_w)

    // Bogus Location for Video RAM
	AM_RANGE(0x06001, 0x06400) AM_RAM AM_BASE(&videoram)

    // Superboard Data
	AM_RANGE(0x7000, 0x7fff) AM_READWRITE(peplus_s7000_r, peplus_s7000_w) AM_BASE(&s7000_ram)

	// Input Bank A, Output Bank C
	AM_RANGE(0x8000, 0x8000) AM_READ(peplus_input_bank_a_r) AM_WRITE(peplus_output_bank_c_w)

	// Drop Door, I2C EEPROM Writes
	AM_RANGE(0x9000, 0x9000) AM_READ(peplus_dropdoor_r) AM_WRITE(i2c_nvram_w)

	// Input Banks B & C, Output Bank B
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0") AM_WRITE(peplus_output_bank_b_w)

    // Superboard Data
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE(peplus_sb000_r, peplus_sb000_w) AM_BASE(&sb000_ram)

	// Output Bank A
	AM_RANGE(0xc000, 0xc000) AM_READ(peplus_watchdog_r) AM_WRITE(peplus_output_bank_a_w)

    // Superboard Data
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(peplus_sd000_r, peplus_sd000_w) AM_BASE(&sd000_ram)

	// DUART
	AM_RANGE(0xe000, 0xe00f) AM_READWRITE(peplus_duart_r, peplus_duart_w)

    // Superboard Data
	AM_RANGE(0xf000, 0xffff) AM_READWRITE(peplus_sf000_r, peplus_sf000_w) AM_BASE(&sf000_ram)

	/* Ports start here */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READ(peplus_io_r) AM_WRITE(peplus_io_w) AM_BASE(&io_port)
ADDRESS_MAP_END

/*************************
*      Input ports       *
*************************/

static CUSTOM_INPUT( peplus_input_r )
{
	UINT8 inp_ret = 0x00;
	UINT8 inp_read = input_port_read(field->port->machine, param);

	if (inp_read & 0x01) inp_ret = 0x01;
	if (inp_read & 0x02) inp_ret = 0x02;
	if (inp_read & 0x04) inp_ret = 0x03;
	if (inp_read & 0x08) inp_ret = 0x04;
	if (inp_read & 0x10) inp_ret = 0x05;
	if (inp_read & 0x20) inp_ret = 0x06;
	if (inp_read & 0x40) inp_ret = 0x07;

	return inp_ret;
}

static INPUT_PORTS_START( peplus )
	/* IN0 has to be defined for each kind of game */

	PORT_START("DOOR")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Upper Door") PORT_CODE(KEYCODE_O) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Lower Door") PORT_CODE(KEYCODE_I)

	PORT_START("SENSOR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin In") PORT_IMPULSE(1)

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Line Frequency" )
	PORT_DIPSETTING(    0x01, "60Hz" )
	PORT_DIPSETTING(    0x00, "50Hz" )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_schip )
	PORT_INCLUDE(peplus)
	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_poker )
	PORT_INCLUDE(peplus)

	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("Bill Acceptor") PORT_CODE(KEYCODE_U)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_bjack )
	PORT_INCLUDE(peplus)

	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Surrender") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Stand") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Insurance") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Double Down") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Split") PORT_CODE(KEYCODE_B)

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("Bill Acceptor") PORT_CODE(KEYCODE_U)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_keno )
	PORT_INCLUDE(peplus)

	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Erase") PORT_CODE(KEYCODE_B)

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("Bill Acceptor") PORT_CODE(KEYCODE_U)

	PORT_START("TOUCH_X")
	PORT_BIT( 0xffff, 0x200, IPT_LIGHTGUN_X ) PORT_MINMAX(0x00, 1024) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)
	PORT_START("TOUCH_Y")
	PORT_BIT( 0xffff, 0x200, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x00, 1024) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Light Pen") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( peplus_slots )
	PORT_INCLUDE(peplus)

	PORT_START("IN_BANK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Jackpot Reset") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Self Test") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN_BANK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Deal-Spin-Start") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Play Credit") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("Change Request") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("Bill Acceptor") PORT_CODE(KEYCODE_U)

	PORT_START("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(peplus_input_r, "IN_BANK2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* same as peplus_poker with additionnal fake option to enable the "Autohold" feature */
static INPUT_PORTS_START( peplus_pokah )
	PORT_INCLUDE(peplus_poker)

	/* If you change this option, you'll have to delete the .nv file next time you launch the game ! */
	PORT_START("AUTOHOLD")
	PORT_CONFNAME( 0x01, 0x00, "Enable Autohold Feature" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Yes ) )
INPUT_PORTS_END


/*************************
*     Machine Reset      *
*************************/

static MACHINE_RESET( peplus )
{
/*
    AutoHold Feature Currently Disabled

    // pepp0158
    program_ram[0xa19f] = 0x22; // RET - Disable Memory Test
    program_ram[0xddea] = 0x22; // RET - Disable Program Checksum
    autohold_addr = 0x5ffe; // AutoHold Address

    // pepp0188
    program_ram[0x9a8d] = 0x22; // RET - Disable Memory Test
    program_ram[0xf429] = 0x22; // RET - Disable Program Checksum
    autohold_addr = 0x742f; // AutoHold Address

    // pepp0516
    program_ram[0x9a24] = 0x22; // RET - Disable Memory Test
    program_ram[0xd61d] = 0x22; // RET - Disable Program Checksum
    autohold_addr = 0x5e7e; // AutoHold Address

    if (autohold_addr)
        program_ram[autohold_addr] = input_port_read_safe(machine, "AUTOHOLD",0x00) & 0x01;
*/
}


/*************************
*     Machine Driver     *
*************************/

static MACHINE_DRIVER_START( peplus )
	// basic machine hardware
	MDRV_CPU_ADD("main", I80C32, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(peplus_map, 0)
	MDRV_CPU_IO_MAP(peplus_iomap, 0)

	MDRV_MACHINE_RESET(peplus)
	MDRV_NVRAM_HANDLER(peplus)

	// video hardware

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((52+1)*8, (31+1)*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 25*8-1)

	MDRV_GFXDECODE(peplus)
	MDRV_PALETTE_LENGTH(16*16)

	MDRV_MC6845_ADD("crtc", R6545_1, MC6845_CLOCK, mc6845_intf)

	MDRV_PALETTE_INIT(peplus)
	MDRV_VIDEO_START(peplus)
	MDRV_VIDEO_UPDATE(peplus)

	// sound hardware
	MDRV_SOUND_ADD("ay", AY8912, 20000000/12)
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END


/*****************
* Initialisation *
*****************/

/* Normal board */
static void peplus_init(running_machine *machine)
{
    /* EEPROM is a X2404P 4K-bit Serial I2C Bus */
	i2cmem_init(machine, 0, I2CMEM_SLAVE_ADDRESS, 8, eeprom_NVRAM_SIZE, NULL);

	/* default : no address to patch in program RAM to enable autohold feature */
	autohold_addr = 0;
}


/*************************
*      Driver Init       *
*************************/

/* Normal board */
static DRIVER_INIT( peplus )
{
	peplus_init(machine);
}

/* Superboard */
static DRIVER_INIT( peplussb )
{
    UINT8 *super_data = memory_region(machine, "user1");

    /* Distribute Superboard Data */
    memcpy(s3000_ram, &super_data[0x3000], 0x1000);
    memcpy(s5000_ram, &super_data[0x5000], 0x1000);
    memcpy(s7000_ram, &super_data[0x7000], 0x1000);
    memcpy(sb000_ram, &super_data[0xb000], 0x1000);
    memcpy(sd000_ram, &super_data[0xd000], 0x1000);
    memcpy(sf000_ram, &super_data[0xf000], 0x1000);

	peplus_init(machine);
}

/*************************
*        Rom Load        *
*************************/

ROM_START( peset038 ) /* Normal board : Set Chip (Set038) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "set038.u68",   0x00000, 0x10000, CRC(9c4b1d1a) SHA1(8a65cd1d8e2d74c7b66f4dfc73e7afca8458e979) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) )
ROM_END

ROM_START( pepp0065 ) /* Normal board : Jokers Wild Poker (PP0065) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "pp0065.u68",   0x00000, 0x10000, CRC(76c1a367) SHA1(ea8be9241e9925b5a4206db6875e1572f85fa5fe) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) )
ROM_END

ROM_START( pepp0158 ) /* Normal board : 4 of a Kind Bonus Poker (PP0158) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "pp0158.u68",   0x00000, 0x10000, CRC(5976cd19) SHA1(6a461ea9ddf78dffa3cf8b65903ebf3127f23d45) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) )
ROM_END

ROM_START( pepp0188 ) /* Normal board : Standard Draw Poker (PP0188) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "pp0188.u68",   0x00000, 0x10000, CRC(cf36a53c) SHA1(99b578538ab24d9ff91971b1f77599272d1dbfc6) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) )
ROM_END

ROM_START( pepp0250 ) /* Normal board : Double Down Stud Poker (PP0250) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "pp0250.u68",   0x00000, 0x10000, CRC(4c919598) SHA1(fe73503c6ccb3c5746fb96be58cd5b740c819713) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) )
ROM_END

ROM_START( pepp0447 ) /* Normal board : Standard Draw Poker (PP0447) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "pp0447.u68",   0x00000, 0x10000, CRC(0ef0bb6c) SHA1(d0ef7a83417054f05d32d0a93ed0d5d618f4dfb9) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) )
ROM_END

ROM_START( pepp0516 ) /* Normal board : Double Bonus Poker (PP0516) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "pp0516.u68",   0x00000, 0x10000, CRC(d9da6e13) SHA1(421678d9cb42daaf5b21074cc3900db914dd26cf) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg740.u72",	 0x00000, 0x8000, CRC(72667f6c) SHA1(89843f472cc0329317cfc643c63bdfd11234b194) )
	ROM_LOAD( "mgo-cg740.u73",	 0x08000, 0x8000, CRC(7437254a) SHA1(bba166dece8af58da217796f81117d0b05752b87) )
	ROM_LOAD( "mbo-cg740.u74",	 0x10000, 0x8000, CRC(92e8c33e) SHA1(05344664d6fdd3f4205c50fa4ca76fc46c18cf8f) )
	ROM_LOAD( "mxo-cg740.u75",	 0x18000, 0x8000, CRC(ce4cbe0b) SHA1(4bafcd68be94a5deaae9661584fa0fc940b834bb) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap740.u50", 0x0000, 0x0100, CRC(6fe619c4) SHA1(49e43dafd010ce0fe9b2a63b96a4ddedcb933c6d) )
ROM_END

ROM_START( pebe0014 ) /* Normal board : Blackjack (BE0014) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "be0014.u68",   0x00000, 0x10000, CRC(232b32b7) SHA1(a3af9414577642fedc23b4c1911901cd31e9d6e0) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg2036.u72",	 0x00000, 0x8000, CRC(0a168d06) SHA1(7ed4fb5c7bcacab077bcec030f0465c6eaf3ce1c) )
	ROM_LOAD( "mgo-cg2036.u73",	 0x08000, 0x8000, CRC(826b4090) SHA1(34390484c0faffe9340fd93d273b9292d09f97fd) )
	ROM_LOAD( "mbo-cg2036.u74",	 0x10000, 0x8000, CRC(46aac851) SHA1(28d84b49c6cebcf2894b5a15d935618f84093caa) )
	ROM_LOAD( "mxo-cg2036.u75",	 0x18000, 0x8000, CRC(60204a56) SHA1(2e3420da9e79ba304ca866d124788f84861380a7) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap707.u50", 0x0000, 0x0100, CRC(9851ba36) SHA1(5a0a43c1e212ae8c173102ede9c57a3d95752f99) )
ROM_END

ROM_START( peke1012 ) /* Normal board : Keno (KE1012) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ke1012.u68",   0x00000, 0x10000, CRC(470e8c10) SHA1(f8a65a3a73477e9e9d2f582eeefa93b470497dfa) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE ) // BAD DUMPS
	ROM_LOAD( "mro-cg1267.u72",	 0x00000, 0x8000, CRC(16498b57) SHA1(9c22726299af7204c4be1c6d8afc4c1b512ad918) )
	ROM_LOAD( "mgo-cg1267.u73",	 0x08000, 0x8000, CRC(80847c5a) SHA1(8422cd13a91c3c462af5efcfca8615e7eeaa2e52) )
	ROM_LOAD( "mbo-cg1267.u74",	 0x10000, 0x8000, CRC(ce7af8a7) SHA1(38675122c764b8fa9260246ea99ac0f0750da277) )
	ROM_LOAD( "mxo-cg1267.u75",	 0x18000, 0x8000, CRC(3aac0d4a) SHA1(764da54bdb2f2c49551cf1d10286de9450abad2f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1267.u50", 0x0000, 0x0100, CRC(7051db57) SHA1(76751a3cc47d506983205decb07e99ca0c178a42) )
ROM_END

ROM_START( peps0014 ) /* Normal board : Super Joker Slots (PS0014) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ps0014.u68",   0x00000, 0x10000, CRC(368c3f58) SHA1(ebefcefbb5386659680719936bff72ad61087343) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg0916.u72",	 0x00000, 0x8000, CRC(d97049d9) SHA1(78f7bb33866ca92922a8b83d5f9ac459edd39176) )
	ROM_LOAD( "mgo-cg0916.u73",	 0x08000, 0x8000, CRC(6e075788) SHA1(e8e9d8b7943d62e31d1d58f870bc765cba65c203) )
	ROM_LOAD( "mbo-cg0916.u74",	 0x10000, 0x8000, CRC(a5cdf0f3) SHA1(23b2749fd2cb5b8462ce7c912005779b611f32f9) )
	ROM_LOAD( "mxo-cg0916.u75",	 0x18000, 0x8000, CRC(1f3a2d72) SHA1(8e07324d436980b628e007d30a835757c1f70f6d) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap0916.u50", 0x0000, 0x0100, CRC(b9a5ee21) SHA1(d3c952f594baca9dc234602d90c506dd537c4dcc) )
ROM_END

ROM_START( peps0022 ) /* Normal board : Red White & Blue Slots (PS0022) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ps0022.u68",   0x00000, 0x10000, CRC(d65c0939) SHA1(d91f472a43f77f9df8845e97561540f988e522e3) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg0960.u72",	 0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",	 0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",	 0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",	 0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap0960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0043 ) /* Normal board : Double Diamond Slots (PS0043) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ps0043.u68",   0x00000, 0x10000, CRC(d612429c) SHA1(95eb4774482a930066456d517fb2e4f67d4df4cb) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg1003.u72",	 0x00000, 0x8000, CRC(41ce0395) SHA1(ae90dbae30e4efed33f83ee7038fb2e5171c1945) )
	ROM_LOAD( "mgo-cg1003.u73",	 0x08000, 0x8000, CRC(5a383fa1) SHA1(27b1febbdda7332e8d474fc0cca683f451a07090) )
	ROM_LOAD( "mbo-cg1003.u74",	 0x10000, 0x8000, CRC(5ec00224) SHA1(bb70a4326cd1810b200e193a449061df62085f37) )
	ROM_LOAD( "mxo-cg1003.u75",	 0x18000, 0x8000, CRC(2ffacd52) SHA1(38126ac4998806a1ddd55e6aa1942044240d41d0) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1003.u50", 0x0000, 0x0100, CRC(cc400805) SHA1(f5ac48ad2a5df64da150f09f2ea5d910230bde56) )
ROM_END

ROM_START( peps0045 ) /* Normal board : Red White & Blue Slots (PS0045) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ps0045.u68",   0x00000, 0x10000, CRC(de180b84) SHA1(0d592d7d535b0aacbd62c18ac222da770fab7b85) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg0960.u72",	 0x00000, 0x8000, CRC(8c38c6fd) SHA1(5d6e9ac18b9b3f1253bba080bef1c067b2fdd7a8) )
	ROM_LOAD( "mgo-cg0960.u73",	 0x08000, 0x8000, CRC(b4f44163) SHA1(1bc635a5160fdff2882c8362644aebf983a1a427) )
	ROM_LOAD( "mbo-cg0960.u74",	 0x10000, 0x8000, CRC(8057e3a8) SHA1(5510872b1607daaf890603e76a8a47680e639e8e) )
	ROM_LOAD( "mxo-cg0960.u75",	 0x18000, 0x8000, CRC(d57b4c25) SHA1(6ddfbaae87f9958642ddb95e581ac31e1dd55608) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap0960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0308 ) /* Normal board : Double Jackpot Slots (PS0308) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ps0308.u68",   0x00000, 0x10000, CRC(fe30e081) SHA1(d216cbc6336727caf359e6b178c856ab2659cabd) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg0911.u72",	 0x00000, 0x8000, CRC(48491b50) SHA1(9ec6d3ff34a08d40082a1347a46635838fd31afc) )
	ROM_LOAD( "mgo-cg0911.u73",	 0x08000, 0x8000, CRC(c1ff7d97) SHA1(78ab138ae9c7f9b3352f9b1ef5fbc473993bb8c8) )
	ROM_LOAD( "mbo-cg0911.u74",	 0x10000, 0x8000, CRC(202e0f9e) SHA1(51421dfd1b00a9e3b1e938d5bffaa3b7cd4c2b5e) )
	ROM_LOAD( "mxo-cg0911.u75",	 0x18000, 0x8000, CRC(d97740a2) SHA1(d76926d7fbbc24d2384a1079cb97e654600b134b) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap0911.u50", 0x0000, 0x0100, CRC(f117e781) SHA1(ba9d850c93e5f3abc26b0ba51f67fa7c07e05f59) )
ROM_END

ROM_START( peps0615 ) /* Normal board : Chaos Slots (PS0615) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ps0615.u68",   0x00000, 0x10000, CRC(d27dd6ab) SHA1(b3f065f507191682edbd93b07b72ed87bf6ae9b1) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg2246.u72",	 0x00000, 0x8000, CRC(7c08c355) SHA1(2a154b81c6d9671cea55a924bffb7f5461747142) )
	ROM_LOAD( "mgo-cg2246.u73",	 0x08000, 0x8000, CRC(b3c16487) SHA1(c97232fadd086f604eaeb3cd3c2d1c8fe0dcfa70) )
	ROM_LOAD( "mbo-cg2246.u74",	 0x10000, 0x8000, CRC(e61331f5) SHA1(4364edc625d64151cbae40780b54cb1981086647) )
	ROM_LOAD( "mxo-cg2246.u75",	 0x18000, 0x8000, CRC(f0f4a27d) SHA1(3a10ab196aeaa5b50d47b9d3c5b378cfadd6fe96) )

	ROM_REGION( 0x100, "proms", 0 ) // WRONG CAP
    ROM_LOAD( "cap0960.u50", 0x0000, 0x0100, CRC(00dd8d0a) SHA1(542763b12aeb0aec2b410f7c075c52907f45d171) )
ROM_END

ROM_START( peps0716 ) /* Normal board : River Gambler Slots (PS0716) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ps0716.u68",   0x00000, 0x10000, CRC(7615d7b6) SHA1(91fe62eec720a0dc2ebf48835065148f19499d16) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg2266.u72",	 0x00000, 0x8000, CRC(590accd8) SHA1(4e1c963c50799eaa49970e25ecf9cb01eb6b09e1) )
	ROM_LOAD( "mgo-cg2266.u73",	 0x08000, 0x8000, CRC(b87ffa05) SHA1(92126b670b9cabeb5e2cc35b6e9c458088b18eea) )
	ROM_LOAD( "mbo-cg2266.u74",	 0x10000, 0x8000, CRC(e3df30e1) SHA1(c7d2ae9a7c7e53bfb6197b635efcb5dc231e4fe0) )
	ROM_LOAD( "mxo-cg2266.u75",	 0x18000, 0x8000, CRC(56271442) SHA1(61ad0756b9f6412516e46ef6625a4c3899104d4e) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap2266.u50", 0x0000, 0x0100, CRC(5aaff103) SHA1(9cfda9c095cb77a8bb761c131a0f358e79b97abc) )
ROM_END

ROM_START( pex2069p ) /* Superboard : Double Double Bonus Poker (X002069P) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "xp000038.u67",   0x00000, 0x10000, CRC(8707ab9e) SHA1(3e00a2ad8017e1495c6d6fe900d0efa68a1772b8) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002069p.u66",   0x00000, 0x10000, CRC(875ecfcf) SHA1(80472cb43b36e518216e60a9b4883a81163718a2) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg2185.u77",	 0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) )
	ROM_LOAD( "mgo-cg2185.u78",	 0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",	 0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",	 0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap1321.u43", 0x0000, 0x0100, CRC(1e19c0ff) SHA1(e48ebc4a3c88e8b8c9740841dac919e9bb2c4b7f) )
ROM_END

ROM_START( pexp0019 ) /* Superboard : Deuces Wild Poker (XP000019) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "xp000019.u67",   0x00000, 0x10000, CRC(8ac876eb) SHA1(105b4aee2692ccb20795586ccbdf722c59db66cf) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002025p.u66",   0x00000, 0x10000, CRC(f3dac423) SHA1(e9394d330deb3b8a1001e57e72a506cd9098f161) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg2185.u77",	 0x00000, 0x8000, CRC(7e64bd1a) SHA1(e988a380ee58078bf5bdc7747e83aed1393cfad8) )
	ROM_LOAD( "mgo-cg2185.u78",	 0x08000, 0x8000, CRC(d4127893) SHA1(75039c45ba6fd171a66876c91abc3191c7feddfc) )
	ROM_LOAD( "mbo-cg2185.u79",	 0x10000, 0x8000, CRC(17dba955) SHA1(5f77379c88839b3a04e235e4fb0120c77e17b60e) )
	ROM_LOAD( "mxo-cg2185.u80",	 0x18000, 0x8000, CRC(583eb3b1) SHA1(4a2952424969917fb1594698a779fe5a1e99bff5) )

	ROM_REGION( 0x100, "proms", 0 ) // WRONG CAP
	ROM_LOAD( "cap2234.u43", 0x0000, 0x0100, CRC(a930e283) SHA1(61bce50fa13b3e980ece3e72d068835e19bd5049) )
ROM_END

ROM_START( pexp0112 ) /* Superboard : White Hot Aces Poker (XP000112) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "xp000112.u67",   0x00000, 0x10000, CRC(c1ae96ad) SHA1(da109602f0fbe9b225cdcd60be0613fd41014864) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x002035p.u66",   0x00000, 0x10000, CRC(dc3f0742) SHA1(d57cf3353b81f41895458019e47203f98645f17a) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg2324.u77",	 0x00000, 0x8000, CRC(6eceef42) SHA1(a2ddd2a3290c41e510f483c6b633fe0002694d0b) )
	ROM_LOAD( "mgo-cg2324.u78",	 0x08000, 0x8000, CRC(26d0acbe) SHA1(09a9127deb88185cd5b748bac657461eadb2f48f) )
	ROM_LOAD( "mbo-cg2324.u79",	 0x10000, 0x8000, CRC(47baee32) SHA1(d8af09022ccb5fc06aa3aa4c200a734b66cbee00) )
	ROM_LOAD( "mxo-cg2324.u80",	 0x18000, 0x8000, CRC(60449fc0) SHA1(251d1e04786b70c1d2bc7b02f3b69cd58ac76398) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap2174.u43", 0x0000, 0x0100, CRC(cbff3f26) SHA1(9e145676f2871c2369042a13cbeabb7efe2728e1) )
ROM_END

ROM_START( pexs0006 ) /* Superboard : Triple Triple Diamond Slots (XS000006) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "xs000006.u67",   0x00000, 0x10000, CRC(4b11ca18) SHA1(f64a1fbd089c01bc35a5484e60b8834a2db4a79f) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x000998s.u66",   0x00000, 0x10000, CRC(e29d4346) SHA1(93901ff65c8973e34ac1f0dd68bb4c4973da5621) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg2361.u77",	 0x00000, 0x8000, CRC(c0eba866) SHA1(8f217aeb3e8028a5633d95e5612f1b55e601650f) )
	ROM_LOAD( "mgo-cg2361.u78",	 0x08000, 0x8000, CRC(345eaea2) SHA1(18ebb94a323e1cf671201db8b9f85d4f30d8b5ec) )
	ROM_LOAD( "mbo-cg2361.u79",	 0x10000, 0x8000, CRC(fa130af6) SHA1(aca5e52e00bc75a4801fd3f6c564e62ed4251d8e) )
	ROM_LOAD( "mxo-cg2361.u80",	 0x18000, 0x8000, CRC(7de1812c) SHA1(c7e23a10f20fc8b618df21dd33ac456e1d2cfe33) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap2361.u43", 0x0000, 0x0100, CRC(051aea66) SHA1(2abf32caaeb821ca50a6398581de69bbfe5930e9) )
ROM_END

ROM_START( pexmp006 ) /* Superboard : Multi-Poker (XMP00006) */
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "xmp00006.u67",   0x00000, 0x10000, CRC(d61f1677) SHA1(2eca1315d6aa310a54de2dfa369e443a07495b76) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "xm00002p.u66",   0x00000, 0x10000, CRC(96cf471c) SHA1(9597bf6a80c392ee22dc4606db610fdaf032377f) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mro-cg2174.u77",	 0x00000, 0x8000, CRC(bb666733) SHA1(dcaa1980b051a554cb0f443b1183a680edc9ad3f) )
	ROM_LOAD( "mgo-cg2174.u78",	 0x08000, 0x8000, CRC(cc46adb0) SHA1(6065aa5dcb9091ad80e499c7ee6dc629e79c865a) )
	ROM_LOAD( "mbo-cg2174.u79",	 0x10000, 0x8000, CRC(7291a0c8) SHA1(1068f35e6ef5fd88c584922860231840a90fb623) )
	ROM_LOAD( "mxo-cg2174.u80",	 0x18000, 0x8000, CRC(14f9480c) SHA1(59323f9fc5995277aea86d088893b6eb95b4e89b) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap2174.u43", 0x0000, 0x0100, CRC(cbff3f26) SHA1(9e145676f2871c2369042a13cbeabb7efe2728e1) )
ROM_END

/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT         INIT      ROT    COMPANY                                  FULLNAME                                                  FLAGS   LAYOUT */

/* Set chips */
GAMEL(1987, peset038, 0,      peplus,  peplus_schip, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (Set038) Set Chip",                      0,   layout_pe_schip )

/* Normal board : poker */
GAMEL(1987, pepp0065, 0,      peplus,  peplus_poker, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0065) Jokers Wild Poker",             0,   layout_pe_poker )
GAMEL(1987, pepp0158, 0,      peplus,  peplus_pokah, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0158) 4 of a Kind Bonus Poker",       0,   layout_pe_poker )
GAMEL(1987, pepp0188, 0,      peplus,  peplus_pokah, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0188) Standard Draw Poker",           0,   layout_pe_poker )
GAMEL(1987, pepp0250, 0,      peplus,  peplus_poker, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0250) Double Down Stud Poker",        0,   layout_pe_poker )
GAMEL(1987, pepp0447, 0,      peplus,  peplus_poker, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0447) Standard Draw Poker",           0,   layout_pe_poker )
GAMEL(1987, pepp0516, 0,      peplus,  peplus_pokah, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PP0516) Double Bonus Poker",            0,   layout_pe_poker )

/* Normal board : blackjack */
GAMEL(1994, pebe0014, 0,      peplus,  peplus_bjack, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (BE0014) Blackjack",                     0,   layout_pe_bjack )

/* Normal board : keno */
GAMEL(1994, peke1012, 0,      peplus,  peplus_keno,  peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (KE1012) Keno",                          0,   layout_pe_keno )

/* Normal board : slots machine */
GAMEL(1996, peps0014, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0014) Super Joker Slots",             0,   layout_pe_slots )
GAMEL(1996, peps0022, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0022) Red White & Blue Slots",        0,   layout_pe_slots )
GAMEL(1996, peps0043, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0043) Double Diamond Slots",          0,   layout_pe_slots )
GAMEL(1996, peps0045, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0045) Red White & Blue Slots",        0,   layout_pe_slots )
GAMEL(1996, peps0308, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0308) Double Jackpot Slots",          0,   layout_pe_slots )
GAMEL(1996, peps0615, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0615) Chaos Slots",                   0,   layout_pe_slots )
GAMEL(1996, peps0716, 0,      peplus,  peplus_slots, peplus,   ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (PS0716) River Gambler Slots",           0,   layout_pe_slots )

/* Superboard : poker */
GAMEL(1995, pex2069p, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (X002069P) Double Double Bonus Poker",   0,   layout_pe_poker )
GAMEL(1995, pexp0019, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XP000019) Deuces Wild Poker",           0,   layout_pe_poker )
GAMEL(1995, pexp0112, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XP000112) White Hot Aces Poker",        0,   layout_pe_poker )

/* Superboard : multi-poker */
GAMEL(1995, pexmp006, 0,      peplus,  peplus_poker, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XMP00006) Multi-Poker",                 0,   layout_pe_poker )

/* Superboard : slots machine */
GAMEL(1997, pexs0006, 0,      peplus,  peplus_slots, peplussb, ROT0,  "IGT - International Gaming Technology", "Player's Edge Plus (XS000006) Triple Triple Diamond Slots", 0,   layout_pe_slots )
