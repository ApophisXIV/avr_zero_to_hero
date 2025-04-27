
#include "Drivers/gpio/gpio.h"
#include "Drivers/uart/uart.h"
#include "board.h"

int main(void) {

    gpio_config(GPIO_PORTB, GPIO_5, GPIO_OUTPUT_INITIAL_LOW);

    UART_init();

    printf("UART initialized\n");

    while (1) {

        // Get user input
        char input = getchar();
        if (input == '1') {
            gpio_write_pin(GPIO_PORTB, GPIO_5, GPIO_HIGH);
            printf("LED ON\n");
        } else if (input == '0') {
            gpio_write_pin(GPIO_PORTB, GPIO_5, GPIO_LOW);
            printf("LED OFF\n");
        } else {
            printf("Invalid input\n");
        }
        printf("You entered: %c\n", input);
    }

    return 0;
}
