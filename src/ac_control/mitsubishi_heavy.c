#include <stdio.h>

#include "mitsubishi_heavy.h"
#include "ir/ir_send.h"

union MH_ir_encoded MH_ir_encode_ac_state(volatile struct MH_ac_state* ac_state) {
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
    encoded.data_8bit[5] = encoded.data_8bit[5] & (atomic_load(&ac_state->power) | ~(0x08));

    // MODE 
    encoded.data_8bit[5] = encoded.data_8bit[5] & (atomic_load(&ac_state->mode) | ~(0x07));

    // TEMPERATURE 
    encoded.data_8bit[7] = encoded.data_8bit[7] & ((~(atomic_load(&ac_state->temperature) - 17)) | ~0x0f);

    // FAN SPEED 
    encoded.data_8bit[9] = encoded.data_8bit[9] & (atomic_load(&ac_state->fan_speed) | ~(0x0f));

    // VERT AIR
    encoded.data_8bit[11] = encoded.data_8bit[11] & (atomic_load(&ac_state->vertical_air) | ~(0xe0));

    // HOR AIR
    encoded.data_8bit[13] = encoded.data_8bit[13] & (atomic_load(&ac_state->horizontal_air) | ~(0x0f));

    // 3D AUTO 
    encoded.data_8bit[11] = encoded.data_8bit[11] & (atomic_load(&ac_state->auto_3d_mode) | ~(0x12));

    // NIGHT SETBACK
    encoded.data_8bit[15] = encoded.data_8bit[15] & (atomic_load(&ac_state->night_setback) | ~(0x40));

    // SILENT MODE
    encoded.data_8bit[15] = encoded.data_8bit[15] & (atomic_load(&ac_state->silent_mode) | ~(0x80));

    // CLEAN AND ALERGEN
    encoded.data_8bit[5] = encoded.data_8bit[5] & (atomic_load(&ac_state->clean_alergen) | ~(0x60));

    // produce inverted pairs 
    for (int i = 5; i <= 17; i += 2) {
        encoded.data_8bit[i + 1] = ~encoded.data_8bit[i];
    }

    return encoded;
}

int MH_init(int pin, struct MH_ac_state* ac_state) {
    if (IR_init(pin) == -1) {
        return 1;
    }

    if (ac_state == NULL) { return 0; }
    *ac_state = (struct MH_ac_state){
                                    .power = MH_power_on,
                                    .temperature = 22,
                                    .mode = MH_mode_auto,
                                    .fan_speed = MH_fan_speed_1,
                                    .vertical_air = MH_vertical_air_stop,
                                    .horizontal_air = MH_horizontal_air_stop,
                                    .auto_3d_mode = MH_3d_auto_off,
                                    .night_setback = MH_night_setback_off,
                                    .silent_mode = MH_silent_mode_off,
                                    .clean_alergen = MH_clean_allergen_off,
    };
    return 0;
}

void MH_send(volatile struct MH_ac_state* ac_state) {
    IR_send(MH_ir_encode_ac_state(ac_state));
}


