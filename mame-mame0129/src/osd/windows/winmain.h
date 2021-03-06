//============================================================
//
//  winmain.h - Win32 main program and core headers
//
//  Copyright Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================


//============================================================
//  CONSTANTS
//============================================================

// debugging options
#define WINOPTION_OSLOG					"oslog"
#define WINOPTION_WATCHDOG				"watchdog"
#define WINOPTION_DEBUGGER_FONT			"debugger_font"
#define WINOPTION_DEBUGGER_FONT_SIZE	"debugger_font_size"

// performance options
#define WINOPTION_PRIORITY				"priority"
#define WINOPTION_MULTITHREADING		"multithreading"

// video options
#define WINOPTION_VIDEO					"video"
#define WINOPTION_NUMSCREENS			"numscreens"
#define WINOPTION_WINDOW				"window"
#define WINOPTION_MAXIMIZE				"maximize"
#define WINOPTION_KEEPASPECT			"keepaspect"
#define WINOPTION_WINDOWZOOM			"windowzoom"
#define WINOPTION_PRESCALE				"prescale"
#define WINOPTION_EFFECT				"effect"
#define WINOPTION_WAITVSYNC				"waitvsync"
#define WINOPTION_SYNCREFRESH			"syncrefresh"

// DirectDraw-specific options
#define WINOPTION_HWSTRETCH				"hwstretch"

// Direct3D-specific options
#define WINOPTION_D3DVERSION			"d3dversion"
#define WINOPTION_FILTER				"filter"

// per-window options
#define WINOPTION_SCREEN				"screen"
#define WINOPTION_ASPECT				"aspect"
#define WINOPTION_RESOLUTION			"resolution"
#define WINOPTION_VIEW					"view"

#define WINOPTION_SCREEN0				"screen0"
#define WINOPTION_ASPECT0				"aspect0"
#define WINOPTION_RESOLUTION0			"resolution0"
#define WINOPTION_VIEW0					"view0"

#define WINOPTION_SCREEN1				"screen1"
#define WINOPTION_ASPECT1				"aspect1"
#define WINOPTION_RESOLUTION1			"resolution1"
#define WINOPTION_VIEW1					"view1"

#define WINOPTION_SCREEN2				"screen2"
#define WINOPTION_ASPECT2				"aspect2"
#define WINOPTION_RESOLUTION2			"resolution2"
#define WINOPTION_VIEW2					"view2"

#define WINOPTION_SCREEN3				"screen3"
#define WINOPTION_ASPECT3				"aspect3"
#define WINOPTION_RESOLUTION3			"resolution3"
#define WINOPTION_VIEW3					"view3"

// full screen options
#define WINOPTION_TRIPLEBUFFER			"triplebuffer"
#define WINOPTION_SWITCHRES				"switchres"
#define WINOPTION_FULLSCREENBRIGHTNESS	"full_screen_brightness"
#define WINOPTION_FULLLSCREENCONTRAST	"full_screen_contrast"
#define WINOPTION_FULLSCREENGAMMA		"full_screen_gamma"

// sound options
#define WINOPTION_AUDIO_LATENCY			"audio_latency"

// input options
#define WINOPTION_DUAL_LIGHTGUN			"dual_lightgun"



//============================================================
//  GLOBAL VARIABLES
//============================================================

extern const options_entry mame_win_options[];



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

// use if you want to print something with the verbose flag
void CLIB_DECL mame_printf_verbose(const char *text, ...) ATTR_PRINTF(1,2);
