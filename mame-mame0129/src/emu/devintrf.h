/***************************************************************************

    devintrf.h

    Device interface functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DEVINTRF_H__
#define __DEVINTRF_H__

#include "mamecore.h"
#include "romload.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* device classes */
enum _device_class
{
	DEVICE_CLASS_GENERAL = 0,			/* default class for general devices */
	DEVICE_CLASS_PERIPHERAL,			/* peripheral devices: PIAs, timers, etc. */
	DEVICE_CLASS_AUDIO,					/* audio devices (not sound chips), including speakers */
	DEVICE_CLASS_VIDEO,					/* video devices, including screens */
	DEVICE_CLASS_CPU_CHIP,				/* CPU chips; only CPU cores should return this class */
	DEVICE_CLASS_SOUND_CHIP,			/* sound chips; only sound cores should return this class */
	DEVICE_CLASS_TIMER,					/* timer devices */
	DEVICE_CLASS_OTHER					/* anything else (the list may expand in the future) */
};
typedef enum _device_class device_class;


/* device start results */
enum _device_start_err
{
	DEVICE_START_OK = 0,				/* everything is fine */
	DEVICE_START_MISSING_DEPENDENCY		/* at least one device we depend on is not yet started */
};
typedef enum _device_start_err device_start_err;


/* useful in device_list functions for scanning through all devices */
#define DEVICE_TYPE_WILDCARD			NULL


/* state constants passed to the device_get_info_func */
enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	DEVINFO_INT_FIRST = 0x00000,

		DEVINFO_INT_TOKEN_BYTES = DEVINFO_INT_FIRST,	/* R/O: bytes to allocate for the token */
		DEVINFO_INT_INLINE_CONFIG_BYTES,				/* R/O: bytes to allocate for the inline configuration */
		DEVINFO_INT_CLASS,								/* R/O: the device's class */

	DEVINFO_INT_CLASS_SPECIFIC = 0x04000,				/* R/W: device-specific values start here */
	DEVINFO_INT_DEVICE_SPECIFIC = 0x08000,				/* R/W: device-specific values start here */
	DEVINFO_INT_LAST = 0x0ffff,

	/* --- the following bits of info are returned as pointers --- */
	DEVINFO_PTR_FIRST = 0x10000,

		DEVINFO_PTR_ROM_REGION = DEVINFO_PTR_FIRST,		/* R/O: pointer to device-specific ROM region */
		DEVINFO_PTR_MACHINE_CONFIG,						/* R/O: pointer to device-specific machine config */

	DEVINFO_PTR_CLASS_SPECIFIC = 0x14000,				/* R/W: device-specific values start here */
	DEVINFO_PTR_DEVICE_SPECIFIC = 0x18000,				/* R/W: device-specific values start here */
	DEVINFO_PTR_LAST = 0x1ffff,

	/* --- the following bits of info are returned as pointers to functions --- */
	DEVINFO_FCT_FIRST = 0x20000,

		DEVINFO_FCT_SET_INFO = DEVINFO_FCT_FIRST,		/* R/O: device_set_info_func */
		DEVINFO_FCT_START,								/* R/O: device_start_func */
		DEVINFO_FCT_STOP,								/* R/O: device_stop_func */
		DEVINFO_FCT_RESET,								/* R/O: device_reset_func */
		DEVINFO_FCT_EXECUTE,							/* R/O: device_execute_func */
		DEVINFO_FCT_VALIDITY_CHECK,						/* R/O: device_validity_check_func */
		DEVINFO_FCT_NVRAM,								/* R/O: device_nvram_func */

	DEVINFO_FCT_CLASS_SPECIFIC = 0x24000,				/* R/W: device-specific values start here */
	DEVINFO_FCT_DEVICE_SPECIFIC = 0x28000,				/* R/W: device-specific values start here */
	DEVINFO_FCT_LAST = 0x2ffff,

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	DEVINFO_STR_FIRST = 0x30000,

		DEVINFO_STR_NAME = DEVINFO_STR_FIRST,			/* R/O: name of the device */
		DEVINFO_STR_FAMILY,								/* R/O: family of the device */
		DEVINFO_STR_VERSION,							/* R/O: version of the device */
		DEVINFO_STR_SOURCE_FILE,						/* R/O: file containing the device implementation */
		DEVINFO_STR_CREDITS,							/* R/O: credits for the device implementation */

	DEVINFO_STR_CLASS_SPECIFIC = 0x34000,				/* R/W: device-specific values start here */
	DEVINFO_STR_DEVICE_SPECIFIC = 0x38000,				/* R/W: device-specific values start here */
	DEVINFO_STR_LAST = 0x3ffff
};



