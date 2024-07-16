#pragma once
#include "mitsubishi_heavy_values.h"
#include "pico/stdlib.h"

struct MH_ac_state {
    atomic_int temperature;
    atomic_MH_mode mode;
    atomic_MH_power power;
    atomic_MH_fan_speed fan_speed;
    atomic_MH_vertical_air vertical_air;
    atomic_MH_horizontal_air horizontal_air;
    atomic_MH_3d_auto auto_3d_mode;
    atomic_MH_night_setback night_setback;
    atomic_MH_silent_mode silent_mode;
    atomic_MH_clean_alergen clean_alergen;
};

union MH_ir_encoded {
    uint32_t data_32bit[5];
    uint8_t data_8bit[20];
};

union MH_ir_encoded MH_ir_encode_ac_state(volatile struct MH_ac_state* ac_state);

/**
 * @brief Initializes pin for sending ir commands, initializes given ac_state to default ignores if passed in NULL
 * 
 * @param pin 
 * @param ac_state
 * @return  0 if success 1 otherwise  
 */
int MH_init(int pin, struct MH_ac_state* ac_state);

/**
 * @brief Sends IR command with given A/C state
 * 
 * @param ac_state 
 * @return struct MH_ac_state 
 */
void MH_send(volatile struct MH_ac_state* ac_state);

void MH_temperature_str(struct MH_ac_state* ac_state, char* buff);
void MH_temperature_change_delta(volatile struct MH_ac_state* ac_state, int delta);
void MH_temperature_change_target(volatile struct MH_ac_state* ac_state, int target);

void MH_mode_str(struct MH_ac_state* ac_state, char* buff);
void MH_mode_set(volatile struct MH_ac_state* ac_state, enum MH_mode mode);
void MH_power_toggle(volatile struct MH_ac_state* ac_state);
void MH_power_set(volatile struct MH_ac_state* ac_state, enum MH_power power);

void MH_fan_str(struct MH_ac_state* ac_state, char* buff);
void MH_fan_cycle(volatile struct MH_ac_state* ac_state, int direction);
void MH_fan_set(volatile struct MH_ac_state* ac_state, enum MH_fan_speed fan);

void MH_vertical_air_str(struct MH_ac_state* ac_state, char* buff);
void MH_vertical_air_cycle(volatile struct MH_ac_state* ac_state, int direction);
void MH_vertical_air_set(volatile struct MH_ac_state* ac_state, enum MH_vertical_air vertical_air);

void MH_horizontal_air_str(struct MH_ac_state* ac_state, char* buff);
void MH_horizontal_air_cycle(volatile struct MH_ac_state* ac_state, int direction);
void MH_horizontal_air_set(volatile struct MH_ac_state* ac_state, enum MH_horizontal_air horizontal_air);

void MH_3d_auto_str(struct MH_ac_state* ac_state, char* buff);
void MH_3d_auto_toggle(volatile struct MH_ac_state* ac_state);
void MH_3d_auto_set(volatile struct MH_ac_state* ac_state, enum MH_3d_auto _3d_auto);

void MH_night_setback_str(struct MH_ac_state* ac_state, char* buff);
void MH_night_setback_toggle(volatile struct MH_ac_state* ac_state);
void MH_night_setback_set(volatile struct MH_ac_state* ac_state, enum MH_night_setback night_setback);

void MH_silent_mode_str(struct MH_ac_state* ac_state, char* buff);
void MH_silent_mode_toggle(volatile struct MH_ac_state* ac_state);
void MH_silent_mode_set(volatile struct MH_ac_state* ac_state, enum MH_silent_mode silent_mode);

void MH_clean_alergen_str(struct MH_ac_state* ac_state, char* buff);
void MH_clean_alergen_toggle(volatile struct MH_ac_state* ac_state);
void MH_clean_alergen_set(volatile struct MH_ac_state* ac_state, enum MH_clean_alergen clean_alergen);