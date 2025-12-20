#pragma once
#if MQTT_ENABLED

#include "ac_control/mitsubishi_heavy.h"

struct mqtt_callback_data {
    char topic[128];
};

int mqtt_init(struct MH_ac_state* _ac_state);
void mqtt_update_send();
#endif