/***************************************************************************
    MACROS
***************************************************************************/

#define DEVICE_GET_INFO_NAME(name)	device_get_info_##name
#define DEVICE_GET_INFO(name)		void DEVICE_GET_INFO_NAME(name)(const device_config *device, UINT32 state, deviceinfo *info)
#define DEVICE_GET_INFO_CALL(name)	DEVICE_GET_INFO_NAME(name)(device, state, info)

#define DEVICE_SET_INFO_NAME(name)	device_set_info_##name
#define DEVICE_SET_INFO(name)		void DEVICE_SET_INFO_NAME(name)(const device_config *device, UINT32 state, const deviceinfo *info)
#define DEVICE_SET_INFO_CALL(name)	DEVICE_SET_INFO_NAME(name)(device, state, info)

#define DEVICE_START_NAME(name)		device_start_##name
#define DEVICE_START(name)			device_start_err DEVICE_START_NAME(name)(const device_config *device)
#define DEVICE_START_CALL(name)		DEVICE_START_NAME(name)(device)

#define DEVICE_STOP_NAME(name)		device_stop_##name
#define DEVICE_STOP(name)			void DEVICE_STOP_NAME(name)(const device_config *device)
#define DEVICE_STOP_CALL(name)		DEVICE_STOP_NAME(name)(device)

#define DEVICE_RESET_NAME(name)		device_reset_##name
#define DEVICE_RESET(name)			void DEVICE_RESET_NAME(name)(const device_config *device)
#define DEVICE_RESET_CALL(name)		DEVICE_RESET_NAME(name)(device)

#define DEVICE_EXECUTE_NAME(name)	device_execute_##name
#define DEVICE_EXECUTE(name)		INT32 DEVICE_EXECUTE_NAME(name)(const device_config *device, INT32 clocks)
#define DEVICE_EXECUTE_CALL(name)	DEVICE_EXECUTE_NAME(name)(device, clocks)

#define DEVICE_NVRAM_NAME(name)		device_NVRAM_##name
#define DEVICE_NVRAM(name)			void DEVICE_NVRAM_NAME(name)(const device_config *device, mame_file *file, int read_or_write)
#define DEVICE_NVRAM_CALL(name)		DEVICE_NVRAM_NAME(name)(device, file, read_or_write)

#define DEVICE_VALIDITY_CHECK_NAME(name)	device_validity_check_##name
#define DEVICE_VALIDITY_CHECK(name)			int DEVICE_VALIDITY_CHECK_NAME(name)(const game_driver *driver, const device_config *device)
#define DEVICE_VALIDITY_CHECK_CALL(name)	DEVICE_VALIDITY_CHECK(name)(driver, device)


/* shorthand for accessing devices by machine/type/tag */
#define devtag_get_device(mach,type,tag)					device_list_find_by_tag((mach)->config->devicelist, type, tag)

#define devtag_reset(mach,type,tag)							device_reset(devtag_get_device(mach, type, tag))

#define devtag_get_info_int(mach,type,tag,state)			device_get_info_int(devtag_get_device(mach, type, tag), state)
#define devtag_get_info_ptr(mach,type,tag,state)			device_get_info_ptr(devtag_get_device(mach, type, tag), state)
#define devtag_get_info_fct(mach,type,tag,state)			device_get_info_fct(devtag_get_device(mach, type, tag), state)
#define devtag_get_info_string(mach,type,tag,state)			device_get_info_string(devtag_get_device(mach, type, tag), state)

#define devtag_set_info_int(mach,type,tag,state,data)		device_set_info_int(devtag_get_device(mach, type, tag), state, data)
#define devtag_set_info_ptr(mach,type,tag,state,data)		device_set_info_ptr(devtag_get_device(mach, type, tag), state, data)
#define devtag_set_info_fct(mach,type,tag,state,data)		device_set_info_fct(devtag_get_device(mach, type, tag), state, data)


/* shorthand for getting standard data about device types */
#define devtype_get_name(type)								devtype_get_info_string(type, DEVINFO_STR_NAME)
#define devtype_get_family(type) 							devtype_get_info_string(type, DEVINFO_STR_FAMILY)
#define devtype_get_version(type) 							devtype_get_info_string(type, DEVINFO_STR_VERSION)
#define devtype_get_source_file(type) 						devtype_get_info_string(type, DEVINFO_STR_SOURCE_FILE)
#define devtype_get_credits(type) 							devtype_get_info_string(type, DEVINFO_STR_CREDITS)


