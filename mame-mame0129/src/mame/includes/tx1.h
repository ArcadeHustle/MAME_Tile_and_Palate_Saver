/*************************************************************************

    TX-1/Buggy Boy hardware

*************************************************************************/
#include "machine/8255ppi.h"
#include "sound/custom.h"

#define TX1_PIXEL_CLOCK		(XTAL_18MHz / 3)
#define TX1_HBSTART			256
#define TX1_HBEND			0
#define TX1_HTOTAL			384
#define TX1_VBSTART			240
#define TX1_VBEND			0
#define TX1_VTOTAL			264

#define BB_PIXEL_CLOCK		(XTAL_18MHz / 3)
#define BB_HBSTART			256
#define BB_HBEND			0
#define BB_HTOTAL			384
#define BB_VBSTART			240
#define BB_VBEND			0
#define BB_VTOTAL			288

#define CPU_MASTER_CLOCK	(XTAL_15MHz)
#define BUGGYBOY_ZCLK		(CPU_MASTER_CLOCK / 2)


/*----------- defined in drivers/tx1.c -----------*/
extern UINT16 *tx1_math_ram;

/*----------- defined in machine/tx1.c -----------*/
READ16_HANDLER( tx1_spcs_rom_r );
READ16_HANDLER( tx1_spcs_ram_r );
WRITE16_HANDLER( tx1_spcs_ram_w );
READ16_HANDLER( tx1_math_r );
WRITE16_HANDLER( tx1_math_w );
MACHINE_START( tx1 );
MACHINE_RESET( tx1 );


READ16_HANDLER( buggyboy_spcs_rom_r );
READ16_HANDLER( buggyboy_spcs_ram_r );
WRITE16_HANDLER( buggyboy_spcs_ram_w );
READ16_HANDLER( buggyboy_math_r );
WRITE16_HANDLER( buggyboy_math_w );
MACHINE_RESET( buggybjr );
MACHINE_RESET( buggyboy );
MACHINE_START( buggybjr );
MACHINE_START( buggyboy );

/*----------- defined in audio/tx1.c -----------*/
READ8_HANDLER( tx1_pit8253_r );
WRITE8_HANDLER( tx1_pit8253_w );

WRITE8_HANDLER( bb_ym1_a_w );
WRITE8_HANDLER( bb_ym2_a_w );
WRITE8_HANDLER( bb_ym2_b_w );
CUSTOM_START( buggyboy_sh_start );
CUSTOM_RESET( buggyboy_sh_reset );

WRITE8_HANDLER( tx1_ay8910_a_w );
WRITE8_HANDLER( tx1_ay8910_b_w );
CUSTOM_START( tx1_sh_start );
CUSTOM_RESET( tx1_sh_reset );


/*----------- defined in video/tx1.c -----------*/
READ16_HANDLER( tx1_crtc_r );
WRITE16_HANDLER( tx1_crtc_w );

extern UINT16 *tx1_vram;
extern UINT16 *tx1_objram;
extern UINT16 *tx1_rcram;
extern size_t tx1_objram_size;
PALETTE_INIT( tx1 );
VIDEO_START( tx1 );
VIDEO_UPDATE( tx1 );
VIDEO_EOF( tx1 );
WRITE16_HANDLER( tx1_slincs_w );
WRITE16_HANDLER( tx1_slock_w );
WRITE16_HANDLER( tx1_scolst_w );
WRITE16_HANDLER( tx1_bankcs_w );
WRITE16_HANDLER( tx1_flgcs_w );

extern UINT16 *buggyboy_objram;
extern UINT16 *buggyboy_rcram;
extern UINT16 *buggyboy_vram;
extern size_t buggyboy_objram_size;
extern size_t buggyboy_rcram_size;
PALETTE_INIT( buggyboy );
VIDEO_START( buggyboy );
VIDEO_UPDATE( buggyboy );
VIDEO_EOF( buggyboy );

extern UINT16 *buggybjr_vram;
VIDEO_START( buggybjr );
VIDEO_UPDATE( buggybjr );
WRITE16_HANDLER( buggyboy_slincs_w );
WRITE16_HANDLER( buggyboy_scolst_w );
WRITE16_HANDLER( buggyboy_gas_w );
WRITE16_HANDLER( buggyboy_sky_w );
