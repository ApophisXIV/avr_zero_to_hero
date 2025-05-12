
#include "board.h"

#include "Drivers/gpio/gpio.h"
#include "Drivers/timer/timer.h"
#include <avr/interrupt.h>

extern void TIM_period_elapsed_callback(TIM_handle_t *htim) {
    GPIO_toggle_pin(GPIO_PORTB, GPIO_1);
}

extern void TIM_CTC_callback(TIM_handle_t *htim) {
    GPIO_toggle_pin(GPIO_PORTB, GPIO_5);
}

int main(void) {

    // Arduino nano: PB1 = D9 , PB5 = D13

    GPIO_config(GPIO_PORTB, GPIO_5, GPIO_OUTPUT_INITIAL_LOW);
    GPIO_config(GPIO_PORTB, GPIO_1, GPIO_OUTPUT_INITIAL_LOW);

    TIM_init_t cfg0 = {
        .timer        = TIM_0,
        .clk_source   = TIM_CLK_INTERNAL_PRESCALER_DIV256,
        .preset_value = 131,    // 2ms
        .mode         = NORMAL_TIMER_AUTORELOAD,
    };

    TIM_handle_t *htim0 = TIM_base_init(&cfg0);
    if (!htim0) {
        printf("Error initializing timer 0\n");
        return -1;
    }

    TIM_init_t cfg2 = {
        .timer        = TIM_2,
        .clk_source   = TIM_CLK_INTERNAL_PRESCALER_DIV64,
        .preset_value = 251,    // 1ms
        .mode         = CTC_CHANNEL_A_NO_OUTPUT,
    };

    TIM_handle_t *htim2 = TIM_CTC_init(&cfg2);
    if (!htim2) {
        printf("Error initializing timer 2\n");
        return -1;
    }

    TIM_base_start_IT(htim0);
    TIM_CTC_A_start_IT(htim2);
    sei();

    while (1) {
    }

    return 0;
}
