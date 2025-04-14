
#include "board.h"

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    DDRB |= (1 << PB5);    // Set pin 13 (PB5) as output
    while (1) {
        PORTB ^= (1 << PB5);    // Toggle pin 13 (PB5)
        _delay_ms(400);
    }

    return 0;
}