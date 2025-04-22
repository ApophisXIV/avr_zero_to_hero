
#include "board.h"

#include "Drivers/gpio/gpio.h"
#include "Drivers/timer/timer.h"
#include <util/delay.h>

void TIM_period_elapsed_callback(TIM_handle_t *htim) {
    gpio_toggle_pin(GPIO_PORTB, GPIO_0);
}

int main(void) {

    TIM_handle_t htim = {
        .timer        = TIM_0,
        .state        = TIM_STATE_READY,
        .clk_source   = TIM_CLK_INTERNAL_PRESCALER_DIV64,
        .preset_value = 0,
    };

    TIM_base_start_IT(&htim);

    while (1) {
    }

    return 0;
}