
#include "Drivers/adc/adc.h"
#include "Drivers/gpio/gpio.h"
#include "Drivers/timer/timer.h"
#include "Drivers/uart/uart.h"
#include "board.h"
#include <util/delay.h>

void ADC_calibration_callback(void) {
    gpio_toggle_pin(GPIO_PORTB, GPIO_5);
    _delay_ms(300);
    gpio_toggle_pin(GPIO_PORTB, GPIO_5);
    _delay_ms(300);
}

int main(void) {

    gpio_config(GPIO_PORTB, GPIO_5, GPIO_OUTPUT_INITIAL_LOW);

    UART_init();

    printf(
        "\n"
        "============================================================\n"
        "||                      FIUBA - CONAE                     ||\n"
        "||                                                        ||\n"
        "||        Project: BMS - Battery Management System        ||\n"
        "||        Firmware Version: v1.0.0                        ||\n"
        "||        Build Date: %s %s                ||\n"
        "||                                                        ||\n"
        "||        Battery monitoring and control system           ||\n"
        "||                                                        ||\n"
        "||        Contact: guerodriguez@fi.uba.ar                 ||\n"
        "============================================================\n\n",
        __DATE__, __TIME__);

    ADC_init_t cfg = {
        .bits               = ADC_10B_RESOLUTION,
        .preescaler         = ADC_CLK_DIV_128,    // 16MHZ /128 / 13.5 (ADC_CLK_Conversion_time) = 9.26kS/s
        .low_power_channels = CH3 | CH4 | CH5,
        .reference          = ADC_AVCC,
        .trigger_source     = ADC_NO_AUTO_TRIGGER,
    };

    ADC_handle_t *hadc = ADC_init(&cfg, ADC_calibration_callback);
    if (hadc == NULL) {
        printf("ERROR: ADC initialization\n");
        return -1;
    }

    printf("ADC initialized\n");

    TIM_init_t cfg0 = {
        .timer        = TIM_0,
        .clk_source   = TIM_CLK_INTERNAL_PRESCALER_DIV64,
        .preset_value = 6,    // 1ms
        .mode         = CTC_CHANNEL_A_NO_OUTPUT,
    };

    TIM_handle_t *htim0 = TIM_base_init(&cfg0);
    if (!htim0) {
        printf("Error initializing timer 0\n");
        return -1;
    }

    printf("TIM initialized\n");

    float v_res[3];

    while (1) {

        ADC_read_voltages(hadc, CH0 | CH1 | CH2, v_res, 3);

        printf("v1:%.2f ; v2:%.2f ; v3:%.2f\n\r", v_res[0], v_res[1], v_res[2]);
        _delay_ms(400);
    }

    return 0;
}
