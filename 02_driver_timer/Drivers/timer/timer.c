/**
 * @file timer.c
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-26
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#include "timer.h"

#include "../../board.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#define TIM_8B_MAX_VALUE  255
#define TIM_16B_MAX_VALUE 65535

struct TIM_handle {
    TIM_init_t config;
    TIM_state_t state;
    bool is_available;
};

#define TOVx_SHIFT  0
#define OCFxA_SHIFT 1
#define OCFxB_SHIFT 2

#define TOIEx_SHIFT  0
#define OCIExA_SHIFT 1
#define OCIExB_SHIFT 2

#define COMxA1_SHIFT 7
#define COMxA0_SHIFT 6

#define COMxB1_SHIFT 5
#define COMxB0_SHIFT 4

#define WGMx2_SHIFT 3
#define WGMx1_SHIFT 1
#define WGMx0_SHIFT 0

#define NO_CLK_SOURCE_MSK 7

static TIM_handle_t timer_handles[3] = {
    [TIM_0] = {.is_available = true},
    [TIM_1] = {.is_available = true},
    [TIM_2] = {.is_available = true},
};

void *TIM_get_TCNTx(TIM_timer_t timer) {
    switch (timer) {
    case TIM_0: return (void *)&TCNT0;
    case TIM_1: return (void *)&TCNT1;
    case TIM_2: return (void *)&TCNT2;
    default: return NULL;    // Invalid timer
    }
}
void *TIM_get_OCRxA(TIM_timer_t timer) {
    switch (timer) {
    case TIM_0: return (void *)&OCR0A;
    case TIM_1: return (void *)&OCR1A;
    case TIM_2: return (void *)&OCR2A;
    default: return NULL;    // Invalid timer
    }
}
void *TIM_get_OCRxB(TIM_timer_t timer) {
    switch (timer) {
    case TIM_0: return (void *)&OCR0B;
    case TIM_1: return (void *)&OCR1B;
    case TIM_2: return (void *)&OCR2B;
    default: return NULL;    // Invalid timer
    }
}

volatile uint8_t *TIM_get_TCCRxA(TIM_timer_t timer) {
    switch (timer) {
    case TIM_0: return &TCCR0A;
    case TIM_1: return &TCCR1A;
    case TIM_2: return &TCCR2A;
    default: return NULL;    // Invalid timer
    }
}
volatile uint8_t *TIM_get_TCCRxB(TIM_timer_t timer) {
    switch (timer) {
    case TIM_0: return &TCCR0B;
    case TIM_1: return &TCCR1B;
    case TIM_2: return &TCCR2B;
    default: return NULL;    // Invalid timer
    }
}
volatile uint8_t *TIM_get_TIFRx(TIM_timer_t timer) {
    switch (timer) {
    case TIM_0: return &TIFR0;
    case TIM_1: return &TIFR1;
    case TIM_2: return &TIFR2;
    default: return NULL;    // Invalid timer
    }
}
volatile uint8_t *TIM_get_TIMSKx(TIM_timer_t timer) {
    switch (timer) {
    case TIM_0: return &TIMSK0;
    case TIM_1: return &TIMSK1;
    case TIM_2: return &TIMSK2;
    default: return NULL;    // Invalid timer
    }
}

static uint8_t TIM_get_clk_source_bits(TIM_handle_t *htim) {
    if (htim->config.timer == TIM_2) {
        switch (htim->config.clk_source) {
        case TIM_CLK_INTERNAL_PRESCALER_DIV1: return 0x01;
        case TIM_CLK_INTERNAL_PRESCALER_DIV8: return 0x02;
        case TIM_CLK_INTERNAL_PRESCALER_DIV32: return 0x03;    // Available only for TMR2
        case TIM_CLK_INTERNAL_PRESCALER_DIV64: return 0x04;
        case TIM_CLK_INTERNAL_PRESCALER_DIV128: return 0x05;    // Available only for TMR2
        case TIM_CLK_INTERNAL_PRESCALER_DIV256: return 0x06;
        case TIM_CLK_INTERNAL_PRESCALER_DIV1024: return 0x07;
        case TIM_CLK_EXTERNAL_FALLING_EDGE: return 0x00;
        case TIM_CLK_EXTERNAL_RISING_EDGE: return 0x00;
        default: return 0x00;    // Invalid clock source
        }
    } else {
        switch (htim->config.clk_source) {
        case TIM_CLK_INTERNAL_PRESCALER_DIV1: return 0x01;
        case TIM_CLK_INTERNAL_PRESCALER_DIV8: return 0x02;
        case TIM_CLK_INTERNAL_PRESCALER_DIV64: return 0x03;
        case TIM_CLK_INTERNAL_PRESCALER_DIV256: return 0x04;
        case TIM_CLK_INTERNAL_PRESCALER_DIV1024: return 0x05;
        case TIM_CLK_EXTERNAL_FALLING_EDGE: return 0x06;
        case TIM_CLK_EXTERNAL_RISING_EDGE: return 0x07;
        default: return 0x00;    // Invalid clock source
        }
    }
}

static TIM_handle_t *TIM_register_handle(TIM_timer_t timer) {
    if (!timer_handles[timer].is_available) return NULL;
    timer_handles[timer].is_available = false;
    return &timer_handles[timer];
}

static void TIM_set_clk_source(TIM_handle_t *htim) {
    *TIM_get_TCCRxB(htim->config.timer) |= TIM_get_clk_source_bits(htim);
    htim->state = TIM_STATE_BUSY;
}

static void TIM_clear_clk_source(TIM_handle_t *htim) {
    *TIM_get_TCCRxB(htim->config.timer) &= ~NO_CLK_SOURCE_MSK;
    htim->state = TIM_STATE_READY;
}

inline TIM_state_t TIM_get_state(TIM_handle_t *htim) {

    volatile uint8_t *flag_reg = TIM_get_TIFRx(htim->config.timer);
    TIM_mode_t mode            = htim->config.mode;

    if ((mode == NORMAL_ONE_SHOT || mode == NORMAL_TIMER_AUTORELOAD) && (*flag_reg & (1 << TOVx_SHIFT))) {
        if (htim->config.mode == NORMAL_ONE_SHOT) {
            TIM_clear_clk_source(htim);
        }
        *flag_reg   = (1 << TOVx_SHIFT);
        htim->state = TIM_STATE_TIMEOUT;
    } else if (mode == CTC_CHANNEL_A_NO_OUTPUT && (*flag_reg & (1 << OCFxA_SHIFT))) {
        *flag_reg   = (1 << OCFxA_SHIFT);
        htim->state = TIM_STATE_MATCH;
    } else if (mode == CTC_CHANNEL_B_NO_OUTPUT && (*flag_reg & (1 << OCFxB_SHIFT))) {
        *flag_reg   = (1 << OCFxB_SHIFT);
        htim->state = TIM_STATE_MATCH;
    } else {
        htim->state = TIM_STATE_BUSY;
    }

    return htim->state;
}

/* ------------------------------- Base timer ------------------------------- */
TIM_handle_t *TIM_base_init(TIM_init_t *cfg) {

    TIM_handle_t *htim = TIM_register_handle(cfg->timer);
    if (htim == NULL) return NULL;

    htim->config.timer        = cfg->timer;
    htim->config.clk_source   = cfg->clk_source;
    htim->config.mode         = cfg->mode;
    htim->config.preset_value = cfg->preset_value;
    htim->state               = TIM_STATE_READY;

    *TIM_get_TCCRxA(cfg->timer) = 0;
    *TIM_get_TCCRxB(cfg->timer) = 0;

    return htim;
}