/* shorthand for getting standard data about devices */
#define device_get_name(dev)								device_get_info_string(dev, DEVINFO_STR_NAME)
#define device_get_family(dev) 								device_get_info_string(dev, DEVINFO_STR_FAMILY)
#define device_get_version(dev) 							device_get_info_string(dev, DEVINFO_STR_VERSION)
#define device_get_source_file(dev)							device_get_info_string(dev, DEVINFO_STR_SOURCE_FILE)
#define device_get_credits(dev) 							device_get_info_string(dev, DEVINFO_STR_CREDITS)


/* shorthand for getting standard data about devices by machine/type/tag */
#define devtag_get_name(mach,type,tag)						devtag_get_info_string(mach, type, tag, DEVINFO_STR_NAME)
#define devtag_get_family(mach,type,tag) 					devtag_get_info_string(mach, type, tag, DEVINFO_STR_FAMILY)
#define devtag_get_version(mach,type,tag) 					devtag_get_info_string(mach, type, tag, DEVINFO_STR_VERSION)
#define devtag_get_source_file(mach,type,tag)				devtag_get_info_string(mach, type, tag, DEVINFO_STR_SOURCE_FILE)
#define devtag_get_credits(mach,type,tag) 					devtag_get_info_string(mach, type, tag, DEVINFO_STR_CREDITS)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward-declare these types */
typedef union _deviceinfo deviceinfo;
typedef struct _device_config device_config;


/* device interface function types */
typedef void (*device_get_info_func)(const device_config *device, UINT32 state, deviceinfo *info);
typedef void (*device_set_info_func)(const device_config *device, UINT32 state, const deviceinfo *info);
typedef device_start_err (*device_start_func)(const device_config *device);
typedef void (*device_stop_func)(const device_config *device);
typedef INT32 (*device_execute_func)(const device_config *device, INT32 clocks);
typedef void (*device_reset_func)(const device_config *device);
typedef void (*device_nvram_func)(const device_config *device, mame_file *file, int read_or_write);
typedef int (*device_validity_check_func)(const game_driver *driver, const device_config *device);


/* a device_type is simply a pointer to its get_info function */
typedef device_get_info_func device_type;


/* the actual deviceinfo union */
union _deviceinfo
{
	INT64					i;						/* generic integers */
	void *					p;						/* generic pointers */
	genf *  				f;						/* generic function pointers */
	char *					s;						/* generic strings */

	device_set_info_func 	set_info;				/* DEVINFO_FCT_SET_INFO */
	device_start_func		start;					/* DEVINFO_FCT_START */
	device_stop_func		stop;					/* DEVINFO_FCT_STOP */
	device_reset_func		reset;					/* DEVINFO_FCT_RESET */
	device_execute_func 	execute;				/* DEVINFO_FCT_EXECUTE */
	device_validity_check_func validity_check;		/* DEVINFO_FCT_VALIDITY_CHECK */
	device_nvram_func		nvram;					/* DEVINFO_FCT_NVRAM */
	const rom_entry *		romregion;				/* DEVINFO_PTR_ROM_REGION */
	const union _machine_config_token *machine_config;/* DEVINFO_PTR_MACHINE_CONFIG */
};


/* the configuration for a general device */
struct _device_config
{
	/* device relationships (always valid) */
	device_config *			next;					/* next device (of any type/class) */
	device_config *			owner;					/* device that owns us, or NULL if nobody */
	device_config *			typenext;				/* next device of the same type */
	device_config *			classnext;				/* next device of the same class */

	/* device properties (always valid) */
	device_type				type;					/* device type */
	device_class			class;					/* device class */
	device_set_info_func 	set_info;				/* quick pointer to set_info callback */

	/* device configuration (always valid) */
	UINT32					clock;					/* device clock */
	const void *			static_config;			/* static device configuration */
	void *					inline_config;			/* inline device configuration */

	/* these fields are only valid once the device is attached to a machine */
	running_machine *		machine;				/* machine if device is live */

