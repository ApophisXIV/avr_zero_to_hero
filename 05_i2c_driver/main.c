#include "Drivers/gpio/gpio.h"
#include "Drivers/uart/uart.h"
#include "board.h"
#include "lib/ILS94202/ils94202.h"
#include <avr/interrupt.h>
#include <util/delay.h>

static volatile bool is_panic_mode = false;

void GPIO_EXTI_callback(GPIO_port_t port, GPIO_pin_t pin, GPIO_pin_state_t state) {
    is_panic_mode = true;
}

int main(void) {

    UART_init();
    printf("UART_INIT_OK\n");

    GPIO_config(GPIO_PORTD, GPIO_2, GPIO_INPUT_IT_FALLING);    // "Panic" mode
    GPIO_config(GPIO_PORTB, GPIO_0, GPIO_OUTPUT_INITIAL_HIGH);

    ILS94202_init_t bms_cfg = {
        .address  = ILS94202_SLAVE_ADDRESS_DEFAULT,
        .i2c_freq = I2C_FREQ_400KHZ,
        .i2c_port = GPIO_PORTC,
        .scl_pin  = GPIO_5,
        .sda_pin  = GPIO_4,
    };

    ILS94202_handle_t *hbms = ILS94202_init(&bms_cfg);
    printf("BMS_INIT_OK\n");

    sei();

    while (1) {
        if (is_panic_mode) {
            cli();

            // TODO: Apagar otros perifericos

            while (!ILS94202_is_power_down(hbms)) {
                // Intento apagar BMS hasta morir sin alimentacion
                ILS94202_set_power_down_mode(hbms);
                _delay_ms(2);
            }
        } else {
            GPIO_toggle_pin(GPIO_PORTB, GPIO_0);
            _delay_ms(20);    // NOTE: NO usar delay en caso real
        }
    }

    return 0;
}
