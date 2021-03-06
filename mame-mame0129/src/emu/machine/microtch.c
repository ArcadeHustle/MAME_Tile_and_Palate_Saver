/*
    Microtouch touch screen controller

    Written by Mariusz Wojcieszek

    Notes/ToDo:
    - calibration mode (command CX)
    - only tablet format is supported for returning touch screen state

*/

#include "driver.h"
#include "microtch.h"

static struct
{
	UINT8		rx_buffer[16];
	int			rx_buffer_ptr;
	emu_timer*	timer;
	UINT8		tx_buffer[16];
	UINT8		tx_buffer_num;
	UINT8		tx_buffer_ptr;
	int			reset_done;
	int			format_tablet;
	int			mode_inactive;
	int			mode_stream;
	int			last_touch_state;
	int			last_x;
	int			last_y;
	microtouch_tx_func	tx_callback;
	microtouch_touch_func	touch_callback;
} microtouch;


static int microtouch_check_command( const char* commandtocheck, int command_len, UINT8* command_data )
{
	if ( (command_len == (strlen(commandtocheck) + 2)) &&
		 (command_data[0] == 0x01) &&
		 (strncmp(commandtocheck, (const char*)command_data + 1, strlen(commandtocheck)) == 0) &&
		 (command_data[command_len-1] == 0x0d) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void microtouch_send_format_table_packet(UINT8 flag, int x, int y)
{
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = flag;
	// lower byte (7bits) of x coordinate
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = x & 0x7f;
	// higher byte (7bits) of x coordinate
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = (x >> 7) & 0x7f;
	// lower byte (7bits) of y coordinate
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = y & 0x7f;
	// higher byte (7bits) of y coordinate
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = (y >> 7) & 0x7f;
};

static TIMER_CALLBACK(microtouch_timer_callback)
{
	if ( microtouch.tx_buffer_ptr < microtouch.tx_buffer_num )
	{
		microtouch.tx_callback( machine, microtouch.tx_buffer[microtouch.tx_buffer_ptr++] );
		if ( microtouch.tx_buffer_ptr == microtouch.tx_buffer_num )
		{
			microtouch.tx_buffer_ptr = microtouch.tx_buffer_num = 0;
		}
		return;
	}

	if ( (microtouch.reset_done == 0) ||
		 (microtouch.format_tablet == 0) ||
		 (microtouch.mode_inactive == 1) ||
		 (microtouch.mode_stream == 0) )
	{
		return;
	}

	// send format tablet packet
	if ( input_port_read(machine, "TOUCH") & 0x01 )
	{
		int tx = input_port_read(machine, "TOUCH_X");
		int ty = input_port_read(machine, "TOUCH_Y");

		if ( microtouch.touch_callback == NULL ||
			 microtouch.touch_callback( machine, &tx, &ty ) != 0 )
		{
			ty = 0x4000 - ty;

			microtouch_send_format_table_packet(0xc0, tx, ty);
			microtouch.last_touch_state = 1;
			microtouch.last_x = tx;
			microtouch.last_y = ty;
		}
	}
	else
	{
		if ( microtouch.last_touch_state == 1 )
		{
			microtouch.last_touch_state = 0;
			microtouch_send_format_table_packet(0x80, microtouch.last_x, microtouch.last_y);
		}
	}
};

void microtouch_init(running_machine *machine, microtouch_tx_func tx_cb, microtouch_touch_func touch_cb)
{
	memset(&microtouch, 0, sizeof(microtouch));

	microtouch.last_touch_state = -1;
	microtouch.tx_callback = tx_cb;
	microtouch.touch_callback = touch_cb;

	microtouch.timer = timer_alloc(machine, microtouch_timer_callback, NULL);
	timer_adjust_periodic(microtouch.timer, ATTOTIME_IN_HZ(167*5), 0, ATTOTIME_IN_HZ(167*5));

	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.reset_done);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.format_tablet);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.mode_inactive);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.mode_stream);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.last_touch_state);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.last_x);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.last_y);
	state_save_register_item_array(machine, "microtouch", NULL, 0, microtouch.rx_buffer);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.rx_buffer_ptr);
	state_save_register_item_array(machine, "microtouch", NULL, 0, microtouch.tx_buffer);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.tx_buffer_num);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.tx_buffer_ptr);

};


void microtouch_rx(int count, UINT8* data)
{
	int i;

	for ( i = 0; (i < count) && ((microtouch.rx_buffer_ptr + i) < 16); i++ )
	{
		microtouch.rx_buffer[i+microtouch.rx_buffer_ptr] = data[i];
		microtouch.rx_buffer_ptr++;
	}

	if (microtouch.rx_buffer_ptr > 0 && (microtouch.rx_buffer[microtouch.rx_buffer_ptr-1] == 0x0d))
	{
		// check command
		if ( microtouch_check_command( "MS", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.mode_stream = 1;
			microtouch.mode_inactive = 0;
		}
		else if ( microtouch_check_command( "MI", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.mode_inactive = 1;
		}
		else if ( microtouch_check_command( "R", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.tx_buffer_num = 0;
			microtouch.reset_done = 1;
		}
		else if ( microtouch_check_command( "FT", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.format_tablet = 1;
		}
		// send response
		microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x01;
		microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x30;
		microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x0d;
		microtouch.rx_buffer_ptr = 0;
	}
};

INPUT_PORTS_START(microtouch)
	PORT_START("TOUCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_IMPULSE(10) PORT_NAME( "Touch screen" )
	PORT_START("TOUCH_X")
	PORT_BIT( 0x3fff, 0x2000, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
	PORT_START("TOUCH_Y")
	PORT_BIT( 0x3fff, 0x2000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