	/* these fields are only valid if the device is live */
	UINT8					started;				/* TRUE if the start function has succeeded */
	void *					token;					/* token if device is live */
	UINT32					tokenbytes;				/* size of the token data allocated */
	UINT8 *					region;					/* pointer to region with the device's tag, or NULL */
	UINT32					regionbytes;			/* size of the region, in bytes */
	device_execute_func 	execute;				/* quick pointer to execute callback */

	/* tag (always valid; at end because it is variable-length) */
	char					tag[1];					/* tag for this instance */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- device configuration ----- */

/* add a new device to the end of a device list */
device_config *device_list_add(device_config **listheadptr, const device_config *owner, device_type type, const char *tag, UINT32 clock);

/* remove a device from a device list */
void device_list_remove(device_config **listheadptr, device_type type, const char *tag);

/* build a tag that combines the device's name and the given tag */
const char *device_build_tag(astring *dest, const device_config *device, const char *tag);

/* build a tag with the same device prefix as the source tag*/
const char *device_inherit_tag(astring *dest, const char *sourcetag, const char *tag);



/* ----- type-based device access ----- */

/* return the number of items of a given type; DEVICE_TYPE_WILDCARD is allowed */
int device_list_items(const device_config *listhead, device_type type);

/* return the first device in the list of a given type; DEVICE_TYPE_WILDCARD is allowed */
const device_config *device_list_first(const device_config *listhead, device_type type);

/* return the next device in the list of a given type; DEVICE_TYPE_WILDCARD is allowed */
const device_config *device_list_next(const device_config *prevdevice, device_type type);

/* retrieve a device configuration based on a type and tag; DEVICE_TYPE_WILDCARD is allowed */
const device_config *device_list_find_by_tag(const device_config *listhead, device_type type, const char *tag);

/* return the index of a device based on its type and tag; DEVICE_TYPE_WILDCARD is allowed */
int device_list_index(const device_config *listhead, device_type type, const char *tag);

/* retrieve a device configuration based on a type and index; DEVICE_TYPE_WILDCARD is allowed */
const device_config *device_list_find_by_index(const device_config *listhead, device_type type, int index);



/* ----- class-based device access ----- */

/* return the number of items of a given class */
int device_list_class_items(const device_config *listhead, device_class class);

/* return the first device in the list of a given class */
const device_config *device_list_class_first(const device_config *listhead, device_class class);

/* return the next device in the list of a given class */
const device_config *device_list_class_next(const device_config *prevdevice, device_class class);

/* retrieve a device configuration based on a class and tag */
const device_config *device_list_class_find_by_tag(const device_config *listhead, device_class class, const char *tag);

/* return the index of a device based on its class and tag */
int device_list_class_index(const device_config *listhead, device_class class, const char *tag);

/* retrieve a device configuration based on a class and index */
const device_config *device_list_class_find_by_index(const device_config *listhead, device_class class, int index);



/* ----- live device management ----- */

/* "attach" a running_machine to its list of devices */
void device_list_attach_machine(running_machine *machine);

/* start the configured list of devices for a machine */
void device_list_start(running_machine *machine);

/* reset a device based on an allocated device_config */
void device_reset(const device_config *device);

/* change the clock on a device */
void device_set_clock(const device_config *device, UINT32 clock);



/* ----- device information getters ----- */

/* return an integer state value from an allocated device */
INT64 device_get_info_int(const device_config *device, UINT32 state);

/* return a pointer state value from an allocated device */
void *device_get_info_ptr(const device_config *device, UINT32 state);

/* return a function pointer state value from an allocated device */
genf *device_get_info_fct(const device_config *device, UINT32 state);

/* return a string value from an allocated device */
const char *device_get_info_string(const device_config *device, UINT32 state);



/* ----- device type information getters ----- */

/* return an integer value from a device type (does not need to be allocated) */
INT64 devtype_get_info_int(device_type type, UINT32 state);

/* return a function pointer from a device type (does not need to be allocated) */
genf *devtype_get_info_fct(device_type type, UINT32 state);

/* return a string value from a device type (does not need to be allocated) */
const char *devtype_get_info_string(device_type type, UINT32 state);



/* ----- device information setters ----- */

/* set an integer state value for an allocated device */
void device_set_info_int(const device_config *device, UINT32 state, INT64 data);

/* set a pointer state value for an allocated device */
void device_set_info_ptr(const device_config *device, UINT32 state, void *data);

/* set a function pointer state value for an allocated device */
void device_set_info_fct(const device_config *device, UINT32 state, genf *data);


#endif	/* __DEVINTRF_H__ */
