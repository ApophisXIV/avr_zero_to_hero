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
#include "../../board.h"
#include <avr/eeprom.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

EEMEM uint16_t eeprom_internal_ref_mv = 1100U;    // Evito usar floats
EEMEM uint8_t eeprom_is_calibrated    = 0;

uint8_t ADC_internal_ref_is_calibrated(void) {
    uint8_t is_calibrated;
    eeprom_read_block(&is_calibrated, &eeprom_is_calibrated, sizeof(uint8_t));
    return is_calibrated;
}

uint16_t ADC_get_calibrated_internal_ref_mV(void) {
    uint16_t stored_val;
    eeprom_read_block(&stored_val, &eeprom_internal_ref_mv, sizeof(uint16_t));
    return stored_val;
}

void ADC_calibrate_internal_ref(void (*user_feedback_callback)(void)) {
    char buffer[32];
    char *err_ptr;
    uint16_t ref;

    do {
        printf("Enter internal reference in [mV] (4 digits) (e.g. 1100 for 1.100 V): ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            continue;
        }
        ref = (uint16_t)strtol(buffer, &err_ptr, 10U);
    } while (err_ptr == buffer || (*err_ptr != '\n' && *err_ptr != '\0'));

    if (user_feedback_callback) user_feedback_callback();

    // TODO: Revisar efecto. Uso update para evitar escrituras inecesarias
    eeprom_update_block(&ref, &eeprom_internal_ref_mv, sizeof(uint16_t));

    uint8_t is_calibrated = 1;
    eeprom_update_block(&is_calibrated, &eeprom_is_calibrated, sizeof(uint8_t));

    printf("Calibration saved: %u mV\n", ref);
}