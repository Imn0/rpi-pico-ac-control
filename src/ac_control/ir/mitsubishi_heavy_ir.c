#if IR_PIO_ENABLED

#include "hardware/pio.h"
#include "mitsubishi_heavy_ir.h"
#include "hardware/clocks.h"
#include <stdio.h>

#include "mitsubishi_heavy_152b_protocol.pio.h"

int mitsubishi_heavy_ir_heavy_init(PIO pio, uint pin_idx) {
    uint mitsubishi_heavy_ir_header_offset;
    if (pio_can_add_program(pio, &mitsubishi_heavy_ir_header_program)) {
        mitsubishi_heavy_ir_header_offset = pio_add_program(pio, &mitsubishi_heavy_ir_header_program);
    }
    else return -1;

    int header_offset_sm = pio_claim_unused_sm(pio, true);
    if (header_offset_sm == -1) return -1;
    mitsubishi_heavy_ir_header_program_init(pio,
                                            header_offset_sm,
                                            mitsubishi_heavy_ir_header_offset,
                                            2 * (1 / 300e-6f),
                                            32);

    uint burst_offset;
    if (pio_can_add_program(pio, &mitsubishi_heavy_ir_burst_program)) {
        burst_offset = pio_add_program(pio, &mitsubishi_heavy_ir_burst_program);
    }
    else return -1;

    int burst_offset_sm = pio_claim_unused_sm(pio, true);
    if (burst_offset_sm == -1) return -1;
    mitsubishi_heavy_ir_burst_program_init(pio,
                                           burst_offset_sm,
                                           burst_offset,
                                           pin_idx,
                                           38.222e3);
    return header_offset_sm;
}


int mitsubishi_heavy_ir_data_init(PIO pio) {

    uint itsubishi_heavy_ir_data_offset;
    if (pio_can_add_program(pio, &mitsubishi_heavy_ir_data_program)) {
        itsubishi_heavy_ir_data_offset = pio_add_program(pio, &mitsubishi_heavy_ir_data_program);
    }
    else return -1;

    int mitsubishi_heavy_ir_data_sm = pio_claim_unused_sm(pio, true);
    if (mitsubishi_heavy_ir_data_sm == -1) {
        return -1;
    }

    mitsubishi_heavy_ir_data_program_init(pio,
                                          mitsubishi_heavy_ir_data_sm,
                                          itsubishi_heavy_ir_data_offset,
                                          2 * (1 / 225e-6f),
                                          32);

    return mitsubishi_heavy_ir_data_sm;
}
#endif