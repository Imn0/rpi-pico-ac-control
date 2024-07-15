#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "stdio.h"

#include "ir_send.h"
#include "AC/mitsubishi_heavy.h"
#include "IR/mitsubishi_heavy_ir.h"




#if IR_PIO_ENABLED
#include "hardware/pio.h"
PIO pio;
int tx_header_sm;
int tx_data_sm;
#endif
int __ir_pin = -1;

void IR_init(int ir_pin) {
    __ir_pin = ir_pin;

#if IR_PIO_ENABLED
    pio = pio0;
    tx_header_sm = mitsubishi_heavy_ir_heavy_init(pio, ir_pin);
    tx_data_sm = mitsubishi_heavy_ir_data_init(pio);
#else 
    gpio_init(__ir_pin);
    gpio_set_dir(__ir_pin, GPIO_OUT);
    gpio_put(__ir_pin, 1);
#endif
}

void IR_send(union MH_ir_encoded encoded) {
#if IR_PIO_ENABLED
    pio_sm_put(pio, tx_data_sm, encoded.data_32bit[0]);
    pio_sm_put(pio, tx_data_sm, encoded.data_32bit[1]);
    pio_sm_put(pio, tx_data_sm, encoded.data_32bit[2]);
    pio_sm_put(pio, tx_data_sm, encoded.data_32bit[3]);
    pio_sm_put(pio, tx_data_sm, encoded.data_32bit[4]);
    pio_sm_exec(pio, tx_header_sm, pio_encode_irq_set(false, 5));
#else 
    _IR_send_pulse(HEADER_MARK);
    _IR_send_space(HEADER_SPACE);

    for (size_t i = 0; i < 19; i++) {
        _IR_send_byte(encoded.data_8bit[i]);
    }

    _IR_send_space(PAUSE_SPACE);
    _IR_send_pulse(BIT_MARK);
#endif

}

void _IR_send_pulse(uint32_t duration_us) {
    uint32_t end_time = time_us_32() + duration_us;
    while (time_us_32() < end_time) {
        gpio_put(__ir_pin, 1);
        sleep_us(13);
        gpio_put(__ir_pin, 0);
        sleep_us(13);
    }
}

void _IR_send_space(uint32_t duration_us) {
    sleep_us(duration_us);
}

void _IR_send_header() {
    _IR_send_pulse(HEADER_MARK);
    _IR_send_space(HEADER_SPACE);
}

void _IR_send_bit(bool bit) {
    _IR_send_pulse(BIT_MARK);
    _IR_send_space(bit ? ONE_SPACE : ZERO_SPACE);
}

void _IR_send_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        _IR_send_bit(byte & 1);
        byte >>= 1;
    }
}