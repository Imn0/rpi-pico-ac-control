#pragma once
#if MQTT_ENABLED

#include "ac_control/mitsubishi_heavy.h"

struct mqtt_callback_data {
    char topic[128];
};

static const char* subscribe_topics[] = {
    POWER_SET_TOPIC,
    MODE_SET_TOPIC,
    TEMPERATURE_SET_TOPIC,
    FAN_SET_TOPIC,
};

#define NUM_SUBSCRIBE_TOPICS (sizeof(subscribe_topics) / sizeof(subscribe_topics[0]))

static const char* publish_topics[] = {
    POWER_STATE_TOPIC,
    MODE_STATE_TOPIC,
    TEMPERATURE_STATE_TOPIC,
    FAN_STATE_TOPIC,
};

#define NUM_PUBLISH_TOPICS (sizeof(publish_topics) / sizeof(publish_topics[0]))

int mqtt_init(struct MH_ac_state* _ac_state);
void mqtt_update_send();
#endif