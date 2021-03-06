//============================================================
//
//  video.h - Win32 implementation of MAME video routines
//
//  Copyright Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WIN_VIDEO__
#define __WIN_VIDEO__


//============================================================
//  CONSTANTS
//============================================================

#define MAX_WINDOWS			4

#define VIDEO_MODE_NONE		0
#define VIDEO_MODE_GDI		1
#define VIDEO_MODE_DDRAW	2
#define VIDEO_MODE_D3D		3



//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef struct _win_monitor_info win_monitor_info;
struct _win_monitor_info
{
	win_monitor_info  *	next;					// pointer to next monitor in list
	HMONITOR			handle;					// handle to the monitor
	MONITORINFOEX		info;					// most recently retrieved info
	float				aspect;					// computed/configured aspect ratio of the physical device
	int					reqwidth;				// requested width for this monitor
	int					reqheight;				// requested height for this monitor
};


typedef struct _win_window_config win_window_config;
struct _win_window_config
{
	float				aspect;						// decoded aspect ratio
	int					width;						// decoded width
	int					height;						// decoded height
	int					refresh;					// decoded refresh
};


typedef struct _win_video_config win_video_config;
struct _win_video_config
{
	// global configuration
	int					windowed;					// start windowed?
	int					prescale;					// prescale factor
	int					keepaspect;					// keep aspect ratio
	int                 windowzoom;					// window mode zoom
	int					numscreens;					// number of screens
	int					layerconfig;				// default configuration of layers

	// per-window configuration
	win_window_config	window[MAX_WINDOWS];		// configuration data per-window

	// hardware options
	int					mode;						// output mode
	int					waitvsync;					// spin until vsync
	int					syncrefresh;				// sync only to refresh rate
	int					triplebuf;					// triple buffer
	int					switchres;					// switch resolutions

	// ddraw options
	int					hwstretch;					// stretch using the hardware

	// d3d options
	int					filter;						// enable filtering
};



//============================================================
//  GLOBAL VARIABLES
//============================================================

extern win_monitor_info *win_monitor_list;
extern win_video_config video_config;


//============================================================
//  PROTOTYPES
//============================================================

void winvideo_init(running_machine *machine);

void winvideo_monitor_refresh(win_monitor_info *monitor);
float winvideo_monitor_get_aspect(win_monitor_info *monitor);
win_monitor_info *winvideo_monitor_from_handle(HMONITOR monitor);

#endif
