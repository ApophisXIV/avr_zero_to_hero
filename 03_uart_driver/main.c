
#include "Drivers/gpio/gpio.h"
#include "Drivers/uart/uart.h"
#include "board.h"
extern void GPIO_EXTI_callback(GPIO_port_t port, GPIO_pin_t pin, GPIO_pin_state_t state);

int main(void) {

    GPIO_config(GPIO_PORTB, GPIO_5, GPIO_OUTPUT_INITIAL_LOW);

    UART_init();

    printf("UART initialized\n");

    while (1) {

        // Get user input
        char input = getchar();
        if (input == '1') {
            GPIO_write_pin(GPIO_PORTB, GPIO_5, GPIO_HIGH);
            printf("LED ON\n");
        } else if (input == '0') {
            GPIO_write_pin(GPIO_PORTB, GPIO_5, GPIO_LOW);
            printf("LED OFF\n");
        } else {
            printf("Invalid input\n");
        }
        printf("You entered: %c\n", input);
    }

    return 0;
}