void TIM_base_start(TIM_handle_t *htim) {
    if (htim->config.timer == TIM_1) {
        *(uint16_t *)TIM_get_TCNTx(htim->config.timer) = htim->config.preset_value;
    } else {
        *(uint8_t *)TIM_get_TCNTx(htim->config.timer) = htim->config.preset_value;
    }
    TIM_set_clk_source(htim);
}

void TIM_base_stop(TIM_handle_t *htim) {
    TIM_clear_clk_source(htim);
    *TIM_get_TIFRx(htim->config.timer) |= (1 << TOVx_SHIFT);    // Clear by writing 1 (p.88 - 14.9.7 TIFR0 (apply to all timers))
}

void TIM_base_start_IT(TIM_handle_t *htim) {
    *TIM_get_TIMSKx(htim->config.timer) |= (1 << TOIEx_SHIFT);
    TIM_base_start(htim);
}

void TIM_base_stop_IT(TIM_handle_t *htim) {
    *TIM_get_TIMSKx(htim->config.timer) &= ~(1 << TOIEx_SHIFT);
    TIM_base_stop(htim);
}
/* -------------------------------------------------------------------------- */

/* -------------------------------- CTC timer ------------------------------- */
// Necesito definir Compare Output Mode (COMxA1, COMxA0, COMxB1, COMxB0) en TCCRxA
// Esto describe como se comportara el pin de salida asociado al canal A o B
// Luego defino el Waveform Generation Mode (WGMx) en TCCRxA y TCCRxB
// WGMx: 000 = Normal mode
// WGMx: 001 = PWM, Phase Correct (limite superior = 0xFF)
// WGMx: 010 = CTC mode (limite superior = OCRxA)
// WGMx: 011 = Fast PWM (limite superior = 0xFF)
// WGMx: 101 = PWM, Phase Correct (limite superior = OCRxA)
// WGMx: 111 = Fast PWM (limite superior = OCRxA)
// TODO: Agregar configuracion de pin correspondiente mediante GPIO driver
// Mirar alternate function de la libreria GPIO
TIM_handle_t *TIM_CTC_init(TIM_init_t *cfg) {
    TIM_handle_t *htim = TIM_base_init(cfg);
    if (htim == NULL) return NULL;
    if (cfg->timer == TIM_1) {
        *TIM_get_TCCRxB(cfg->timer) |= (1 << WGM12);    // CTC mode TIM_1
    } else {
        *TIM_get_TCCRxA(cfg->timer) |= (1 << WGMx1_SHIFT);    // CTC mode TIM_0 and TIM_2
    }
    return htim;
}

