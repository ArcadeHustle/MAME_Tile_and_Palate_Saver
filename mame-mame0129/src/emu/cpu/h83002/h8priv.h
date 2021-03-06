/***************************************************************************

 h8priv.h : Private constants and other definitions for the H8/3002 emulator.

****************************************************************************/

#pragma once

#ifndef __H8PRIV_H__
#define __H8PRIV_H__

typedef struct _h83xx_state h83xx_state;
struct _h83xx_state
{
	// main CPU stuff
	UINT32 h8err;
	UINT32 regs[8];
	UINT32 pc, ppc;

	UINT32 h8_IRQrequestH, h8_IRQrequestL;
	INT32 cyccnt;

	UINT8  ccr;
	UINT8  h8nflag, h8vflag, h8cflag, h8zflag, h8iflag, h8hflag;
	UINT8  h8uflag, h8uiflag;

	cpu_irq_callback irq_cb;
	const device_config *device;

	const address_space *program;
	const address_space *io;

	// H8/3002 onboard peripherals stuff

	UINT8 per_regs[256];

	UINT16 h8TCNT[5];
	UINT8 h8TSTR;

	emu_timer *timer[5];

	int mode_8bit;
};
extern h83xx_state h8;


UINT8 h8_register_read8(h83xx_state *h8, UINT32 address);
UINT8 h8_3007_register_read8(h83xx_state *h8, UINT32 address);
UINT8 h8_3007_register1_read8(h83xx_state *h8, UINT32 address);
void h8_register_write8(h83xx_state *h8, UINT32 address, UINT8 val);
void h8_3007_register_write8(h83xx_state *h8, UINT32 address, UINT8 val);
void h8_3007_register1_write8(h83xx_state *h8, UINT32 address, UINT8 val);

void h8_itu_init(h83xx_state *h8);
void h8_3007_itu_init(h83xx_state *h8);
void h8_itu_reset(h83xx_state *h8);
UINT8 h8_itu_read8(h83xx_state *h8, UINT8 reg);
UINT8 h8_3007_itu_read8(h83xx_state *h8, UINT8 reg);
void h8_itu_write8(h83xx_state *h8, UINT8 reg, UINT8 val);
void h8_3007_itu_write8(h83xx_state *h8, UINT8 reg, UINT8 val);

#endif /* __H8PRIV_H__ */
