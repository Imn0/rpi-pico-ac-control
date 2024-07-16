#if MQTT_ENABLED
#include "lwip/apps/mqtt.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include <string.h>

#include "lwipopts.h"
#include "mqtt.h"

static mqtt_client_t* mqtt_client;

static struct mqtt_callback_data callback_data;
static struct MH_ac_state* ac_state;

static void mqtt_incoming_publish_cb(void* arg, const char* topic, u32_t tot_len) {
    printf("AOFH\n");
    struct mqtt_callback_data* data = (struct mqtt_callback_data*)arg;
    strncpy(data->topic, topic, sizeof(data->topic) - 1);
    data->topic[sizeof(data->topic) - 1] = '\0';  // Ensure null-termination
}

static void mqtt_incoming_data_cb(void* arg, const u8_t* data, u16_t len, u8_t flags) {
    struct mqtt_callback_data* cb_data = (struct mqtt_callback_data*)arg;
    printf("Message content: %.*s\n", len, (char*)data);
    printf("Message is from topic: %s\n", cb_data->topic);

    if (strcmp(cb_data->topic, POWER_SET_TOPIC) == 0) {}
    else if (strcmp(cb_data->topic, MODE_SET_TOPIC) == 0) {
        if (strcmp((char*)data, "auto") == 0) {
            MH_mode_set(ac_state, MH_mode_auto);
        }
        else if (strcmp((char*)data, "off") == 0) {
            MH_power_set(ac_state, MH_power_off);
        }
        else if (strcmp((char*)data, "cool") == 0) {
            MH_mode_set(ac_state, MH_mode_cooling);
        }
        else if (strcmp((char*)data, "heat") == 0) {
            MH_mode_set(ac_state, MH_mode_heating);
        }
        else if (strcmp((char*)data, "dry") == 0) {
            MH_mode_set(ac_state, MH_mode_drying);
        }
        else if (strcmp((char*)data, "fan_only") == 0) {
            MH_mode_set(ac_state, MH_mode_fan);
        }
    }
    else if (strcmp(cb_data->topic, TEMPERATURE_SET_TOPIC) == 0) {
        MH_temperature_change_target(ac_state, (int)atof((char*)data));
    }
    else if (strcmp(cb_data->topic, FAN_SET_TOPIC) == 0) {
        if (strcmp((char*)data, "auto") == 0) {
            MH_fan_set(ac_state, MH_fan_speed_auto);
        }
        else if (strcmp((char*)data, "low") == 0 || strcmp((char*)data, "1") == 0) {
            MH_fan_set(ac_state, MH_fan_speed_1);
        }
        else if (strcmp((char*)data, "medium") == 0 || strcmp((char*)data, "3") == 0) {
            MH_fan_set(ac_state, MH_fan_speed_3);
        }
        else if (strcmp((char*)data, "high") == 0) {
            MH_fan_set(ac_state, MH_fan_speed_high);
        }
        else if (strcmp((char*)data, "4") == 0) {
            MH_fan_set(ac_state, MH_fan_speed_4);
        }
        else if (strcmp((char*)data, "2") == 0) {
            MH_fan_set(ac_state, MH_fan_speed_2);
        }
        else if (strcmp((char*)data, "eco") == 0) {
            MH_fan_set(ac_state, MH_fan_speed_eco);
        }
    }
    MH_send(ac_state);
}

static void mqtt_sub_request_cb(void* arg, err_t result) {
    printf("Subscribe result: %d\n", result);
}

static void mqtt_pub_request_cb(void* arg, err_t result) {
    printf("Publish result: %d\n", result);
}

static void mqtt_connection_cb(mqtt_client_t* client, void* arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connected\n");

        for (uint i = 0; i < NUM_SUBSCRIBE_TOPICS; i++) {
            mqtt_sub_unsub(client,
                           subscribe_topics[i],
                           0,
                           mqtt_sub_request_cb,
                           arg,
                           1);
        }
    }
    else {
        printf("MQTT connection failed: %d\n", status);
    }
}

void mqtt_update_send() {
    char message[50];
    MH_mode_str(ac_state, message);
    err_t err = mqtt_publish(mqtt_client,
                             MODE_STATE_TOPIC,
                             message,
                             strlen(message),
                             1,
                             0,
                             mqtt_pub_request_cb,
                             NULL);
    if (err != ERR_OK) {
        printf("Publish error on topic %s: %d\n", MODE_STATE_TOPIC, err);
    }
    MH_temperature_str(ac_state, message);
    err = mqtt_publish(mqtt_client,
                             TEMPERATURE_STATE_TOPIC,
                             message,
                             strlen(message),
                             1,
                             0,
                             mqtt_pub_request_cb,
                             NULL);
    if (err != ERR_OK) {
        printf("Publish error on topic %s: %d\n", TEMPERATURE_STATE_TOPIC, err);
    }
    MH_fan_str(ac_state, message);
    err = mqtt_publish(mqtt_client,
                             FAN_STATE_TOPIC,
                             message,
                             strlen(message),
                             1,
                             0,
                             mqtt_pub_request_cb,
                             NULL);
    if (err != ERR_OK) {
        printf("Publish error on topic %s: %d\n", FAN_STATE_TOPIC, err);
    }
}

int mqtt_init(struct MH_ac_state* _ac_state) {
    ac_state = _ac_state;
    struct mqtt_connect_client_info_t ci = {};

    ci.client_id = MQTT_CLIENT_ID;
    ci.keep_alive = 60;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_retain = 0;
    ci.will_qos = 0;

#if defined(MQTT_CLIENT_USER) && defined(MQTT_CLIENT_PASSWORD)
    ci.client_user = MQTT_CLIENT_USER;
    ci.client_pass = MQTT_CLIENT_PASSWORD;
#endif

    ip_addr_t mqtt_server_ip;
    if (!ip4addr_aton(MQTT_SERVER_IP, &mqtt_server_ip)) {
        printf("Invalid mqtt server ip address.");
        return 1;
    }

    mqtt_client = mqtt_client_new();
    mqtt_set_inpub_callback(mqtt_client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, &callback_data);

    err_t err = mqtt_client_connect(mqtt_client,
                                    &mqtt_server_ip,
                                    MQTT_PORT,
                                    mqtt_connection_cb,
                                    NULL,
                                    &ci);

    if (err != ERR_OK) {
        printf("MQTT client connect failed: %d\n", err);
        return 1;
    }
    return 0;
}
#endif