static void TIM_set_compare_value(TIM_handle_t *htim) {
    if (
        htim->config.mode == CTC_CHANNEL_A_NO_OUTPUT ||
        htim->config.mode == CTC_CHANNEL_A_PIN_TOGGLE ||
        htim->config.mode == CTC_CHANNEL_A_PIN_CLEAR ||
        htim->config.mode == CTC_CHANNEL_A_PIN_SET) {

        if (htim->config.timer == TIM_1) {
            *(uint16_t *)TIM_get_OCRxA(htim->config.timer) = htim->config.preset_value;
            *(uint16_t *)TIM_get_TCNTx(htim->config.timer) = 0;
        } else {
            *(uint8_t *)TIM_get_OCRxA(htim->config.timer) = htim->config.preset_value;
            *(uint8_t *)TIM_get_TCNTx(htim->config.timer) = 0;
        }
    } else {
        if (htim->config.timer == TIM_1) {
            *(uint16_t *)TIM_get_OCRxB(htim->config.timer) = htim->config.preset_value;
            *(uint16_t *)TIM_get_TCNTx(htim->config.timer) = 0;

        } else {
            *(uint8_t *)TIM_get_OCRxB(htim->config.timer) = htim->config.preset_value;
            *(uint8_t *)TIM_get_TCNTx(htim->config.timer) = 0;
        }
    }
}

void TIM_CTC_A_start(TIM_handle_t *htim) {
    TIM_set_compare_value(htim);
    TIM_set_clk_source(htim);
}
void TIM_CTC_A_stop(TIM_handle_t *htim) {
    TIM_clear_clk_source(htim);
    *TIM_get_TIFRx(htim->config.timer) |= 1 << OCFxA_SHIFT;    // Clear by writing 1 (p.88 - 14.9.7 TIFR0 (apply to all timers))
}

void TIM_CTC_B_start(TIM_handle_t *htim) {
    TIM_set_compare_value(htim);
    TIM_set_clk_source(htim);
}

void TIM_CTC_B_stop(TIM_handle_t *htim) {
    TIM_clear_clk_source(htim);
    *TIM_get_TIFRx(htim->config.timer) |= 1 << OCFxB_SHIFT;    // Clear by writing 1 (p.88 - 14.9.7 TIFR0 (apply to all timers))
}

void TIM_CTC_A_start_IT(TIM_handle_t *htim) {
    *TIM_get_TIMSKx(htim->config.timer) |= (1 << OCIExA_SHIFT);
    TIM_CTC_A_start(htim);
}
void TIM_CTC_A_stop_IT(TIM_handle_t *htim) {
    *TIM_get_TIMSKx(htim->config.timer) &= ~(1 << OCIExA_SHIFT);
    TIM_CTC_A_stop(htim);
}
void TIM_CTC_B_start_IT(TIM_handle_t *htim) {
    *TIM_get_TIMSKx(htim->config.timer) |= (1 << OCIExB_SHIFT);
    TIM_CTC_B_start(htim);
}
void TIM_CTC_B_stop_IT(TIM_handle_t *htim) {
    *TIM_get_TIMSKx(htim->config.timer) &= ~(1 << OCIExB_SHIFT);
    TIM_CTC_B_stop(htim);
}
/* -------------------------------------------------------------------------- */

