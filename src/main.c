#include "lwipopts.h"
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

#include "frontend/frontend.h"
#include "wifi.h"
#include "IR/ir_send.h"
#include "IR/mitsubishi_heavy_ir.h"
#include "AC/mitsubishi_heavy.h"

#define IR_LED_PIN 14

static void gpio_callback_reboot() {
    printf("REBOOTING\n");
    reset_usb_boot(0, 0);
}

int main() {

    stdio_init_all();

    gpio_init(23);
    gpio_set_dir(23, GPIO_OUT);
    gpio_pull_up(23);


    gpio_init(15);
    gpio_set_dir(15, GPIO_IN);
    gpio_set_irq_enabled_with_callback(15, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback_reboot);
    irq_set_enabled(PIO0_IRQ_0, true);

    wifi_init_and_connect();
    frontend_init();

    struct MH_ac_state ac = {
                            .power = MH_power_on,
                            .temperatre = 22,
                            .mode = MH_mode_auto,
                            .fan_speed = MH_fan_speed_1,
                            .vertical_air = MH_vertical_air_swing,
                            .horizontal_air = MH_horizontal_air_stop,
                            .auto_3d_mode = MH_3d_auto_off,
                            .night_setback = MH_night_setback_off,
                            .silent_mode = MH_silent_mode_off,
                            .clean_alergen = MH_clean_alergen_off,
    };


    IR_init(IR_LED_PIN);
    while (true) {
        IR_send(MH_ir_encode_ac_state(ac));
        sleep_ms(3000);
    }

}
