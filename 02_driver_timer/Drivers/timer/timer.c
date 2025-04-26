#include "timer.h"

#include "../../board.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#define TIM_8B_MAX_VALUE  255
#define TIM_16B_MAX_VALUE 65535

struct TIM_handle {
    TIM_init_t config;
    TIM_state_t state;
};

typedef struct {
    volatile uint8_t *TCCRxA;
    volatile uint8_t *TCCRxB;
    volatile uint8_t *TIFRx;
    volatile uint8_t *TIMSKx;
    union {
        volatile uint8_t *ptr8;
        volatile uint16_t *ptr16;
    } TCNTx;
    union {
        volatile uint8_t *ptr8;
        volatile uint16_t *ptr16;
    } OCRxA;
    union {
        volatile uint8_t *ptr8;
        volatile uint16_t *ptr16;
    } OCRxB;
} TIM_config_regs_t;

// clang-format off
static TIM_config_regs_t TIM_regs[] = {
    [TIM_0] = { 
        .TCCRxA = &TCCR0A,  .TCCRxB = &TCCR0B,
        .TIFRx  = &TIFR0,   .TIMSKx = &TIMSK0,
        .OCRxA.ptr8  = &OCR0A,   .OCRxB.ptr8  = &OCR0B,
        .TCNTx.ptr8  = &TCNT0
    },
    [TIM_1] = { 
        .TCCRxA = &TCCR1A,  .TCCRxB = &TCCR1B,
        .TIFRx  = &TIFR1,   .TIMSKx = &TIMSK1,
        .OCRxA.ptr16  = &OCR1A,   .OCRxB.ptr16  = &OCR1B,
        .TCNTx.ptr16  = &TCNT1
    },
    [TIM_2] = { 
        .TCCRxA = &TCCR2A,  .TCCRxB = &TCCR2B,
        .TIFRx  = &TIFR2,   .TIMSKx = &TIMSK2,
        .OCRxA.ptr8  = &OCR2A,   .OCRxB.ptr8  = &OCR2B,
        .TCNTx.ptr8  = &TCNT2
    },
};
// clang-format on

#define TOVx_SHIFT  0
#define OCFxA_SHIFT 1
#define OCFxA_SHIFT 2

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

static TIM_handle_t *timer_handles[3] = {[TIM_0] = 0, [TIM_1] = 0, [TIM_2] = 0};
static void TIM_register_handle(TIM_handle_t *htim) {
    timer_handles[htim->config.timer] = htim;
    // TODO: Dejar que vuele todo o ser mem-safe?
    // TODO: Comprobar si el timer ya tiene un handle registrado
}

static inline void TIM_update_state(TIM_handle_t *htim) {
    uint8_t flag_reg = *TIM_regs[htim->config.timer].TIFRx;
    if (flag_reg & (1 << TOVx_SHIFT)) {
        htim->state = TIM_STATE_TIMEOUT;
    } else if (flag_reg & (1 << OCIExA_SHIFT) || flag_reg & (1 << OCIExB_SHIFT)) {
        htim->state = TIM_STATE_MATCH;
    }
}

TIM_state_t TIM_get_state(TIM_handle_t *htim) {
    TIM_update_state(htim);
    return htim->state;
}
static inline void TIM_set_clk_source(TIM_handle_t *htim) {
    *TIM_regs[htim->config.timer].TCCRxB |= htim->config.clk_source;
    htim->state = TIM_STATE_BUSY;
}
static inline void TIM_clear_clk_source(TIM_handle_t *htim) {
    *TIM_regs[htim->config.timer].TCCRxB &= ~htim->config.clk_source;
    htim->state = TIM_STATE_READY;
}
static inline void TIM_set_timer_value(TIM_handle_t *htim) {
    if (htim->config.timer == TIM_1) {
        *TIM_regs[htim->config.timer].TCNTx.ptr16 = htim->config.preset_value;
    } else {
        *TIM_regs[htim->config.timer].TCNTx.ptr8 = htim->config.preset_value;
    }
}
static void TIM_base_config(TIM_handle_t *htim, TIM_init_t *cfg) {
    htim->config.timer        = cfg->timer;
    htim->config.preset_value = cfg->preset_value;
    htim->config.mode         = cfg->mode;
    htim->config.clk_source   = cfg->clk_source;
    htim->state               = TIM_STATE_READY;

    *TIM_regs[cfg->timer].TCCRxA = 0;
    *TIM_regs[cfg->timer].TCCRxB = 0;
}

void TIM_base_init(TIM_handle_t *htim, TIM_init_t *cfg) {
    TIM_base_config(htim, cfg);
    TIM_set_timer_value(htim);
    TIM_register_handle(htim);
}

void TIM_base_start(TIM_handle_t *htim) {
    TIM_set_clk_source(htim);
}

void TIM_base_stop(TIM_handle_t *htim) {
    TIM_clear_clk_source(htim);
    *TIM_regs[htim->config.timer].TIFRx |= (1 << TOVx_SHIFT);    // Clear by writing 1 (p.88 - 14.9.7 TIFR0 (apply to all timers))
}

void TIM_base_start_IT(TIM_handle_t *htim) {
    *TIM_regs[htim->config.timer].TIMSKx |= (1 << TOIEx_SHIFT);
    TIM_base_start(htim);
}

