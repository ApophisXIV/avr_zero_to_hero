
#include "board.h"

#include <avr/io.h>

#include "Drivers/clock/clock.h"

void delay_nop(uint16_t ms) {
    for (uint16_t i = 0; i < ms; i++) {
        __asm__ __volatile__("nop");
    }
}

int main(void) {

    DDRB |= (1 << PB5);      // Set pin 13 (PB5) as output
    PORTB &= ~(1 << PB5);    // Set pin 13 (PB5) low

    while (1) {

#ifdef USE_CPU_CLOCK_PRESCALER_AT_RUNTIME
        for (clk_prescaler_t prescaler = CLK_DIV_1; prescaler <= CLK_DIV_256; prescaler++) {
            clock_prescaler_config(prescaler);
            PORTB |= (1 << PB5);
            delay_nop(CLK_DIV_256 - prescaler + 10);
            PORTB &= ~(1 << PB5);
            delay_nop(CLK_DIV_256 - prescaler + 10);
        }
#else
        PORTB ^= (1 << PB5);
        delay_nop(200);
#endif
    }

    return 0;
}