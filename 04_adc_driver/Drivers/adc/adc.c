#include "adc.h"
#include "adc_cal.h"
#include <avr/interrupt.h>
#include <stddef.h>
#include <util/delay.h>

#include "../uart/uart.h"

#define ADC_10_BITS 10U
#define ADC_8_BITS  8U

#define SAMPLE_HOLD_CAPACITY             14e-12f
#define SAMPLE_HOLD_DISCHARGE_TIME(load) (5 * SAMPLE_HOLD_CAPACITY * load)

/* ------------------------ Calibration coefficients ------------------------ */
static uint16_t vref_internal_mV  = 1100;
static uint16_t vref_avcc_mv      = 5000;
static int16_t vref_drift_avcc_mV = 0;
/* -------------------------------------------------------------------------- */

struct adc_handle {
    bool is_avaliable;
    ADC_init_t config;
    ADC_state_t state;
    ADC_reference_t last_reference;
};

typedef enum {
    ADC_IT_IDLE,
    ADC_IT_EOC,
    ADC_IT_START_READ,
    ADC_IT_START_READ_VOLTAGE,
    ADC_IT_START_READ_HIGH_IMPEDANCE,
    ADC_IT_START_READ_HIGH_IMPEDANCE_VOLTAGE,
    ADC_IT_START_READ_VCC_VOLTAGE,
} ADC_IT_last_call_t;

static ADC_IT_last_call_t ADC_IT_last_state = ADC_IT_IDLE;

static ADC_handle_t adc_handle = {
    .is_avaliable     = true,
    .state            = ADC_STOPED,
    .last_reference   = ADC_AREF,    // P.O.R value: Datasheet 23.9.1 ADMUX – ADC Multiplexer Selection Register
    .config.reference = ADC_AREF,    // P.O.R value: Datasheet 23.9.1 ADMUX – ADC Multiplexer Selection Register
};

/* --------------------------------- Getters -------------------------------- */
uint16_t ADC_get_value(ADC_resolution_t resolution) {
    uint8_t lsb = ADCL;
    if (resolution == ADC_8B_RESOLUTION) return ADCH;
    return (ADCH << 8) | lsb;
}

ADC_reference_t ADC_get_reference(ADC_handle_t *hadc) {
    return (ADC_reference_t)(ADMUX & (1 << REFS1 | 1 << REFS0));
}

static inline __attribute__((always_inline)) uint16_t ADC_get_steps(ADC_handle_t *hadc) {
    return 1 << (ADC_10_BITS - (hadc->config.bits / ADC_8B_RESOLUTION) * (ADC_10_BITS - ADC_8_BITS));
}

static inline __attribute__((always_inline)) uint16_t ADC_get_voltage_mV(ADC_handle_t *hadc) {
    if (hadc->config.reference == ADC_INTERNAL_1_1) {
        return ((uint32_t)vref_internal_mV * ADC_get_value(hadc->config.bits)) / ADC_get_steps(hadc);
    } else if (hadc->config.reference == ADC_AVCC) {
        return (((uint32_t)vref_avcc_mv * ADC_get_value(hadc->config.bits)) / ADC_get_steps(hadc));
    }
    return 0;
}

/* --------------------------------- Setters -------------------------------- */
static inline __attribute__((always_inline)) void ADC_set_resolution(ADC_handle_t *hadc, ADC_resolution_t res) {
    if (hadc->config.bits == res) return;
    hadc->config.bits = res;
    ADMUX |= res;
}

static inline __attribute__((always_inline)) void ADC_set_prescaler(ADC_handle_t *hadc, ADC_preescaler_t preescaler) {
    if (hadc->config.preescaler == preescaler) return;
    hadc->config.preescaler = preescaler;
    ADCSRA &= ~(1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0);
    ADCSRA |= preescaler;
}

static inline __attribute__((always_inline)) void ADC_set_low_power_channels(ADC_handle_t *hadc, ADC_low_power_channel_t lp_channels) {
    if (hadc->config.low_power_channels == lp_channels) return;
    hadc->config.low_power_channels = lp_channels;
    DIDR0 &= ~(1 << ADC5D | 1 << ADC4D | 1 << ADC3D | 1 << ADC2D | 1 << ADC1D | 1 << ADC0D);
    DIDR0 |= lp_channels;
}

inline __attribute__((always_inline)) void ADC_set_reference(ADC_handle_t *hadc, ADC_reference_t reference) {
    if (hadc->last_reference == reference && hadc->config.reference == reference) return;
    hadc->last_reference   = hadc->config.reference;
    hadc->config.reference = reference;
    ADMUX &= ~(1 << REFS1 | 1 << REFS0);
    ADMUX |= reference;
}