void TIM_base_stop_IT(TIM_handle_t *htim) {
    *TIM_regs[htim->config.timer].TIMSKx &= ~(1 << TOIEx_SHIFT);
    TIM_base_stop(htim);
}

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

// TODO: Revisar si no separar en canal A y canal B
static void TIM_set_compare_value(TIM_handle_t *htim) {
    if (htim->config.timer == TIM_1) {
        if (htim->config.mode.ctc.channel == CTC_CHANNEL_A) {
            *TIM_regs[htim->config.timer].OCRxA.ptr16 = htim->config.preset_value;
        } else {
            *TIM_regs[htim->config.timer].OCRxB.ptr16 = htim->config.preset_value;
        }
    } else {
        if (htim->config.mode.ctc.channel == CTC_CHANNEL_A) {
            *TIM_regs[htim->config.timer].OCRxA.ptr8 = htim->config.preset_value;
        } else {
            *TIM_regs[htim->config.timer].OCRxB.ptr8 = htim->config.preset_value;
        }
    }
}

void TIM_CTC_init(TIM_handle_t *htim, TIM_init_t *cfg) {
    TIM_base_config(htim, cfg);
    TIM_set_compare_value(htim);
    *TIM_regs[cfg->timer].TCCRxA |= (1 << WGMx1_SHIFT);    // CTC mode
    TIM_register_handle(htim);
}

void TIM_CTC_A_start(TIM_handle_t *htim) {
    TIM_set_clk_source(htim);
}

void TIM_CTC_B_start(TIM_handle_t *htim) {
    TIM_set_clk_source(htim);
}

void TIM_CTC_A_stop(TIM_handle_t *htim) {
    TIM_base_stop(htim);
    *TIM_regs[htim->config.timer].TIFRx |= (1 << OCIExA_SHIFT);    // Clear by writing 1 (p.88 - 14.9.7 TIFR0 (apply to all timers))
}

void TIM_CTC_B_stop(TIM_handle_t *htim) {
    TIM_base_stop(htim);
    *TIM_regs[htim->config.timer].TIFRx |= (1 << OCIExB_SHIFT);    // Clear by writing 1 (p.88 - 14.9.7 TIFR0 (apply to all timers))
}

void TIM_CTC_A_start_IT(TIM_handle_t *htim) {
    *TIM_regs[htim->config.timer].TIMSKx |= (1 << OCIExA_SHIFT);
    TIM_CTC_start(htim);
}

void TIM_CTC_B_start_IT(TIM_handle_t *htim) {
    *TIM_regs[htim->config.timer].TIMSKx |= (1 << OCIExB_SHIFT);
    TIM_CTC_start(htim);
}

void TIM_CTC_A_stop_IT(TIM_handle_t *htim) {
    *TIM_regs[htim->config.timer].TIMSKx &= ~(1 << OCIExA_SHIFT);
    TIM_CTC_stop(htim);
}

void TIM_CTC_B_stop_IT(TIM_handle_t *htim) {
    *TIM_regs[htim->config.timer].TIMSKx &= ~(1 << OCIExB_SHIFT);
    TIM_CTC_stop(htim);
}

/* ---------------------- Callback interrupts handlers ---------------------- */
extern void TIM_period_elapsed_callback(TIM_handle_t *htim) {
    // Callback function to be implemented by the user
}
extern void TIM_CTC_callback(TIM_handle_t *htim) {
    // Callback function to be implemented by the user
}

// TODO: Como hacer para que las ISR reciban el mismo handle? -> REVISAR SOLUCION CON HANDLES_REGISTER
// --- ISR Handlers ---
#define TIMER_ISR(TIMER_NAME, TIMER_NUMBER, COMPA, COMPB, OVF)                                   \
    ISR(TIMER_NAME##_##COMPA##_vect) {                                                           \
        if (timer_handles[TIM_##TIMER] && timer_handles[TIM_##TIMER]->state == TIM_STATE_BUSY) { \
            timer_handles[TIM_##TIMER]->state = TIM_STATE_MATCH;                                 \
            TIM_CTC_callback(timer_handles[TIM_##TIMER]);                                        \
        }                                                                                        \
    }                                                                                            \
    ISR(TIMER_NAME##_##COMPB##_vect) {                                                           \
        if (timer_handles[TIM_##TIMER] && timer_handles[TIM_##TIMER]->state == TIM_STATE_BUSY) { \
            timer_handles[TIM_##TIMER]->state = TIM_STATE_MATCH;                                 \
            TIM_CTC_callback(timer_handles[TIM_##TIMER]);                                        \
        }                                                                                        \
    }                                                                                            \
    ISR(TIMER_NAME##_##OVF##_vect) {                                                             \
        if (timer_handles[TIM_##TIMER] && timer_handles[TIM_##TIMER]->state == TIM_STATE_BUSY) { \
            timer_handles[TIM_##TIMER]->state = TIM_STATE_TIMEOUT;                               \
            TIM_period_elapsed_callback(timer_handles[TIM_##TIMER]);                             \
        }                                                                                        \
    }

TIMER_ISR(TIMER0, 0, COMPA, COMPB, OVF)
TIMER_ISR(TIMER1, 1, COMPA, COMPB, OVF)
TIMER_ISR(TIMER2, 2, COMPA, COMPB, OVF)