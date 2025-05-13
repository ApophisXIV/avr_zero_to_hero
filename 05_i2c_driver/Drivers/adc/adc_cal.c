/**
 * @file adc_cal.c
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-28
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */
#include "adc_cal.h"
#ifdef USE_ADC_CALIBRATION

#include <avr/eeprom.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ADC_AVCC_N_SAMPLES 100

EEMEM uint16_t eeprom_internal_ref_mv        = 0;
EEMEM bool eeprom_internal_ref_is_calibrated = 0;

EEMEM uint16_t eeprom_avcc_ref_mv        = 0;
EEMEM int16_t eeprom_avcc_ref_drift_mv   = 0;
EEMEM bool eeprom_avcc_ref_is_calibrated = 0;

struct ADC_calibration {
    uint16_t int_ref;
    uint16_t avcc_ref;
    int16_t avcc_drift;
};

bool ADC_is_calibrated(void) {
    return ADC_internal_ref_is_calibrated() && ADC_avcc_ref_is_calibrated();
}

ADC_calibration_t *ADC_get_calibration(void) {
    static ADC_calibration_t calibration;
    eeprom_busy_wait();
    eeprom_read_block(&calibration.int_ref, &eeprom_internal_ref_mv, sizeof(uint16_t));
    eeprom_busy_wait();
    eeprom_read_block(&calibration.avcc_ref, &eeprom_avcc_ref_mv, sizeof(uint16_t));
    eeprom_busy_wait();
    eeprom_read_block(&calibration.avcc_drift, &eeprom_avcc_ref_drift_mv, sizeof(int16_t));
    return &calibration;
}

bool ADC_internal_ref_is_calibrated(void) {
    bool is_calibrated;
    eeprom_busy_wait();
    eeprom_read_block(&is_calibrated, &eeprom_internal_ref_is_calibrated, sizeof(bool));
    return is_calibrated;
}

uint16_t ADC_get_calibrated_internal_ref_mV(ADC_calibration_t *calibration) {
    return calibration->int_ref;
}

bool ADC_avcc_ref_is_calibrated(void) {
    bool is_calibrated;
    eeprom_busy_wait();
    eeprom_read_block(&is_calibrated, &eeprom_avcc_ref_is_calibrated, sizeof(bool));
    return is_calibrated;
}

int16_t ADC_get_calibrated_avcc_ref_drift_mV(ADC_calibration_t *calibration) {
    return calibration->avcc_drift;
}

uint16_t ADC_get_calibrated_avcc_ref_mV(ADC_calibration_t *calibration) {
    return calibration->avcc_ref;
}

typedef struct {
    const char *message;
    uint8_t n_digits;
    uint16_t low_limit;
    uint16_t high_limit;
} ADC_CAL_parameters_t;

static void strip_newline(char *buffer) {
    while (*buffer != '\0') {
        if (*buffer == '\n' || *buffer == '\r') {
            *buffer = '\0';
            break;
        }
        buffer++;
    }
}

static uint16_t ADC_base_calibration(ADC_CAL_parameters_t *calibration) {
    char buffer[8];
    char *err_ptr;
    uint16_t value;
    uint8_t calibration_completed = 0;

    while (!calibration_completed) {

        printf("%s", calibration->message);
        fflush(stdout);

        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Input error. Please try again.\n");
            continue;
        }

        strip_newline(buffer);
        value = (uint16_t)strtol(buffer, &err_ptr, 10U);

        if (err_ptr == buffer ||                                                    // No se leyo nada
            err_ptr - buffer != calibration->n_digits ||                            // No tiene n digitos
            value < calibration->low_limit || value > calibration->high_limit) {    // Fuera de rango
            printf("Invalid value. Please enter a number between %u and %u mV\n",
                   calibration->low_limit, calibration->high_limit);
            continue;
        } else {
            calibration_completed = 1;
        }
    }

    printf("Calibration saved: %u mV\n", value);
    return value;
}

void ADC_calibrate(ADC_handle_t *hadc) {
    ADC_CAL_parameters_t cal_parameters = {0};

    ADC_calibration_t values = {0};
    ADC_reference_t last_ref = ADC_get_reference(hadc);

    eeprom_busy_wait();
    eeprom_update_byte(&eeprom_internal_ref_is_calibrated, 0);

    eeprom_busy_wait();
    eeprom_update_byte(&eeprom_avcc_ref_is_calibrated, 0);

    /* --------------------- Internal reference calibration --------------------- */
    ADC_set_reference(hadc, ADC_INTERNAL_1_1);

    cal_parameters.message    = "Internal REF calibration in progress...\n"
                                "Please connect a multimeter to the AREF pin\n"
                                "Enter internal reference in [mV] (4 digits) (e.g. 1100 for 1.100 V): ";
    cal_parameters.n_digits   = 4;
    cal_parameters.low_limit  = 1000;
    cal_parameters.high_limit = 1200;

    values.int_ref = ADC_base_calibration(&cal_parameters);
    eeprom_busy_wait();
    eeprom_update_word(&eeprom_internal_ref_mv, values.int_ref);

    eeprom_busy_wait();
    eeprom_update_byte(&eeprom_internal_ref_is_calibrated, 1);

    ADC_set_calibration(hadc, &values);

    /* ----------------------- AVCC reference calibration ----------------------- */
    ADC_set_reference(hadc, ADC_AVCC);

    cal_parameters.message    = "AVCC REF calibration in progress...\n"
                                "Please connect a multimeter to the AREF pin\n"
                                "An internal VCC measurement is performed by the device\n"
                                "This value is used to calculate the drift from the AVCC reference\n"
                                "Enter AVCC reference in [mV] (4 digits) (e.g. 5000 for 5.000 V): ";
    cal_parameters.n_digits   = 4;
    cal_parameters.low_limit  = 2700;
    cal_parameters.high_limit = 5500;

    values.avcc_ref = ADC_base_calibration(&cal_parameters);
    uint32_t vcc    = 0;
    for (uint8_t i = 0; i < ADC_AVCC_N_SAMPLES; i++) {
        vcc += ADC_read_VCC_mV(hadc);
    }
    vcc /= ADC_AVCC_N_SAMPLES;
    printf("VCC measured: %u mV\n", vcc);
    values.avcc_drift = (int16_t)((int16_t)values.avcc_ref - (int16_t)vcc);
    printf("AVCC drift calibration saved: %d mV\n", values.avcc_drift);

    eeprom_busy_wait();
    eeprom_update_word(&eeprom_avcc_ref_mv, values.avcc_ref);

    eeprom_busy_wait();
    eeprom_update_word(&eeprom_avcc_ref_drift_mv, values.avcc_drift);

    eeprom_busy_wait();
    eeprom_update_byte(&eeprom_avcc_ref_is_calibrated, 1);

    ADC_set_calibration(hadc, &values);

    /* -------------------------------------------------------------------------- */
    ADC_set_reference(hadc, last_ref);
    printf("Calibration completed\n");
}

#endif