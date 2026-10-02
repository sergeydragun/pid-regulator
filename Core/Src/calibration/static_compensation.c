//
// Created by sergey on 10/1/26.
//

#include "static_compensation.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "global_vars.h"

static float *s_table =  NULL;

int32_t get_compensation_size()
{
    return (g_max_pos - g_min_pos) / STATIC_COMPENSATION_STEP + 1;
}

void init_compensation_table()
{
    int32_t size = get_compensation_size();
    s_table = (float *)malloc(size * sizeof(float));
}

int32_t get_index(int32_t position)
{
    return (position - g_min_pos) / STATIC_COMPENSATION_STEP;
}


void StaticCompensation_Set(int32_t position, float u_stat)
{
    s_table[get_index(position)] = u_stat;
}

float StaticCompensation_Get(int32_t position)
{
    const float normalized = (float)(position - g_min_pos) / STATIC_COMPENSATION_STEP;
    const int32_t lower = (int32_t)(normalized);
    const int32_t upper = lower + 1;
    const float fraction = normalized - (float)lower;

    return s_table[lower] + (s_table[upper] - s_table[lower]) * fraction;
}

void StaticCompensation_Reset()
{
    memset(s_table, 0, get_compensation_size() * sizeof(float));
}
