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

#include <stdint.h>

uint8_t ADC_internal_ref_is_calibrated(void);

void ADC_calibrate_internal_ref(void (*user_feedback_callback)(void));

uint16_t ADC_get_calibrated_internal_ref_mV(void);

#endif    // ADC_CAL_H