void MH_fan_str(struct MH_ac_state* ac_state, char* buff) {
    enum MH_fan_speed current_sped = atomic_load(&ac_state->fan_speed);
    switch (current_sped) {
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
    enum MH_power current_power = atomic_load(&ac_state->power);
    if (current_power == MH_power_off) {
        sprintf(buff, "OFF");
        return;
    }
    enum MH_mode current_mode = atomic_load(&ac_state->mode);
    switch (current_mode) {
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
    int current_temperature = atomic_load(&ac_state->temperature);
    sprintf(buff, "%d", current_temperature);
}

inline void MH_fan_cycle(volatile struct MH_ac_state* ac_state, int direction) {
    enum MH_fan_speed current_fan_speed = atomic_load(&ac_state->fan_speed);

    switch (current_fan_speed) {
    case MH_fan_speed_auto:
    case MH_fan_speed_high:
    case MH_fan_speed_eco:
        if (direction < 0) { current_fan_speed = MH_fan_speed_1; }
        if (direction > 0) { current_fan_speed = MH_fan_speed_1; }
        break;
    case MH_fan_speed_1:
        if (direction < 0) { current_fan_speed = MH_fan_speed_1; }
        if (direction > 0) { current_fan_speed = MH_fan_speed_2; }
        break;
    case MH_fan_speed_2:
        if (direction < 0) { current_fan_speed = MH_fan_speed_1; }
        if (direction > 0) { current_fan_speed = MH_fan_speed_3; }
        break;
    case MH_fan_speed_3:
        if (direction < 0) { current_fan_speed = MH_fan_speed_2; }
        if (direction > 0) { current_fan_speed = MH_fan_speed_4; }
        break;
    case MH_fan_speed_4:
        if (direction < 0) { current_fan_speed = MH_fan_speed_3; }
        if (direction > 0) { current_fan_speed = MH_fan_speed_4; }
        break;
    }
    atomic_store(&ac_state->fan_speed, current_fan_speed);
}

inline void MH_fan_set(volatile struct MH_ac_state* ac_state, enum MH_fan_speed fan) {
    atomic_store(&ac_state->fan_speed, fan);
}

inline  void MH_mode_set(volatile struct MH_ac_state* ac_state, enum MH_mode mode) {
    if (atomic_load(&ac_state->power) == MH_power_off) { atomic_store(&ac_state->power, MH_power_on); }
    atomic_store(&ac_state->mode, mode);
}

inline void MH_power_toggle(volatile struct MH_ac_state* ac_state) {
    atomic_uint current_power = atomic_load(&ac_state->power);
    if (current_power == MH_power_off) { current_power = MH_power_on; }
    else { current_power = MH_power_off; }
    atomic_store(&ac_state->power, current_power);
}

inline void MH_power_set(volatile struct MH_ac_state* ac_state, enum MH_power power) {
    atomic_store(&ac_state->power, power);
}


inline void MH_temperature_change_target(volatile struct MH_ac_state* ac_state, int target) {
    if (target > MH_MAX_TEMP) { target = MH_MAX_TEMP; }
    if (target < MH_MIN_TEMP) { target = MH_MIN_TEMP; }
    atomic_store(&ac_state->temperature, target);
}

inline void MH_temperature_change_delta(volatile struct MH_ac_state* ac_state, int delta) {
    int current_temperature = atomic_load(&ac_state->temperature);
    current_temperature += delta;

    if (current_temperature > MH_MAX_TEMP) { current_temperature = MH_MAX_TEMP; }
    if (current_temperature < MH_MIN_TEMP) { current_temperature = MH_MIN_TEMP; }
    atomic_store(&ac_state->temperature, current_temperature);
}


void MH_vertical_air_str(struct MH_ac_state* ac_state, char* buff) {
    enum MH_vertical_air vertical_air = atomic_load(&ac_state->vertical_air);
    switch (vertical_air) {
    case MH_vertical_air_down:
        sprintf(buff, "Down");
        break;
    case MH_vertical_air_middle_down:
        sprintf(buff, "Middle Down");
        break;
    case MH_vertical_air_middle:
        sprintf(buff, "Middle");
        break;
    case MH_vertical_air_middle_up:
        sprintf(buff, "MIiddle Up");
        break;
    case MH_vertical_air_up:
        sprintf(buff, "Up");
        break;
    case MH_vertical_air_swing:
        sprintf(buff, "Swing");
        break;
    case MH_vertical_air_stop:
        buff[0] = '\0';
        break;
    }
}

void MH_vertical_air_cycle(volatile struct MH_ac_state* ac_state) {
    enum MH_vertical_air current_vertical_air = atomic_load(&ac_state->vertical_air);
    switch (current_vertical_air) {
    case MH_vertical_air_stop:
        current_vertical_air = MH_vertical_air_swing;
        break;
    case MH_vertical_air_swing:
        current_vertical_air = MH_vertical_air_down;
        break;
    case MH_vertical_air_down:
        current_vertical_air = MH_vertical_air_middle_down;
        break;
    case MH_vertical_air_middle_down:
        current_vertical_air = MH_vertical_air_middle;
        break;
    case MH_vertical_air_middle:
        current_vertical_air = MH_vertical_air_middle_up;
        break;
    case MH_vertical_air_middle_up:
        current_vertical_air = MH_vertical_air_up;
        break;
    case MH_vertical_air_up:
        current_vertical_air = MH_vertical_air_stop;
        break;
    }
    atomic_store(&ac_state->vertical_air, current_vertical_air);
}

void MH_vertical_air_set(volatile struct MH_ac_state* ac_state, enum MH_vertical_air vertical_air) {
    atomic_store(&ac_state->vertical_air, vertical_air);
}

void MH_horizontal_air_str(struct MH_ac_state* ac_state, char* buff) {
    enum MH_horizontal_air horizontal_air = atomic_load(&ac_state->horizontal_air);
    switch (horizontal_air) {
    case MH_horizontal_air_swing:
        sprintf(buff, "Auto");
        break;
    case MH_horizontal_air_left:
        sprintf(buff, "Left");
        break;
    case MH_horizontal_air_middle_left:
        sprintf(buff, "Middle Left");
        break;
    case MH_horizontal_air_middle:
        sprintf(buff, "Middle");
        break;
    case MH_horizontal_air_middle_right:
        sprintf(buff, "Middle Right");
        break;
    case MH_horizontal_air_right:
        sprintf(buff, "Right");
        break;
    case MH_horizontal_air_stop:
        buff[0] = '\0';
        break;
    case MH_horizontal_air_left_right:
        sprintf(buff, "Left Right");
        break;
    case MH_horizontal_air_right_left:
        sprintf(buff, "Right Left");
        break;
    }

}

void MH_horizontal_air_cycle(volatile struct MH_ac_state* ac_state) {
    enum MH_horizontal_air current_horizontal_air = atomic_load(&ac_state->horizontal_air);
    switch (current_horizontal_air) {
    case MH_horizontal_air_stop:
        current_horizontal_air = MH_horizontal_air_swing;
        break;
    case MH_horizontal_air_swing:
        current_horizontal_air = MH_horizontal_air_left;
        break;
    case MH_horizontal_air_left:
        current_horizontal_air = MH_horizontal_air_right;
        break;
    case MH_horizontal_air_right:
        current_horizontal_air = MH_horizontal_air_middle;
        break;
    case MH_horizontal_air_middle:
        current_horizontal_air = MH_horizontal_air_middle_left;
        break;
    case MH_horizontal_air_middle_left:
        current_horizontal_air = MH_horizontal_air_middle_right;
        break;
    case MH_horizontal_air_middle_right:
        current_horizontal_air = MH_horizontal_air_left_right;
        break;
    case MH_horizontal_air_left_right:
        current_horizontal_air = MH_horizontal_air_right_left;
        break;
    case MH_horizontal_air_right_left:
        current_horizontal_air = MH_horizontal_air_stop;
        break;

    }
    atomic_store(&ac_state->horizontal_air, current_horizontal_air);
}

void MH_horizontal_air_set(volatile struct MH_ac_state* ac_state, enum MH_horizontal_air horizontal_air) {
    atomic_store(&ac_state->horizontal_air, horizontal_air);
}

void MH_3d_auto_str(struct MH_ac_state* ac_state, char* buff) {
    enum MH_3d_auto _3d_auto = atomic_load(&ac_state->auto_3d_mode);
    switch (_3d_auto) {
    case MH_3d_auto_off:
        sprintf(buff, "Off");
        break;
    case MH_3d_auto_on:
        sprintf(buff, "On");
        break;
    }

}
void MH_3d_auto_toggle(volatile struct MH_ac_state* ac_state) {
    enum MH_3d_auto _3d_auto = atomic_load(&ac_state->auto_3d_mode);
    if (_3d_auto == MH_3d_auto_off) { _3d_auto = MH_3d_auto_on; }
    else { _3d_auto = MH_3d_auto_off; }
    atomic_store(&ac_state->auto_3d_mode, _3d_auto);

}
void MH_3d_auto_set(volatile struct MH_ac_state* ac_state, enum MH_3d_auto _3d_auto) {
    atomic_store(&ac_state->auto_3d_mode, _3d_auto);
}

void MH_night_setback_str(struct MH_ac_state* ac_state, char* buff) {
    enum MH_night_setback night_setback = atomic_load(&ac_state->night_setback);
    switch (night_setback) {
    case MH_night_setback_off:
        sprintf(buff, "Off");
        break;
    case MH_night_setback_on:
        sprintf(buff, "On");
        break;
    }

}
void MH_night_setback_toggle(volatile struct MH_ac_state* ac_state) {
    enum MH_night_setback night_setback = atomic_load(&ac_state->night_setback);
    if (night_setback == MH_night_setback_off) { night_setback = MH_night_setback_on; }
    else { night_setback = MH_night_setback_off; }
    atomic_store(&ac_state->night_setback, night_setback);

}
void MH_night_setback_set(volatile struct MH_ac_state* ac_state, enum MH_night_setback night_setback) {
    atomic_store(&ac_state->night_setback, night_setback);
}

void MH_silent_mode_str(struct MH_ac_state* ac_state, char* buff) {
    enum MH_silent_mode silent_mode = atomic_load(&ac_state->silent_mode);
    switch (silent_mode) {
    case MH_silent_mode_off:
        sprintf(buff, "Off");
        break;
    case MH_silent_mode_on:
        sprintf(buff, "On");
        break;
    }
}
void MH_silent_mode_toggle(volatile struct MH_ac_state* ac_state) {
    enum MH_silent_mode silent_mode = atomic_load(&ac_state->silent_mode);
    if (silent_mode == MH_silent_mode_off) { silent_mode = MH_silent_mode_on; }
    else { silent_mode = MH_silent_mode_off; }
    atomic_store(&ac_state->silent_mode, silent_mode);

}
void MH_silent_mode_set(volatile struct MH_ac_state* ac_state, enum MH_silent_mode silent_mode) {
    atomic_store(&ac_state->silent_mode, silent_mode);
}

void MH_clean_alergen_str(struct MH_ac_state* ac_state, char* buff) {
    enum MH_clean_alergen clean_alergen = atomic_load(&ac_state->clean_alergen);
    switch (clean_alergen) {
    case MH_clean_alergen_clean:
        sprintf(buff, "Clean");
        break;
    case MH_clean_alergen_alergen:
        sprintf(buff, "Alergen");
        break;
    case MH_clean_allergen_off:
        sprintf(buff, "Off");
        break;
    }
}
void MH_clean_allergen_toggle(volatile struct MH_ac_state* ac_state, enum MH_clean_alergen clean_alergen) {
    enum MH_clean_alergen current_clean_alergen = atomic_load(&ac_state->clean_alergen);

    if (current_clean_alergen == MH_clean_allergen_off) {
        atomic_store(&ac_state->clean_alergen, clean_alergen);
        return;
    }
    else if (current_clean_alergen == clean_alergen) {
        atomic_store(&ac_state->clean_alergen, MH_clean_allergen_off);
        return;
    }

}
void MH_clean_alergen_set(volatile struct MH_ac_state* ac_state, enum MH_clean_alergen clean_alergen) {
    atomic_store(&ac_state->clean_alergen, clean_alergen);
}

void MH_other_str(struct MH_ac_state* ac_state, char* buff) {
    enum MH_3d_auto _3d_auto = atomic_load(&ac_state->auto_3d_mode);
    enum MH_night_setback night_setback = atomic_load(&ac_state->night_setback);
    enum MH_silent_mode silent_mode = atomic_load(&ac_state->silent_mode);
    enum MH_clean_alergen clean_alergen = atomic_load(&ac_state->clean_alergen);


    sprintf(buff, "%s %s %s %s", _3d_auto == MH_3d_auto_on ? "3D auto" : "", night_setback == MH_night_setback_on ? "Night" : "", silent_mode == MH_silent_mode_on ? "Silent" : "", clean_alergen == MH_clean_alergen_clean ? "Clean" : clean_alergen == MH_clean_alergen_alergen ? "Alergen" : "");
}