/* -------------------------------- Callbacks ------------------------------- */
// TODO: Agregar a cada COMPARE ISR el caso de output mode
ISR(TIMER0_OVF_vect) {
    if (timer_handles[TIM_0].state == TIM_STATE_BUSY) {

        timer_handles[TIM_0].state = TIM_STATE_TIMEOUT;
        TIM_period_elapsed_callback(&timer_handles[TIM_0]);

        if (timer_handles[TIM_0].config.mode == NORMAL_ONE_SHOT) {
            TIM_base_stop_IT(&timer_handles[TIM_0]);
        } else {
            TIM_base_start(&timer_handles[TIM_0]);
        }
    }
}
ISR(TIMER0_COMPA_vect) {
    if (timer_handles[TIM_0].state == TIM_STATE_BUSY) {
        timer_handles[TIM_0].state = TIM_STATE_MATCH;
        TIM_CTC_callback(&timer_handles[TIM_0]);
        timer_handles[TIM_0].state = TIM_STATE_BUSY;
    }
}
ISR(TIMER0_COMPB_vect) {
    if (timer_handles[TIM_0].state == TIM_STATE_BUSY) {
        timer_handles[TIM_0].state = TIM_STATE_MATCH;
        TIM_CTC_callback(&timer_handles[TIM_0]);
        timer_handles[TIM_0].state = TIM_STATE_BUSY;
    }
}

ISR(TIMER1_OVF_vect) {
    if (timer_handles[TIM_1].state == TIM_STATE_BUSY) {

        timer_handles[TIM_1].state = TIM_STATE_TIMEOUT;
        TIM_period_elapsed_callback(&timer_handles[TIM_1]);

        if (timer_handles[TIM_1].config.mode == NORMAL_ONE_SHOT) {
            TIM_base_stop_IT(&timer_handles[TIM_1]);
        } else {
            TIM_base_start(&timer_handles[TIM_1]);
        }
    }
}
ISR(TIMER1_COMPA_vect) {
    if (timer_handles[TIM_1].state == TIM_STATE_BUSY) {
        timer_handles[TIM_1].state = TIM_STATE_MATCH;
        TIM_CTC_callback(&timer_handles[TIM_1]);
        timer_handles[TIM_1].state = TIM_STATE_BUSY;
    }
}
ISR(TIMER1_COMPB_vect) {
    if (timer_handles[TIM_1].state == TIM_STATE_BUSY) {
        timer_handles[TIM_1].state = TIM_STATE_MATCH;
        TIM_CTC_callback(&timer_handles[TIM_1]);
        timer_handles[TIM_1].state = TIM_STATE_BUSY;
    }
}

ISR(TIMER2_OVF_vect) {
    if (timer_handles[TIM_2].state == TIM_STATE_BUSY) {

        timer_handles[TIM_2].state = TIM_STATE_TIMEOUT;
        TIM_period_elapsed_callback(&timer_handles[TIM_2]);

        if (timer_handles[TIM_2].config.mode == NORMAL_ONE_SHOT) {
            TIM_base_stop_IT(&timer_handles[TIM_2]);
        } else {
            TIM_base_start(&timer_handles[TIM_2]);
        }
    }
}
ISR(TIMER2_COMPA_vect) {
    if (timer_handles[TIM_2].state == TIM_STATE_BUSY) {
        timer_handles[TIM_2].state = TIM_STATE_MATCH;
        TIM_CTC_callback(&timer_handles[TIM_2]);
        timer_handles[TIM_2].state = TIM_STATE_BUSY;
    }
}
ISR(TIMER2_COMPB_vect) {
    if (timer_handles[TIM_2].state == TIM_STATE_BUSY) {
        timer_handles[TIM_2].state = TIM_STATE_MATCH;
        TIM_CTC_callback(&timer_handles[TIM_2]);
        timer_handles[TIM_2].state = TIM_STATE_BUSY;
    }
}