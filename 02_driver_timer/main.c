
#include "board.h"

#include "Drivers/gpio/gpio.h"
#include "Drivers/timer/timer.h"
#include <util/delay.h>

void TIM_period_elapsed_callback(TIM_handle_t *htim) {
    gpio_toggle_pin(GPIO_PORTB, GPIO_0);
}

int main(void) {

    TIM_init_t tim0_cfg = {
        .timer        = TIM_0,
        .clk_source   = TIM_CLK_INTERNAL_PRESCALER_DIV64,
        .preset_value = 0,
        .mode.normal  = NORMAL_AUTO_RELOAD,
    };

    TIM_handle_t *htim0;

    TIM_base_init(htim0, &tim0_cfg);

    TIM_base_start(htim0);

    while (1) {
        if (TIM_get_state(htim0) == TIM_STATE_TIMEOUT) {
        }
    }

    return 0;
}
