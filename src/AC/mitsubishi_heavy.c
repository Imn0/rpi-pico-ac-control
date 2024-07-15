#include <stdio.h>

#include "mitsubishi_heavy.h"

union MH_ir_encoded MH_ir_encode_ac_state(struct MH_ac_state ac_state) {
    union MH_ir_encoded encoded = (union MH_ir_encoded){};

    for (int i = 0; i < 5; i++) {
        encoded.data_32bit[i] = -1;
    }


    //                                      inv         inv         inv         inv         inv         inv        
    //  0x52, 0xAE, 0xC3, 0x1A, 0xE5, 0xF6, 0x09, 0xF9, 0x06, 0xfe, 0x01, 0x3f, 0xc0, 0x37, 0xc8, 0xff, 0x00, 0x7f, 0x80, 0x00
    //   0     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18    19 
    // |       IDENTIFICATION       |   MODE    |TEMPERATURE| FAN SPEED | VERT AIR  |  HOR AIR  |   NIGHT   |
    // |                            |   POWER   |           |           |  3D AUTO  |           |   SILENT  |
    // |                            |   CLEAN   |

    // IDENTIFICATION
    encoded.data_8bit[0] = 0x52;
    encoded.data_8bit[1] = 0xae;
    encoded.data_8bit[2] = 0xc3;
    encoded.data_8bit[3] = 0x1a;
    encoded.data_8bit[4] = 0xe5;
    encoded.data_8bit[17] = 0x7f;

    // POWER
    encoded.data_8bit[5] = encoded.data_8bit[5] & (ac_state.power | ~(0x08));

    // MODE 
    encoded.data_8bit[5] = encoded.data_8bit[5] & (ac_state.mode | ~(0x07));

    // TEMPERATURE 
    encoded.data_8bit[7] = encoded.data_8bit[7] & ((~(ac_state.temperatre - 17)) | ~0x0f);

    // FAN SPEED 
    encoded.data_8bit[9] = encoded.data_8bit[9] & (ac_state.fan_speed | ~(0x0f));

    // VERT AIR
    encoded.data_8bit[11] = encoded.data_8bit[11] & (ac_state.vertical_air | ~(0xe0));

    // HOR AIR
    encoded.data_8bit[13] = encoded.data_8bit[13] & (ac_state.horizontal_air | ~(0x0f));

    // 3D AUTO 
    encoded.data_8bit[11] = encoded.data_8bit[11] & (ac_state.auto_3d_mode | ~(0x12));

    // NIGHT SETBACK
    encoded.data_8bit[15] = encoded.data_8bit[15] & (ac_state.night_setback | ~(0x40));

    // SILENT MODE
    encoded.data_8bit[15] = encoded.data_8bit[15] & (ac_state.silent_mode | ~(0x80));

    // CLEAN AND ALERGEN
    encoded.data_8bit[5] = encoded.data_8bit[5] & (ac_state.clean_alergen | ~(0x60));

    // produce inverted pairs 
    for (int i = 5; i <= 17; i += 2) {
        encoded.data_8bit[i + 1] = ~encoded.data_8bit[i];
    }

    return encoded;
}

void MH_fan_str(struct MH_ac_state* ac_state, char* buff) {
    switch (ac_state->fan_speed) {
    case MH_fan_speed_auto:
        sprintf(buff, "Auto");
        break;
    case MH_fan_speed_1:
        sprintf(buff, "1");
        break;
    case MH_fan_speed_2:
        sprintf(buff, "2");
        break;
    case MH_fan_speed_3:
        sprintf(buff, "3");
        break;
    case MH_fan_speed_4:
        sprintf(buff, "4");
        break;
    case MH_fan_speed_eco:
        sprintf(buff, "ECO");
        buff = "ECO";
        break;
    case MH_fan_speed_high:
        sprintf(buff, "High");
        break;
    }
}
void MH_mode_str(struct MH_ac_state* ac_state, char* buff) {
    if (ac_state->power == MH_power_off) {
        sprintf(buff, "OFF");
        return;
    }
    switch (ac_state->mode) {
    case MH_mode_auto:
        sprintf(buff, "Auto");
        break;
    case MH_mode_cooling:
        sprintf(buff, "Cooling");
        break;
    case MH_mode_drying:
        sprintf(buff, "Drying");
        break;
    case MH_mode_fan:
        sprintf(buff, "Fan Only");
        break;
    case MH_mode_heating:
        sprintf(buff, "Heating");
        break;
    }
}

inline void MH_temperature_str(struct MH_ac_state* ac_state, char* buff) {
    sprintf(buff, "%d", ac_state->temperatre);
}

void MH_fan_change(struct MH_ac_state* ac_state, int direction) {
    switch (ac_state->fan_speed) {
    case MH_fan_speed_auto:
    case MH_fan_speed_high:
    case MH_fan_speed_eco:
        if (direction < 0) { ac_state->fan_speed = MH_fan_speed_1; }
        if (direction > 0) { ac_state->fan_speed = MH_fan_speed_1; }
        break;
    case MH_fan_speed_1:
        if (direction < 0) { ac_state->fan_speed = MH_fan_speed_1; }
        if (direction > 0) { ac_state->fan_speed = MH_fan_speed_2; }
        break;
    case MH_fan_speed_2:
        if (direction < 0) { ac_state->fan_speed = MH_fan_speed_1; }
        if (direction > 0) { ac_state->fan_speed = MH_fan_speed_3; }
        break;
    case MH_fan_speed_3:
        if (direction < 0) { ac_state->fan_speed = MH_fan_speed_2; }
        if (direction > 0) { ac_state->fan_speed = MH_fan_speed_4; }
        break;
    case MH_fan_speed_4:
        if (direction < 0) { ac_state->fan_speed = MH_fan_speed_3; }
        if (direction > 0) { ac_state->fan_speed = MH_fan_speed_4; }
        break;
    }
}

void MH_mode_change(struct MH_ac_state* ac_state, enum MH_mode mode) {
    if (ac_state->power == MH_power_off) { ac_state->power = MH_power_on; }
    ac_state->mode = mode;
}

void MH_power_toggle(struct MH_ac_state* ac_state) {
    if (ac_state->power == MH_power_off) { ac_state->power = MH_power_on; }
    else { ac_state->power = MH_power_off; }
}

void MH_temperature_change(struct MH_ac_state* ac_state, int direction) {
    if (direction > 0) { ac_state->temperatre += 1; }
    if (direction < 0) { ac_state->temperatre -= 1; }

    if (ac_state->temperatre > MH_MAX_TEMP) { ac_state->temperatre = MH_MAX_TEMP; }
    if (ac_state->temperatre < MH_MIN_TEMP) { ac_state->temperatre = MH_MIN_TEMP; }

}