static inline __attribute__((always_inline)) void ADC_set_trigger(ADC_handle_t *hadc, ADC_trigger_t trigger_source) {
    if (hadc->config.trigger_source == trigger_source) return;
    if (trigger_source == ADC_NO_AUTO_TRIGGER) {    // NOTE: Check if needed. Power On Reset value ADATE = 0. (No auto trigger)
        ADCSRA &= ~(1 << ADATE);
        return;
    };
    hadc->config.trigger_source = trigger_source;
    ADCSRB &= ~(1 << ADTS2 | 1 << ADTS1 | 1 << ADTS0);
    ADCSRB |= trigger_source;
}

static inline __attribute__((always_inline)) void ADC_set_channel(ADC_channel_t ch) {
    if ((ADMUX & (1 << MUX3 | 1 << MUX2 | 1 << MUX1 | 1 << MUX0)) == ch) return;
    ADMUX &= ~(1 << MUX3 | 1 << MUX2 | 1 << MUX1 | 1 << MUX0);
    ADMUX |= ch;
}

/* ---------------------------------- Utils --------------------------------- */
static ADC_handle_t *ADC_register_handle(void) {
    if (adc_handle.is_avaliable) {
        adc_handle.is_avaliable = false;
        return &adc_handle;
    }
    return NULL;
}

static void ADC_unregister_handle(ADC_handle_t *hadc) {
    if (!adc_handle.is_avaliable) {
        adc_handle.is_avaliable = true;
    }
}

static inline __attribute__((always_inline)) void ADC_enable(ADC_handle_t *hadc) {
    hadc->state = ADC_IDLE;
    ADCSRA |= (1 << ADEN);
}

static inline __attribute__((always_inline)) void ADC_disable(ADC_handle_t *hadc) {
    ADCSRA &= ~(1 << ADEN);
    hadc->state = ADC_STOPED;
}

static inline __attribute__((always_inline)) void ADC_delay(uint16_t delay_us) {
    for (uint16_t i = 0; i < (F_CPU / 1000000) * delay_us; i++) {
        __asm__ __volatile__("nop");
    }
}

uint16_t ADC_read_VCC_mV(ADC_handle_t *hadc) {
    ADC_set_reference(hadc, ADC_AVCC);
    ADC_read(hadc, CH_VBG);    // Dummy read
    uint16_t vcc = ((uint32_t)vref_internal_mV * ADC_get_steps(hadc) / ADC_read(hadc, CH_VBG)) + vref_drift_avcc_mV;
    ADC_set_reference(hadc, hadc->last_reference);
    return vcc;
}

void ADC_set_calibration(ADC_handle_t *hadc, void *calibration) {
    hadc->state = ADC_CALIBRATING;
    if (ADC_internal_ref_is_calibrated()) {
        vref_internal_mV = ADC_get_calibrated_internal_ref_mV((ADC_calibration_t *)calibration);
    }
    if (ADC_avcc_ref_is_calibrated()) {
        vref_avcc_mv       = ADC_get_calibrated_avcc_ref_mV((ADC_calibration_t *)calibration);
        vref_drift_avcc_mV = ADC_get_calibrated_avcc_ref_drift_mV((ADC_calibration_t *)calibration);
    }
    hadc->state = ADC_IDLE;
}

ADC_state_t ADC_get_state(ADC_handle_t *hadc) {
    return hadc->state;
}

/* --------------------- Public functions : Polling mode -------------------- */
ADC_handle_t *ADC_init(ADC_init_t *cfg) {
    ADC_handle_t *hadc = ADC_register_handle();
    if (hadc == NULL) return NULL;

    ADC_set_resolution(hadc, cfg->bits);
    ADC_set_prescaler(hadc, cfg->preescaler);
    ADC_set_low_power_channels(hadc, cfg->low_power_channels);
    ADC_set_reference(hadc, cfg->reference);
    ADC_set_trigger(hadc, cfg->trigger_source);
    ADC_enable(hadc);
    return hadc;
}

void ADC_deinit(ADC_handle_t *hadc) {
    ADC_disable(hadc);
    ADC_unregister_handle(hadc);
}

static uint16_t ADC_read_base(ADC_handle_t *hadc, ADC_channel_t channel, uint16_t delay_us) {
    ADC_set_channel(channel);
    ADC_delay(delay_us);

    hadc->state = ADC_BUSY;
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    hadc->state = ADC_IDLE;
    return ADC_get_value(hadc->config.bits);
}

