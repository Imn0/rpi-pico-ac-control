#pragma once
#include "mitsubishi_heavy_values.h"
#include "pico/stdlib.h"



struct MH_ac_state {
    int temperatre;
    enum MH_mode mode;
    enum MH_power power;
    enum MH_fan_speed fan_speed;
    enum MH_vertical_air vertical_air;
    enum MH_horizontal_air horizontal_air;
    enum MH_3d_auto auto_3d_mode;
    enum MH_night_setback night_setback;
    enum MH_silent_mode silent_mode;
    enum MH_clean_alergen clean_alergen;
};

union MH_ir_encoded {
    uint32_t data_32bit[5];
    uint8_t data_8bit[20];
};

union MH_ir_encoded MH_ir_encode_ac_state(struct MH_ac_state ac_state);