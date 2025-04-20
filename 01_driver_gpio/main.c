
#include "board.h"

#include "Drivers/gpio/gpio.h"
#include <util/delay.h>

int main(void) {

    gpio_config(GPIO_PORTB, GPIO_5, GPIO_OUTPUT_INITIAL_LOW);
    gpio_config(GPIO_PORTB, GPIO_1, GPIO_OUTPUT_INITIAL_LOW);
    gpio_config(GPIO_PORTD, GPIO_3, GPIO_OUTPUT_INITIAL_HIGH);
    gpio_config(GPIO_PORTC, GPIO_1, GPIO_OUTPUT_INITIAL_LOW);
    gpio_config(GPIO_PORTC, GPIO_2, GPIO_OUTPUT_INITIAL_HIGH);

    gpio_write_pin(GPIO_PORTB, GPIO_5, GPIO_HIGH);
    gpio_write_pin(GPIO_PORTD, GPIO_3, GPIO_LOW);
    gpio_write_pin(GPIO_PORTB, GPIO_5, GPIO_LOW);

    while (1) {
        gpio_write_pin(GPIO_PORTB, GPIO_5, !gpio_read_pin(GPIO_PORTB, GPIO_5));
        gpio_write_pin(GPIO_PORTB, GPIO_1, !gpio_read_pin(GPIO_PORTB, GPIO_1));
        _delay_ms(500);
        gpio_write_pin(GPIO_PORTC, GPIO_1, !gpio_read_pin(GPIO_PORTC, GPIO_1));
        _delay_ms(500);
    }

    return 0;
}