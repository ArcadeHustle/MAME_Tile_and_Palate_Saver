#include "sound/custom.h"

/* flags for init_pc_common */
#define PCCOMMON_KEYBOARD_PC	0
#define PCCOMMON_KEYBOARD_AT	1

void init_pc_common(running_machine *machine, UINT32 flags, void (*set_keyb_int_func)(running_machine *, int));

void pc_keyboard(void);
UINT8 pc_keyb_read(void);
void pc_keyb_set_clock(int on);
void pc_keyb_clear(void);

