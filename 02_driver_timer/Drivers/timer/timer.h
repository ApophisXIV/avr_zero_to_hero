/**
 * @file timer.h
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-21
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef TIMER_H
#define TIMER_H

#include "../../board.h"

#include <avr/io.h>
#include <stdbool.h>

// typedef struct {
//     union {
//         tmr0_prescaler_t tmr0_prescaler;
//         tmr1_prescaler_t tmr1_prescaler;
//         tmr2_prescaler_t tmr2_prescaler;
//     } prescaler;
//     union {
//         uint8_t tmr0_preset;
//         uint16_t tmr1_preset;
//         uint8_t tmr2_preset;
//     } preset;
// } timer_cfg_t;

// typedef struct {
//     tmr0_prescaler_t prescaler;
// } timer_cfg_t;

typedef enum {
    TIM_CLK_INTERNAL_PRESCALER_DIV1,
    TIM_CLK_INTERNAL_PRESCALER_DIV8,
    TIM_CLK_INTERNAL_PRESCALER_DIV64,
    TIM_CLK_INTERNAL_PRESCALER_DIV32,    // Available only for TMR2
    TIM_CLK_INTERNAL_PRESCALER_DIV256,
    TIM_CLK_INTERNAL_PRESCALER_DIV1024,
    TIM_CLK_EXTERNAL_FALLING_EDGE,
    TIM_CLK_EXTERNAL_RISING_EDGE,
} TIM_clk_source_t;

typedef enum {
    TIM_0,
    TIM_1,
    TIM_2,
} TIM_timer_t;

typedef enum {
    TIM_CHANNEL_A,
    TIM_CHANNEL_B,
} TIM_channel_t;

typedef enum {
    TIM_STATE_READY,
    TIM_STATE_BUSY,
    TIM_STATE_TIMEOUT,
} TIM_state_t;

typedef enum {
    TIM_MODE_NORMAL,
    TIM_MODE_CTC,
    TIM_MODE_FAST_PWM,
    TIM_MODE_PHASE_CORRECT_PWM,
} TIM_mode_t;

typedef struct {
    TIM_timer_t timer;
    TIM_state_t state;
    TIM_clk_source_t clk_source;
    uint16_t preset_value;
} TIM_handle_t;

void TIM_base_init(TIM_handle_t *htim);

void TIM_base_start(TIM_handle_t *htim);
void TIM_base_stop(TIM_handle_t *htim);

TIM_state_t TIM_get_state(TIM_handle_t *htim);

void TIM_base_start_IT(TIM_handle_t *htim);
void TIM_base_stop_IT(TIM_handle_t *htim);

void TIM_compare_match_A_start(TIM_handle_t *htim);
void TIM_compare_match_A_stop(TIM_handle_t *htim);

void TIM_compare_match_A_start_IT(TIM_handle_t *htim);
void TIM_compare_match_A_stop_IT(TIM_handle_t *htim);

void TIM_compare_match_B_start(TIM_handle_t *htim);
void TIM_compare_match_B_stop(TIM_handle_t *htim);

void TIM_compare_match_B_start_IT(TIM_handle_t *htim);
void TIM_compare_match_B_stop_IT(TIM_handle_t *htim);

// Callbacks
extern void TIM_period_elapsed_callback(TIM_handle_t *htim);
extern void TIM_compare_match_A_callback(TIM_handle_t *htim);
extern void TIM_compare_match_B_callback(TIM_handle_t *htim);

#endif    // TIMER_H