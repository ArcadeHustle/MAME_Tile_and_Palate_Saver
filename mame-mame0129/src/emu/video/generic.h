/*********************************************************************

    generic.h

    Generic simple video functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __VIDEO_GENERIC_H__
#define __VIDEO_GENERIC_H__

#include "mamecore.h"



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern UINT8 *videoram;
extern UINT16 *videoram16;
extern UINT32 *videoram32;
extern size_t videoram_size;

extern UINT8 *colorram;
extern UINT16 *colorram16;
extern UINT32 *colorram32;

extern UINT8 *spriteram;
extern UINT16 *spriteram16;
extern UINT32 *spriteram32;

extern UINT8 *spriteram_2;
extern UINT16 *spriteram16_2;
extern UINT32 *spriteram32_2;

extern UINT8 *spriteram_3;
extern UINT16 *spriteram16_3;
extern UINT32 *spriteram32_3;

extern UINT8 *buffered_spriteram;
extern UINT16 *buffered_spriteram16;
extern UINT32 *buffered_spriteram32;

extern UINT8 *buffered_spriteram_2;
extern UINT16 *buffered_spriteram16_2;
extern UINT32 *buffered_spriteram32_2;

extern size_t spriteram_size;
extern size_t spriteram_2_size;
extern size_t spriteram_3_size;

extern UINT8 *paletteram;
extern UINT16 *paletteram16;
extern UINT32 *paletteram32;

extern UINT8 *paletteram_2;	/* use when palette RAM is split in two parts */
extern UINT16 *paletteram16_2;

extern bitmap_t *tmpbitmap;



/***************************************************************************
    COMMON GRAPHICS LAYOUTS
***************************************************************************/

extern const gfx_layout gfx_8x8x1;
extern const gfx_layout gfx_8x8x2_planar;
extern const gfx_layout gfx_8x8x3_planar;
extern const gfx_layout gfx_8x8x4_planar;
extern const gfx_layout gfx_8x8x5_planar;
extern const gfx_layout gfx_8x8x6_planar;

extern const gfx_layout gfx_16x16x4_planar;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- initialization ----- */

/* set up all the common systems */
void generic_video_init(running_machine *machine);

/* generic video start with a temporary bitmap */
VIDEO_START( generic_bitmapped );

/* generic video update to blit a temporary bitmap */
VIDEO_UPDATE( generic_bitmapped );



/* ----- sprite buffering ----- */

/* buffered sprite RAM write handlers */
WRITE8_HANDLER( buffer_spriteram_w );
WRITE16_HANDLER( buffer_spriteram16_w );
WRITE32_HANDLER( buffer_spriteram32_w );
WRITE8_HANDLER( buffer_spriteram_2_w );
WRITE16_HANDLER( buffer_spriteram16_2_w );
WRITE32_HANDLER( buffer_spriteram32_2_w );

/* perform the actual buffering */
void buffer_spriteram(UINT8 *ptr, int length);
void buffer_spriteram_2(UINT8 *ptr, int length);



/* ----- global attributes ----- */

/* set global attributes */
void flip_screen_set(running_machine *machine, int on);
void flip_screen_set_no_update(running_machine *machine, int on); 	/* will not call update_flip */
void flip_screen_x_set(running_machine *machine, int on);
void flip_screen_y_set(running_machine *machine, int on);
int flip_screen_get(running_machine *machine);
int flip_screen_x_get(running_machine *machine);
int flip_screen_y_get(running_machine *machine);

//#define flip_screen flip_screen_get(machine)



/* ----- generic palette init routines ----- */

PALETTE_INIT( black_and_white );
PALETTE_INIT( RRRR_GGGG_BBBB );
PALETTE_INIT( RRRRR_GGGGG_BBBBB );
PALETTE_INIT( BBBBB_GGGGG_RRRRR );
PALETTE_INIT( RRRRR_GGGGGG_BBBBB );



/* ----- generic palette RAM write handlers, use MRAx_RAM to read ----- */

/* 3-3-2 RGB palette write handlers */
WRITE8_HANDLER( paletteram_BBGGGRRR_w );
WRITE8_HANDLER( paletteram_RRRGGGBB_w );
WRITE8_HANDLER( paletteram_BBGGRRII_w );

/* 4-4-4 RGB palette write handlers */
WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_le_w );
WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_be_w );
WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_split1_w );	/* uses paletteram[] */
WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_split2_w );	/* uses paletteram_2[] */
WRITE16_HANDLER( paletteram16_xxxxBBBBGGGGRRRR_word_w );

WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_le_w );
WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_be_w );
WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_split1_w );	/* uses paletteram[] */
WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_split2_w );	/* uses paletteram_2[] */
WRITE16_HANDLER( paletteram16_xxxxBBBBRRRRGGGG_word_w );

WRITE8_HANDLER( paletteram_xxxxRRRRBBBBGGGG_split1_w );	/* uses paletteram[] */
WRITE8_HANDLER( paletteram_xxxxRRRRBBBBGGGG_split2_w );	/* uses paletteram_2[] */

WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_le_w );
WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_be_w );
WRITE16_HANDLER( paletteram16_xxxxRRRRGGGGBBBB_word_w );

WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_be_w );
WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_split1_w );	/* uses paletteram[] */
WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_split2_w );	/* uses paletteram_2[] */
WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBxxxx_word_w );

/* 4-4-4-4 IRGB palette write handlers */
WRITE16_HANDLER( paletteram16_IIIIRRRRGGGGBBBB_word_w );
WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBIIII_word_w );

/* 5-5-5 RGB palette write handlers */
WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_le_w );
WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_be_w );
WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_split1_w );	/* uses paletteram[] */
WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_split2_w );	/* uses paletteram_2[] */
WRITE16_HANDLER( paletteram16_xBBBBBGGGGGRRRRR_word_w );

WRITE8_HANDLER( paletteram_xBBBBBRRRRRGGGGG_split1_w );  /* uses paletteram[] */
WRITE8_HANDLER( paletteram_xBBBBBRRRRRGGGGG_split2_w );  /* uses paletteram_2[] */

WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_le_w );
WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_split1_w );
WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_split2_w );
WRITE16_HANDLER( paletteram16_xRRRRRGGGGGBBBBB_word_w );

WRITE16_HANDLER( paletteram16_xGGGGGRRRRRBBBBB_word_w );
WRITE16_HANDLER( paletteram16_xGGGGGBBBBBRRRRR_word_w );

WRITE16_HANDLER( paletteram16_RRRRRGGGGGBBBBBx_word_w );
WRITE16_HANDLER( paletteram16_GGGGGRRRRRBBBBBx_word_w );
WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBRGBx_word_w );

/* 8-8-8 RGB palette write handlers */
WRITE16_HANDLER( paletteram16_xrgb_word_be_w );
WRITE16_HANDLER( paletteram16_xbgr_word_be_w );


#endif	/* __VIDEO_GENERIC_H__ */