uint16_t ADC_high_impedance_read(ADC_handle_t *hadc, ADC_channel_t channel, uint8_t load_kohms) {
    return ADC_read_base(hadc, channel, SAMPLE_HOLD_DISCHARGE_TIME(load_kohms));
}
uint16_t ADC_high_impedance_read_mV(ADC_handle_t *hadc, ADC_channel_t channel, uint8_t load_kohms) {
    ADC_high_impedance_read(hadc, channel, load_kohms);
    return ADC_get_voltage_mV(hadc);
}

uint16_t ADC_read(ADC_handle_t *hadc, ADC_channel_t channel) {
    return ADC_read_base(hadc, channel, 10);    // NOTE - Maybe a small delay is needed (aprox 10us)
}
uint16_t ADC_read_mV(ADC_handle_t *hadc, ADC_channel_t channel) {
    ADC_read(hadc, channel);
    return ADC_get_voltage_mV(hadc);
}

/* -------------------- Public functions : Interrupt mode -------------------- */
ADC_handle_t *ADC_IT_init(ADC_init_t *cfg) {
    ADC_handle_t *hadc = ADC_init(cfg);
    if (hadc == NULL) return NULL;
    ADCSRA |= (1 << ADIE);
    return hadc;
}

void ADC_IT_read_base(ADC_handle_t *hadc, ADC_channel_t channel, uint16_t delay_us) {
    if (hadc->state == ADC_BUSY) return;

    hadc->state = ADC_BUSY;
    ADC_set_channel(channel);
    ADC_delay(delay_us);

    ADCSRA |= (1 << ADSC);
}

void ADC_IT_high_impedance_read(ADC_handle_t *hadc, ADC_channel_t channel, uint8_t load_kohms) {
    ADC_IT_last_state = ADC_IT_START_READ_HIGH_IMPEDANCE;
    ADC_IT_read_base(hadc, channel, SAMPLE_HOLD_DISCHARGE_TIME(load_kohms));
}
void ADC_IT_high_impedance_read_mV(ADC_handle_t *hadc, ADC_channel_t channel, uint8_t load_kohms) {
    ADC_IT_last_state = ADC_IT_START_READ_HIGH_IMPEDANCE_VOLTAGE;
    ADC_IT_read_base(hadc, channel, SAMPLE_HOLD_DISCHARGE_TIME(load_kohms));
}

void ADC_IT_read(ADC_handle_t *hadc, ADC_channel_t channel) {
    ADC_IT_last_state = ADC_IT_START_READ;
    ADC_IT_read_base(hadc, channel, 0);
}
void ADC_IT_read_mV(ADC_handle_t *hadc, ADC_channel_t channel) {
    ADC_IT_last_state = ADC_IT_START_READ_VOLTAGE;
    ADC_IT_read_base(hadc, channel, 0);
}

void ADC_IT_read_VCC_mV(ADC_handle_t *hadc) {
    ADC_IT_last_state = ADC_IT_START_READ_VCC_VOLTAGE;
    ADC_set_reference(hadc, ADC_AVCC);
    ADC_IT_read_base(hadc, CH_VBG, 10);
}

ISR(ADC_vect) {
    if (adc_handle.state == ADC_BUSY) {
        adc_handle.state = ADC_EOC;
    }

    switch (ADC_IT_last_state) {
    case ADC_IT_START_READ:
        ADC_EOC_callback(&adc_handle, ADC_get_value(adc_handle.config.bits));
        break;
    case ADC_IT_START_READ_VOLTAGE:
        ADC_EOC_callback(&adc_handle, ADC_get_voltage_mV(&adc_handle));
        break;
    case ADC_IT_START_READ_HIGH_IMPEDANCE:
        ADC_EOC_callback(&adc_handle, ADC_get_value(adc_handle.config.bits));
        break;
    case ADC_IT_START_READ_HIGH_IMPEDANCE_VOLTAGE:
        ADC_EOC_callback(&adc_handle, ADC_get_voltage_mV(&adc_handle));
        break;
    case ADC_IT_START_READ_VCC_VOLTAGE:
        ADC_EOC_callback(&adc_handle, ((uint32_t)vref_internal_mV * ADC_get_steps(&adc_handle) / ADC_get_value(adc_handle.config.bits)) + vref_drift_avcc_mV);
        ADC_set_reference(&adc_handle, adc_handle.last_reference);
        break;
    default:
        ADC_IT_last_state = ADC_IT_IDLE;
        break;
    }

    if (adc_handle.state == ADC_EOC) {
        adc_handle.state  = ADC_IDLE;
        ADC_IT_last_state = ADC_IT_IDLE;
    }
}