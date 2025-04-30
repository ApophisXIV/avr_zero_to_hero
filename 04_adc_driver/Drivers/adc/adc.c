#include "adc.h"
#include "../uart/uart.h"
#include <avr/interrupt.h>
#include <stddef.h>

#define AVCC_N_SAMPLES         3    // If gets more, more noise are get
#define ADC_10_BITS            10
#define ADC_8_BITS             8
#define ADC_DIGITAL_INPUTS_MSK 0x3F

/* ------------------------ Calibration coefficients ------------------------ */
#ifdef USE_ADC_EEPROM_CALIBRATED_INT_REF
#include "adc_cal.h"
static uint16_t INTERNAL_VREF_mV = 1100;
#define CAL_COEFF 1
// #define CAL_COEFF 3.263f / 3.134f
#else
#define INTERNAL_VREF_mV 1100
#endif
/* -------------------------------------------------------------------------- */

struct adc_handle {
    bool is_avaliable;
    float vref;
    ADC_init_t config;
    ADC_state_t state;
};

static ADC_handle_t adc_handle = {
    .is_avaliable     = true,
    .vref             = 0.0f,
    .config.reference = ADC_INTERNAL_1_1,
    .config.bits      = ADC_10_BITS,
};

static ADC_handle_t *ADC_register_handle(void) {
    if (adc_handle.is_avaliable) {
        adc_handle.is_avaliable = false;
        return &adc_handle;
    }
    return NULL;
}

inline void delay_asm(void) {
    for (uint16_t i = 0; i < 600; i++) {
        __asm__ __volatile__("nop");
    }
}

static void ADC_set_avcc_vref(void) {

    uint32_t avcc = 0;
    for (uint8_t i = 0; i < AVCC_N_SAMPLES; i++) {
        avcc += ADC_read(&adc_handle, CH_VBG);
    }

    avcc /= AVCC_N_SAMPLES;

    adc_handle.vref = (((1 << adc_handle.config.bits)) * INTERNAL_VREF_mV) / avcc / 1000.0f;

    printf("AVCC medida: %.3f V\n", adc_handle.vref);
}

static void ADC_set_reference(ADC_reference_t ref) {

    ADMUX &= ~(1 << REFS1 | 1 << REFS0);    // Limpia los bits de referencia
    ADMUX |= ref;                           // Setea la referencia

    if (ref == ADC_AVCC) {
        ADC_set_avcc_vref();
    }
}

static void ADC_set_resolution(ADC_bitres_t bits) {
    if (bits == ADC_10B_RESOLUTION) {
        ADMUX &= ~(1 << ADLAR);
        adc_handle.config.bits = ADC_10_BITS;
    } else {
        ADMUX |= (1 << ADLAR);
        adc_handle.config.bits = ADC_8_BITS;
    }
}

/* -------------------------------- ADC Utils ------------------------------- */
// void ADC_set_ref_voltage_mV(ADC_handle_t *hadc, uint16_t vref_mV) {
//     hadc->vref = vref_mV;
// }

float ADC_get_ref_voltage(ADC_handle_t *hadc) {
    return hadc->vref;
}

