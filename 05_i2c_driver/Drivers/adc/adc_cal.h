/**
 * @file adc_cal.h
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

#ifndef ADC_CAL_H
#define ADC_CAL_H

#include "adc.h"
#ifdef USE_ADC_CALIBRATION

#include <stdbool.h>
#include <stdint.h>

struct ADC_calibration;
typedef struct ADC_calibration ADC_calibration_t;

bool ADC_is_calibrated(void);

bool ADC_internal_ref_is_calibrated(void);
uint16_t ADC_get_calibrated_internal_ref_mV(ADC_calibration_t *calibration);

bool ADC_avcc_ref_is_calibrated(void);
int16_t ADC_get_calibrated_avcc_ref_drift_mV(ADC_calibration_t *calibration);
uint16_t ADC_get_calibrated_avcc_ref_mV(ADC_calibration_t *calibration);

void ADC_calibrate(ADC_handle_t *hadc);
ADC_calibration_t *ADC_get_calibration(void);

#endif
#endif    // ADC_CAL_H