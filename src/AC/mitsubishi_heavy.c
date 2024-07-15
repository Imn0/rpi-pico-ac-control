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
