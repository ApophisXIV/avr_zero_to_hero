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
    TIM_CLK_INTERNAL_PRESCALER_DIV32,     // Available only for TMR2
    TIM_CLK_INTERNAL_PRESCALER_DIV128,    // Avaliable only for TMR2
    TIM_CLK_INTERNAL_PRESCALER_DIV256,
    TIM_CLK_INTERNAL_PRESCALER_DIV1024,
    TIM_CLK_EXTERNAL_FALLING_EDGE,
    TIM_CLK_EXTERNAL_RISING_EDGE,
} TIM_clk_source_t;

/* Mode config ------------------------------- */
typedef enum {
    NORMAL_TIMER_AUTORELOAD,
    NORMAL_ONE_SHOT,
    CTC_CHANNEL_A_NO_OUTPUT,
    CTC_CHANNEL_B_NO_OUTPUT,
    CTC_CHANNEL_A_PIN_TOGGLE,
    CTC_CHANNEL_B_PIN_TOGGLE,
    CTC_CHANNEL_A_PIN_CLEAR,
    CTC_CHANNEL_B_PIN_CLEAR,
    CTC_CHANNEL_A_PIN_SET,
    CTC_CHANNEL_B_PIN_SET,
} TIM_mode_t;

/* Config datatype ---------------------------- */
typedef struct {
    TIM_timer_t timer;
    TIM_clk_source_t clk_source;
    uint16_t preset_value;
    TIM_mode_t mode;
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
TIM_handle_t *TIM_base_init(TIM_init_t *cfg);

void TIM_base_start(TIM_handle_t *htim);
void TIM_base_stop(TIM_handle_t *htim);

void TIM_base_start_IT(TIM_handle_t *htim);
void TIM_base_stop_IT(TIM_handle_t *htim);

/* CTC functions ----------------------------- */
TIM_handle_t *TIM_CTC_init(TIM_init_t *cfg);

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

#ifdef DEBUG_TIM
void TIM_debug_handle(TIM_handle_t *htim);
#endif

#endif    // TIMER_H