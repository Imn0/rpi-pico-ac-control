target_compile_definitions(${PROGRAM_NAME} PRIVATE IR_LED_PIN=14)
target_compile_definitions(${PROGRAM_NAME} PRIVATE IR_PIO_ENABLED=1)

# Uncomment below if you want to setup network without dhcp 
# target_compile_definitions(${PROGRAM_NAME} PRIVATE
#     IP_ADDR=\"192.168.XXX.XXX\"
#     IP_NETMASK=\"255.255.255.0\"
#     IP_GATEWAY=\"192.168.XXX.XXX\"
# )

# MQTT CONFIGURATION
target_compile_definitions(${PROGRAM_NAME} PRIVATE MQTT_ENABLED=0)


target_compile_definitions(${PROGRAM_NAME} PRIVATE
    MQTT_CLIENT_ID="Pico-AC-mqtt"
    POWER_SET_TOPIC="Pico-AC/ac/power/set"
    MODE_SET_TOPIC="Pico-AC/ac/mode/set"
    TEMPERATURE_SET_TOPIC="Pico-AC/ac/temperature/set"
    FAN_SET_TOPIC="Pico-AC/ac/fan/set"
    POWER_STATE_TOPIC="Pico-AC/ac/power/state"
    MODE_STATE_TOPIC="Pico-AC/ac/mode/state"
    TEMPERATURE_STATE_TOPIC="Pico-AC/ac/temperature/state"
    FAN_STATE_TOPIC="Pico-AC/ac/fan/state"
)
