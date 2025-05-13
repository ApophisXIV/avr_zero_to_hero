/**
 * @file adc.h
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-24
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef ADC_H
#define ADC_H

#include "../../board.h"
#ifdef USE_ADC

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

struct adc_handle;
typedef struct adc_handle ADC_handle_t;

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
    CH0 = (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0),
    CH1 = (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0),
    CH2 = (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0),
    CH3 = (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (1 << MUX0),
    CH4 = (0 << MUX3) | (1 << MUX2) | (0 << MUX1) | (0 << MUX0),
    CH5 = (0 << MUX3) | (1 << MUX2) | (0 << MUX1) | (1 << MUX0),
    CH6 = (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0),
    CH7 = (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0),
} ADC_channel_t;

typedef enum {
    LP_CH0 = (1 << 0),
    LP_CH1 = (1 << 1),
    LP_CH2 = (1 << 2),
    LP_CH3 = (1 << 3),
    LP_CH4 = (1 << 4),
    LP_CH5 = (1 << 5),
} ADC_low_power_channel_t;

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
    ADC_10B_RESOLUTION = (0 << ADLAR),
    ADC_8B_RESOLUTION  = (1 << ADLAR),
} ADC_resolution_t;

typedef struct {
    ADC_resolution_t bits;
    ADC_reference_t reference;
    ADC_preescaler_t preescaler;
    ADC_low_power_channel_t low_power_channels;    // e.g: CH1|CH2|CH3|CH4|CH5|CH6
    ADC_trigger_t trigger_source;
} ADC_init_t;

typedef enum {
    ADC_STOPED,
    ADC_IDLE,
    ADC_BUSY,
    ADC_EOC,
    ADC_CALIBRATING,
} ADC_state_t;

ADC_handle_t *ADC_init(ADC_init_t *cfg);
ADC_handle_t *ADC_IT_init(ADC_init_t *cfg);

void ADC_set_calibration(ADC_handle_t *hadc, void *parameters);

void ADC_set_reference(ADC_handle_t *hadc, ADC_reference_t reference);
ADC_reference_t ADC_get_reference(ADC_handle_t *hadc);

uint16_t ADC_read(ADC_handle_t *hadc, ADC_channel_t channel);
uint16_t ADC_read_mV(ADC_handle_t *hadc, ADC_channel_t channel);

uint16_t ADC_high_impedance_read(ADC_handle_t *hadc, ADC_channel_t channel, uint8_t load_kohms);
uint16_t ADC_high_impedance_read_mV(ADC_handle_t *hadc, ADC_channel_t channel, uint8_t load_kohms);

void ADC_IT_read(ADC_handle_t *hadc, ADC_channel_t channel);
void ADC_IT_read_mV(ADC_handle_t *hadc, ADC_channel_t channel);

void ADC_IT_high_impedance_read(ADC_handle_t *hadc, ADC_channel_t channel, uint8_t load_kohms);
void ADC_IT_high_impedance_read_mV(ADC_handle_t *hadc, ADC_channel_t channel, uint8_t load_kohms);

/* ---------------------------------- Utils --------------------------------- */
uint16_t ADC_read_VCC_mV(ADC_handle_t *hadc);
void ADC_IT_read_VCC_mV(ADC_handle_t *hadc);

ADC_state_t ADC_get_state(ADC_handle_t *hadc);

extern void ADC_EOC_callback(ADC_handle_t *hadc, uint16_t value);

#endif
#endif