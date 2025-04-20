
#include "board.h"

#include <avr/io.h>

#include "Drivers/clock/clock.h"

void delay_nop(uint16_t ms) {
    while (ms--) {
        for (uint16_t i = 0; i < 800; i++) {
            __asm__ __volatile__("nop");
        }
    }
}

int main(void) {

    DDRB |= (1 << PB5);    // Set pin 13 (PB5) as output

    while (1) {

#ifdef USE_CPU_CLOCK_PRESCALER_AT_RUNTIME
        for (clk_prescaler_t prescaler = CLK_DIV_1; prescaler <= CLK_DIV_256; prescaler++) {
            clock_prescaler_config(prescaler);
            PORTB ^= (1 << PB5);
            delay_nop(200);
        }
#else
        PORTB ^= (1 << PB5);
        delay_nop(200);
#endif
    }

    return 0;
}