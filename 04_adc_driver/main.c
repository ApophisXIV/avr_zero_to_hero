#include "Drivers/adc/adc.h"
#include "Drivers/adc/adc_cal.h"
#include "Drivers/gpio/gpio.h"
#include "Drivers/timer/timer.h"
#include "Drivers/uart/uart.h"
#include "board.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/* -------------------------------- ADC Task -------------------------------- */
ADC_handle_t *hadc0             = NULL;
static ADC_channel_t current_ch = CH0;
#define CH_VCC 3
typedef struct {
    float voltage;
    uint8_t samples;
} ADC_measurement_t;

ADC_measurement_t measurements[4] = {0};

static volatile bool is_vcc_measurment = false;    // NOTE: Is more scalable to use function pointers to update ones
void ADC_EOC_callback(ADC_handle_t *hadc, uint16_t value_mV) {
    if (is_vcc_measurment) {
        GPIO_write_pin(GPIO_PORTB, GPIO_3, GPIO_LOW);
        measurements[CH_VCC].voltage += value_mV / 1000.0f;
        measurements[CH_VCC].samples++;
    } else {
        GPIO_write_pin(GPIO_PORTB, GPIO_5, GPIO_LOW);
        measurements[current_ch].voltage += value_mV / 1000.0f;
        measurements[current_ch].samples++;
        current_ch = (current_ch + 1) % 3;
    }
}

void task_measure_adc_channel_update(void) {
    is_vcc_measurment = false;
    ADC_IT_read_mV(hadc0, current_ch);
    GPIO_write_pin(GPIO_PORTB, GPIO_5, GPIO_HIGH);
    GPIO_toggle_pin(GPIO_PORTB, GPIO_0);
}

void task_measure_vcc_update(void) {
    is_vcc_measurment = true;
    ADC_IT_read_VCC_mV(hadc0);
    GPIO_write_pin(GPIO_PORTB, GPIO_3, GPIO_HIGH);
    GPIO_toggle_pin(GPIO_PORTB, GPIO_1);
}
/* -------------------------------------------------------------------------- */

/* -------------------------------- UART Task ------------------------------- */
#define N_SAMPLES 10
void task_print_averages_update(void) {
    for (uint8_t i = 0; i < sizeof(measurements) / sizeof(measurements[0]); i++) {
        if (i == CH_VCC) {
            printf(">VCC:%.3f\n", measurements[i].voltage / measurements[i].samples);
            // printf(">VCC:%.3f:%u\n", measurements[i].voltage / measurements[i].samples, measurements[i].samples);
        } else {
            printf(">ADC%d:%.3f\n", i, measurements[i].voltage / measurements[i].samples);
            // printf(">ADC%d:%.3f:%u\n", i, measurements[i].voltage / measurements[i].samples, measurements[i].samples);
        }
        measurements[i].voltage = 0.0f;
        measurements[i].samples = 0;
    }

    GPIO_toggle_pin(GPIO_PORTB, GPIO_2);
}
/* -------------------------------------------------------------------------- */

#define SAMPLE_TIME_MS_ADC      10
#define START_TIME_VCC_MS       25
#define SAMPLE_TIME_VCC_MS      30
#define START_PRINT_AVG_TIME_MS 305
#define PRINT_AVG_TIME_MS       300

typedef struct {
    void (*task_update)(void);
    uint32_t start_time;
    uint32_t last_time;
    uint32_t update_time;
} task_t;

#define N_TASKS 3
task_t tasks[N_TASKS] = {
    {.task_update = task_measure_adc_channel_update, .start_time = SAMPLE_TIME_MS_ADC, .last_time = 0, .update_time = SAMPLE_TIME_MS_ADC},
    {.task_update = task_measure_vcc_update, .start_time = START_TIME_VCC_MS, .last_time = 0, .update_time = SAMPLE_TIME_VCC_MS},
    {.task_update = task_print_averages_update, .start_time = START_PRINT_AVG_TIME_MS, .last_time = 0, .update_time = PRINT_AVG_TIME_MS},
};

static volatile uint32_t tick = 0;
void TIM_CTC_callback(TIM_handle_t *htim) {
    tick++;
    GPIO_toggle_pin(GPIO_PORTB, GPIO_4);
    for (uint8_t i = 0; i < N_TASKS; i++) {
        if ((tick == tasks[i].start_time) || (tick > tasks[i].start_time && (tick - tasks[i].last_time >= tasks[i].update_time))) {
            tasks[i].last_time = tick;
            tasks[i].task_update();
        }
    }
}

void GPIO_EXTI_callback(GPIO_port_t port, GPIO_pin_t pin, GPIO_pin_state_t state) {
    ADC_calibrate(hadc0);
    tick = 0;
    for (uint8_t i = 0; i < sizeof(measurements) / sizeof(measurements[0]); i++) {
        measurements[i].voltage = 0.0f;
        measurements[i].samples = 0;
    }
    current_ch = CH0;
}

int main(void) {

    UART_init();
    printf("UART_INIT_OK\n");

    GPIO_config(GPIO_PORTB, GPIO_0, GPIO_OUTPUT_INITIAL_HIGH);    // LED-Task ADC measurement
    GPIO_config(GPIO_PORTB, GPIO_1, GPIO_OUTPUT_INITIAL_HIGH);    // LED-Task ADC vcc measurement
    GPIO_config(GPIO_PORTB, GPIO_2, GPIO_OUTPUT_INITIAL_HIGH);    // LED-Task Print avg
    GPIO_config(GPIO_PORTB, GPIO_3, GPIO_OUTPUT_INITIAL_LOW);     // GPIO_diff_time
    GPIO_config(GPIO_PORTB, GPIO_4, GPIO_OUTPUT_INITIAL_LOW);     // GPIO_tick
    GPIO_config(GPIO_PORTB, GPIO_5, GPIO_OUTPUT_INITIAL_LOW);     // GPIO_sample
    GPIO_config(GPIO_PORTD, GPIO_2, GPIO_INPUT_IT_FALLING);       // Forced calibration (D2)

    ADC_init_t adc_config = {
        .bits               = ADC_10B_RESOLUTION,
        .low_power_channels = LP_CH0 | LP_CH1 | LP_CH2 | LP_CH3 | LP_CH4 | LP_CH5,
        .preescaler         = ADC_CLK_DIV_128,
        .reference          = ADC_AVCC,
        .trigger_source     = ADC_NO_AUTO_TRIGGER,
    };

    hadc0 = ADC_IT_init(&adc_config);
    if (!hadc0) {
        printf("ERR_ADC_INIT\n");
        return 1;
    }
    printf("ADC_INIT_OK\n");

    if (!ADC_is_calibrated()) {
        ADC_calibrate(hadc0);
        printf("ADC_CALIBRATE_OK\n");
    } else {
        ADC_set_calibration(hadc0, ADC_get_calibration());
    }

    TIM_init_t tim0_cfg = {
        .timer        = TIM_0,
        .clk_source   = TIM_CLK_INTERNAL_PRESCALER_DIV64,
        .mode         = CTC_CHANNEL_A_NO_OUTPUT,
        .preset_value = 250,    // 1ms
    };

    TIM_handle_t *htim0 = TIM_CTC_init(&tim0_cfg);
    if (!htim0) {
        printf("Error initializing TIM0 CTC A\n");
        return 1;
    }
    printf("TIM0_INIT_OK\n");

    GPIO_toggle_pin(GPIO_PORTB, GPIO_4);
    GPIO_toggle_pin(GPIO_PORTB, GPIO_4);

    sei();

    TIM_CTC_A_start_IT(htim0);

    while (1) {
    }

    return 0;
}
