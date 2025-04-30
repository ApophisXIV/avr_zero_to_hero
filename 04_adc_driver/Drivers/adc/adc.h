#ifndef ADC_H
#define ADC_H

#include "../../board.h"
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

struct adc_handle;
typedef struct adc_handle ADC_handle_t;

#define MAX_CHANNELS 8

typedef enum {
    ADC_AREF         = 0 << REFS1 | 0 << REFS0,
    ADC_AVCC         = 0 << REFS1 | 1 << REFS0,
    ADC_INTERNAL_1_1 = 1 << REFS1 | 1 << REFS0,
} ADC_reference_t;

typedef enum {
    ADC_CLK_DIV_1   = 0 << ADPS2 | 0 << ADPS1 | 0 << ADPS0,
    ADC_CLK_DIV_2   = 0 << ADPS2 | 0 << ADPS1 | 1 << ADPS0,
    ADC_CLK_DIV_4   = 0 << ADPS2 | 1 << ADPS1 | 0 << ADPS0,
    ADC_CLK_DIV_8   = 0 << ADPS2 | 1 << ADPS1 | 1 << ADPS0,
    ADC_CLK_DIV_16  = 1 << ADPS2 | 0 << ADPS1 | 0 << ADPS0,
    ADC_CLK_DIV_32  = 1 << ADPS2 | 0 << ADPS1 | 1 << ADPS0,
    ADC_CLK_DIV_64  = 1 << ADPS2 | 1 << ADPS1 | 0 << ADPS0,
    ADC_CLK_DIV_128 = 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0,
} ADC_preescaler_t;

typedef enum {
    CH0 = 1 << 0,
    CH1 = 1 << 1,
    CH2 = 1 << 2,
    CH3 = 1 << 3,
    CH4 = 1 << 4,
    CH5 = 1 << 5,
    CH6 = 1 << 6,
    CH7 = 1 << 7,
} ADC_channel_t;

typedef enum {
    CH_VBG  = (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0),    // Internal 1.1V bandgap reference
    CH_TEMP = (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0),    // Internal temperature sensor
} ADC_alt_channel_t;

typedef enum {
    ADC_NO_AUTO_TRIGGER              = -1,
    ADC_FREE_RUNNING_MODE            = 0 << ADTS2 | 0 << ADTS1 | 0 << ADTS0,
    ADC_ANALOG_COMPARATOR            = 0 << ADTS2 | 0 << ADTS1 | 1 << ADTS0,
    ADC_EXTERNAL_INTERRUPT_REQUEST_0 = 0 << ADTS2 | 1 << ADTS1 | 0 << ADTS0,
    ADC_TIMER0_COMPARE_MATCH_A       = 0 << ADTS2 | 1 << ADTS1 | 1 << ADTS0,
    ADC_TIMER0_OVERFLOW              = 1 << ADTS2 | 0 << ADTS1 | 0 << ADTS0,
    ADC_TIMER1_COMPARE_MATCH_B       = 1 << ADTS2 | 0 << ADTS1 | 1 << ADTS0,
    ADC_TIMER1_OVERFLOW              = 1 << ADTS2 | 1 << ADTS1 | 0 << ADTS0,
    ADC_TIMER1_CAPTURE_EVENT         = 1 << ADTS2 | 1 << ADTS1 | 1 << ADTS0,
} ADC_trigger_t;

typedef enum {
    ADC_10B_RESOLUTION,
    ADC_8B_RESOLUTION,
} ADC_bitres_t;

typedef struct {
    ADC_bitres_t bits;
    ADC_reference_t reference;
    ADC_preescaler_t preescaler;
    ADC_channel_t low_power_channels;    // e.g: CH1|CH2|CH3|CH4|CH5|CH6
    ADC_trigger_t trigger_source;
} ADC_init_t;

typedef enum {
    ADC_BUSY,
    ADC_EOC,
    ADC_IDLE,
} ADC_state_t;

float ADC_get_ref_voltage(ADC_handle_t *hadc);
void ADC_read_voltages(ADC_handle_t *hadc, ADC_channel_t channels, float *res_array, uint8_t n_res);

#ifdef USE_ADC_EEPROM_CALIBRATED_INT_REF
ADC_handle_t *ADC_init(ADC_init_t *cfg, void (*feedback_callback)(void));
#else
ADC_handle_t *ADC_init(ADC_init_t *cfg);
#endif
uint16_t ADC_read(ADC_handle_t *hadc, ADC_channel_t ch);
float ADC_read_voltage(ADC_handle_t *hadc, ADC_channel_t ch);
float ADC_read_vcc_voltage(ADC_handle_t *hadc);

#ifdef USE_ADC_EEPROM_CALIBRATED_INT_REF
ADC_handle_t *ADC_init_IT(ADC_init_t *cfg, void (*feedback_callback)(void));
#else
ADC_handle_t *ADC_init_IT(ADC_init_t *cfg);
#endif
void ADC_read_IT(ADC_channel_t ch);
void ADC_read_voltage_IT(ADC_channel_t ch);
void ADC_read_vcc_voltage_IT(ADC_handle_t *hadc);

extern void ADC_EOC_callback(ADC_handle_t *hadc);

#endif    // ADC_H