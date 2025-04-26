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

/* Base config ------------------------------ */
typedef enum {
    TIM_0,
    TIM_1,
    TIM_2,
} TIM_timer_t;

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

/* CTC config ------------------------------- */
typedef enum {
    CTC_NO_OUTPUT  = 0,
    CTC_PIN_TOGGLE = 1,
    CTC_PIN_CLEAR  = 2,
    CTC_PIN_SET    = 3,
} TIM_CTC_output_t;

typedef enum {
    CTC_CHANNEL_A,
    CTC_CHANNEL_B,
} TIM_CTC_channel_t;

typedef struct {
    TIM_CTC_output_t output_behavior;
    TIM_CTC_channel_t channel;
} TIM_CTC_mode_t;

/* Normal config ----------------------------- */
typedef enum {
    NORMAL_AUTO_RELOAD,
    NORMAL_ONE_SHOT,
} TIM_Normal_mode_t;

/* Config datatype ---------------------------- */
typedef struct {
    TIM_timer_t timer;
    TIM_clk_source_t clk_source;
    uint16_t preset_value;
    union {
        TIM_Normal_mode_t normal;
        TIM_CTC_mode_t ctc;
    } mode;
} TIM_init_t;

typedef enum {
    TIM_STATE_READY,
    TIM_STATE_BUSY,
    TIM_STATE_TIMEOUT,
    TIM_STATE_MATCH,
} TIM_state_t;

struct TIM_handle;
typedef struct TIM_handle TIM_handle_t;

TIM_state_t TIM_get_state(TIM_handle_t *htim);

/* Base functions ----------------------------- */
void TIM_base_init(TIM_handle_t *htim, TIM_init_t *cfg);

void TIM_base_start(TIM_handle_t *htim);
void TIM_base_stop(TIM_handle_t *htim);

void TIM_base_start_IT(TIM_handle_t *htim);
void TIM_base_stop_IT(TIM_handle_t *htim);

/* CTC functins ------------------------------ */
void TIM_CTC_init(TIM_handle_t *htim, TIM_init_t *cfg);

void TIM_CTC_A_start(TIM_handle_t *htim);
void TIM_CTC_A_stop(TIM_handle_t *htim);

void TIM_CTC_B_start(TIM_handle_t *htim);
void TIM_CTC_B_stop(TIM_handle_t *htim);

void TIM_CTC_A_start_IT(TIM_handle_t *htim);
void TIM_CTC_A_stop_IT(TIM_handle_t *htim);

void TIM_CTC_B_start_IT(TIM_handle_t *htim);
void TIM_CTC_B_stop_IT(TIM_handle_t *htim);

/* Callbacks ------------------------------- */
extern void TIM_period_elapsed_callback(TIM_handle_t *htim);
extern void TIM_CTC_callback(TIM_handle_t *htim);

#endif    // TIMER_H