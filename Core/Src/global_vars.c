#include <stdint.h>
#include "global_vars.h"

#include <stddef.h>

volatile float g_target = 0;
volatile ADRC_t g_adrc;

volatile int32_t g_max_pos;
volatile int32_t g_min_pos;
