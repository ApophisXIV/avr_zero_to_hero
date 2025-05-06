
#include "board.h"

#include "Drivers/gpio/gpio.h"
#include <util/delay.h>

#define TEST_GPIO 3

#if TEST_GPIO == 1 || TEST_GPIO == 2 || TEST_GPIO == 3
#include <avr/interrupt.h>
#endif

void GPIO_EXTI_callback(GPIO_port_t port, GPIO_pin_t pin, GPIO_pin_state_t state) {
    if (pin == GPIO_3) {    // INT1 (Rising)
        GPIO_toggle_pin(GPIO_PORTB, GPIO_4);
    }
    if (pin == GPIO_2) {    // INT0 (Falling)
        GPIO_toggle_pin(GPIO_PORTB, GPIO_3);
    }
    if (port == GPIO_PORTB && pin == GPIO_1) {    // Change on any state
        GPIO_toggle_pin(GPIO_PORTB, GPIO_2);
    }
    if (port == GPIO_PORTB && pin == GPIO_0 && state == GPIO_EDGE_RISING) {    // Separate composite state
        GPIO_toggle_pin(GPIO_PORTD, GPIO_6);
    }
    if (port == GPIO_PORTB && pin == GPIO_1 && state == GPIO_EDGE_FALLING) {    // Separate composite state
        GPIO_toggle_pin(GPIO_PORTD, GPIO_7);
    }
}

int main(void) {

    UART_init();

    GPIO_config(GPIO_PORTB, GPIO_5 | GPIO_1, GPIO_OUTPUT_INITIAL_LOW);
    GPIO_config(GPIO_PORTC, GPIO_1, GPIO_OUTPUT_INITIAL_LOW);
    GPIO_config(GPIO_PORTC, GPIO_2, GPIO_OUTPUT_INITIAL_HIGH);

    GPIO_write_pin(GPIO_PORTB, GPIO_5, GPIO_HIGH);
    GPIO_write_pin(GPIO_PORTB, GPIO_5, GPIO_LOW);

    // Output LEDs
    GPIO_config(GPIO_PORTB, GPIO_2 | GPIO_3 | GPIO_4, GPIO_OUTPUT_INITIAL_HIGH);
    GPIO_config(GPIO_PORTD, GPIO_6 | GPIO_7, GPIO_OUTPUT_INITIAL_HIGH);
#if TEST_GPIO == 1
    GPIO_config(GPIO_PORTD, GPIO_3, GPIO_INPUT_IT_LEVEL_CHANGE);
    sei();
#elif TEST_GPIO == 2
    GPIO_config(GPIO_PORTD, GPIO_2, GPIO_INPUT_IT_FALLING);
    GPIO_config(GPIO_PORTD, GPIO_3, GPIO_INPUT_IT_RISING);
    sei();
#elif TEST_GPIO == 3
    // INT0 / INT1
    GPIO_config(GPIO_PORTD, GPIO_2, GPIO_INPUT_IT_FALLING);
    GPIO_config(GPIO_PORTD, GPIO_3, GPIO_INPUT_IT_RISING);
    // PCINT0 / PCINT1
    GPIO_config(GPIO_PORTB, GPIO_0, GPIO_INPUT_IT_LEVEL_CHANGE);
    GPIO_config(GPIO_PORTB, GPIO_1, GPIO_INPUT_IT_LEVEL_CHANGE);
    sei();
#endif

    while (1) {
        GPIO_write_pin(GPIO_PORTB, GPIO_5, !GPIO_read_pin(GPIO_PORTB, GPIO_5));
        _delay_ms(500);
#if TEST_GPIO == 0
        GPIO_write_pin(GPIO_PORTB, GPIO_1, !GPIO_read_pin(GPIO_PORTB, GPIO_1));
        GPIO_write_pin(GPIO_PORTC, GPIO_1, !GPIO_read_pin(GPIO_PORTC, GPIO_1));
        _delay_ms(500);
#endif
    }

    return 0;
}