#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/interp.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "communication/web/frontend.h"
#include "communication/mqtt/mqtt.h"
#include "networking/wifi.h"
#include "ac_control/mitsubishi_heavy.h"


static void gpio_callback_reboot() {
    printf("REBOOTING\n");
    reset_usb_boot(0, 0);
}

int main() {


    if (!stdio_init_all()) {
        return 1;
    }

    int err;

    err = wifi_init_and_connect();
    if (err) {
        return 1;
    }
    struct MH_ac_state ac;
    err = MH_init(IR_LED_PIN, &ac);
    if (err) {
        return 1;
    }
    err = frontend_init(&ac);
    if (err) {
        return 1;
    }
#if MQTT_ENABLED
    err = mqtt_init(&ac);
    if (err) {
        return 1;
    }
#endif

    for (int i = 0; i < 3; i++) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(300);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(300);
    }

    while (true) {
        sleep_ms(5000);
#if MQTT_ENABLED
        mqtt_update_send();
#endif
    }

}