/* ---------------------------- ADC Polling mode ---------------------------- */
#ifdef USE_ADC_EEPROM_CALIBRATED_INT_REF
ADC_handle_t *ADC_init(ADC_init_t *cfg, void (*feedback_callback)(void)) {
#else
ADC_handle_t *ADC_init(ADC_init_t *cfg) {
#endif

    ADC_handle_t *hadc = ADC_register_handle();
    if (hadc == NULL) return NULL;

    ADC_set_resolution(cfg->bits);

    adc_handle.config.low_power_channels = cfg->low_power_channels;
    adc_handle.config.preescaler         = cfg->preescaler;
    adc_handle.config.reference          = cfg->reference;
    adc_handle.config.trigger_source     = cfg->trigger_source;

    DIDR0 |= adc_handle.config.low_power_channels & ADC_DIGITAL_INPUTS_MSK;

    ADCSRA |= adc_handle.config.preescaler;

    ADCSRA |= 1 << ADEN;    // Habilita el ADC

#ifdef USE_ADC_EEPROM_CALIBRATED_INT_REF
    if (!ADC_internal_ref_is_calibrated()) {
        ADC_set_reference(ADC_INTERNAL_1_1);
        ADC_calibrate_internal_ref(feedback_callback);
    }

    INTERNAL_VREF_mV = ADC_get_calibrated_internal_ref_mV();
    adc_handle.vref  = INTERNAL_VREF_mV / 1000.0f;

    printf("vref_ee: %.3f\n", adc_handle.vref);
#else
    adc_handle.vref = INTERNAL_VREF_mV;
#endif

    ADC_set_reference(adc_handle.config.reference);

    if (adc_handle.config.trigger_source == ADC_NO_AUTO_TRIGGER) return hadc;

    ADCSRA |= 1 << ADATE;
    ADCSRB |= adc_handle.config.trigger_source;

    return hadc;
}

static void ADC_set_channel(ADC_channel_t ch) {
    ADMUX &= ~(1 << MUX3 | 1 << MUX2 | 1 << MUX1 | 1 << MUX0);
    ADMUX |= ch;
    delay_asm();    // NOTE: Empiricamente se observo la necesidad critica de tener
                    // un retardo para garantizar el settime entre cambios de canal
}

uint16_t ADC_read(ADC_handle_t *hadc, ADC_channel_t ch) {

    ADC_set_channel(ch);

    ADCSRA |= 1 << ADSC;             // Single convertion
    while (ADCSRA & (1 << ADSC));    // Espera a que termine la conversion

    uint16_t lsb = ADCL;    // Hoja de datos dice primero leer ADCL

    if (hadc->config.bits == ADC_8_BITS) return ADCH;
    return ADCH << 8 | lsb;
}

float ADC_read_vcc_voltage(ADC_handle_t *hadc) {
    ADC_set_reference(ADC_INTERNAL_1_1);
    uint16_t value = ADC_read(hadc, CH_VBG);
    ADC_set_reference(hadc->config.reference);
    return ((INTERNAL_VREF_mV / 1000.0f) * (1 << hadc->config.bits)) / value;
}

float ADC_read_voltage(ADC_handle_t *hadc, ADC_channel_t ch) {
    return hadc->vref * ADC_read(hadc, ch) / ((1 << hadc->config.bits) - 1);
}

void ADC_read_voltages(ADC_handle_t *hadc, ADC_channel_t channels, float *res_array, uint8_t n_res) {
    for (uint8_t i = 0, j = 0; i != MAX_CHANNELS && j != n_res; i++) {
        if (channels & (1 << i)) {
            res_array[j++] = ADC_read_voltage(hadc, channels & (1 << i));
        }
    }
}

/* --------------------------- ADC Interrupt mode --------------------------- */
#ifdef USE_ADC_EEPROM_CALIBRATED_INT_REF
ADC_handle_t *ADC_init_IT(ADC_init_t *cfg, void (*feedback_callback)(void)) {
    ADC_handle_t *hadc = ADC_init(cfg, feedback_callback);
#else
ADC_handle_t *ADC_init_IT(ADC_init_t *cfg) {
    ADC_handle_t *hadc = ADC_init(cfg);
#endif

    if (hadc == NULL) return NULL;
    ADCSRA |= 1 << ADIE;
    return hadc;
}

void ADC_read_vcc_voltage_IT(ADC_handle_t *hadc) {
}

void ADC_read_IT(ADC_channel_t ch) {
}

void ADC_read_voltage_IT(ADC_channel_t ch) {
}

ISR(ADC_vect) {
    if (adc_handle.state == ADC_BUSY) {
        adc_handle.state = ADC_EOC;
        ADC_EOC_callback(&adc_handle);
    }
}