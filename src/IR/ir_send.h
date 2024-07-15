// Timings in microseconds
#include "AC/mitsubishi_heavy.h"
#define PAUSE_SPACE 0
#define HEADER_MARK 3158
#define HEADER_SPACE 1610
#define BIT_MARK 364
#define ZERO_SPACE 419
#define ONE_SPACE 1207

void IR_init(int ir_pin);
void IR_send(union MH_ir_encoded encoded);

void _IR_send_pulse(uint32_t duration_us);
void _IR_send_header();
void _IR_send_bit(bool bit);
void _IR_send_byte(uint8_t byte);
void _IR_send_space(uint32_t duration_us) ;

