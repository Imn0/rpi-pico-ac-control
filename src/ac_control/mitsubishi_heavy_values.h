#pragma once
// configuration for RLA502A700B / RLA502A700L / RLA502A704
// bytes and decoding from https://github.com/ToniA/Raw-IR-decoder-for-Arduino
#include <stdatomic.h>

#define MH_MAX_TEMP 34
#define MH_MIN_TEMP 17


/**
 * @brief bytes[5] & 0x07
 */
enum MH_mode {
    MH_mode_auto = 0x07,
    MH_mode_cooling = 0x06,
    MH_mode_drying = 0x05,
    MH_mode_fan = 0x04,
    MH_mode_heating = 0x03,
};
typedef atomic_uint atomic_MH_mode;


/**
 * @brief bytes[5] & 0x08
 */
enum MH_power {
    MH_power_on = 0x00,
    MH_power_off = 0x08,
};
typedef atomic_uint atomic_MH_power;


/**
 * @brief bytes[9] & 0x0f
 */
enum MH_fan_speed {
    MH_fan_speed_auto = 0x0f,
    MH_fan_speed_1 = 0x0e,
    MH_fan_speed_2 = 0x0d,
    MH_fan_speed_3 = 0x0c,
    MH_fan_speed_4 = 0x0b,
    MH_fan_speed_eco = 0x09,
    MH_fan_speed_high = 0x07,
};
typedef atomic_uint atomic_MH_fan_speed;


/**
 * @brief bytes[11] & 0xe0
 */
enum MH_vertical_air {
    MH_vertical_air_swing = 0xe0,
    MH_vertical_air_up = 0xc0,
    MH_vertical_air_middle_up = 0xa0,
    MH_vertical_air_middle = 0x80,
    MH_vertical_air_middle_down = 0x60,
    MH_vertical_air_down = 0x40,
    MH_vertical_air_stop = 0x20,
};
typedef atomic_uint atomic_MH_vertical_air;


/**
 * @brief bytes[13] & 0x0f
 */
enum MH_horizontal_air {
    MH_horizontal_air_left = 0x0e,
    MH_horizontal_air_middle_left = 0x0d,
    MH_horizontal_air_middle = 0x0c,
    MH_horizontal_air_middle_right = 0x0b,
    MH_horizontal_air_right = 0x0a,
    MH_horizontal_air_left_right = 0x08,
    MH_horizontal_air_right_left = 0x09,
    MH_horizontal_air_swing = 0x0f,
    MH_horizontal_air_stop = 0x07,
};
typedef atomic_uint atomic_MH_horizontal_air;

/**
 * @brief bytes[11] & 0x12
 */
enum MH_3d_auto {
    MH_3d_auto_on = 0x00,
    MH_3d_auto_off = 0x12
};
typedef atomic_uint atomic_MH_3d_auto;

/**
 * @brief bytes[15] & 0x40
 */
enum MH_night_setback {
    MH_night_setback_on = 0x00,
    MH_night_setback_off = 0x40
};
typedef atomic_uint atomic_MH_night_setback;

/**
 * @brief bytes[15] & 0x80
 */
enum MH_silent_mode {
    MH_silent_mode_on = 0x00,
    MH_silent_mode_off = 0x80,
};
typedef atomic_uint atomic_MH_silent_mode;

/**
 * @brief bytes[5] & 0x60
 */
enum MH_clean_alergen {
    MH_clean_alergen_clean = 0x00,
    MH_clean_alergen_alergen = 0x20,
    MH_clean_alergen_off = 0x60
};
typedef atomic_uint atomic_MH_clean_alergen;
