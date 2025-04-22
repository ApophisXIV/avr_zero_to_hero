#include "timer.h"

#include "../../board.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#define TMR0_MAX_VALUE 255
#define TMR1_MAX_VALUE 65535
#define TMR2_MAX_VALUE 255

typedef enum {
    TMR0_DIV_1    = 0 << CS02 | 0 << CS01 | 1 << CS00,
    TMR0_DIV_8    = 0 << CS02 | 1 << CS01 | 0 << CS00,
    TMR0_DIV_64   = 0 << CS02 | 1 << CS01 | 1 << CS00,
    TMR0_DIV_256  = 1 << CS02 | 0 << CS01 | 0 << CS00,
    TMR0_DIV_1024 = 1 << CS02 | 0 << CS01 | 1 << CS00,
} tmr0_prescaler_t;

typedef enum {
    TMR1_DIV_1    = 0 << CS12 | 0 << CS11 | 1 << CS10,
    TMR1_DIV_8    = 0 << CS12 | 1 << CS11 | 0 << CS10,
    TMR1_DIV_64   = 0 << CS12 | 1 << CS11 | 1 << CS10,
    TMR1_DIV_256  = 1 << CS12 | 0 << CS11 | 0 << CS10,
    TMR1_DIV_1024 = 1 << CS12 | 0 << CS11 | 1 << CS10,
} tmr1_prescaler_t;

typedef enum {
    TMR2_DIV_1    = 0 << CS22 | 0 << CS21 | 1 << CS20,
    TMR2_DIV_8    = 0 << CS22 | 1 << CS21 | 0 << CS20,
    TMR2_DIV_32   = 0 << CS22 | 1 << CS21 | 1 << CS20,
    TMR2_DIV_64   = 1 << CS22 | 0 << CS21 | 0 << CS20,
    TMR2_DIV_128  = 1 << CS22 | 0 << CS21 | 1 << CS20,
    TMR2_DIV_256  = 1 << CS22 | 1 << CS21 | 0 << CS20,
    TMR2_DIV_1024 = 1 << CS22 | 1 << CS21 | 1 << CS20,
} tmr2_prescaler_t;

TIM_state_t TIM_get_state(TIM_handle_t *htim) {
    switch (htim->timer) {
    case TIM_0:
        if (TIFR0 & (1 << TOV0)) {
            htim->state = TIM_STATE_TIMEOUT;
        }
        break;
    case TIM_1:
        if (TIFR1 & (1 << TOV1)) {
            htim->state = TIM_STATE_TIMEOUT;
        }
        break;
    case TIM_2:
        if (TIFR2 & (1 << TOV2)) {
            htim->state = TIM_STATE_TIMEOUT;
        }
        break;
    default:
        htim->state = TIM_STATE_READY;
        break;
    }
    return htim->state;
}

void TIM_base_init(TIM_handle_t *htim) {
    switch (htim->timer) {
    case TIM_0:
        TCCR0A = 0;
        TCCR0B = 0;
        TCCR0B |= htim->clk_source;
        break;
    case TIM_1:
        TCCR1A = 0;
        TCCR1B = 0;
        TCCR1B |= htim->clk_source;
        break;
    case TIM_2:
        TCCR2A = 0;
        TCCR2B = 0;
        TCCR2B |= htim->clk_source;
        break;
    }
}

void TIM_base_start(TIM_handle_t *htim){
    
}
void TIM_base_stop(TIM_handle_t *htim);

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

/* ---------------------- Callback interrupts handlers ---------------------- */
extern TIM_handle_t htim;

ISR(TIMER0_OVF_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_period_elapsed_callback(&htim);
    }
    sei();
}

ISR(TIMER0_COMPA_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_compare_match_A_callback(&htim);
    }
    sei();
}

ISR(TIMER0_COMPB_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_compare_match_B_callback(&htim);
    }
    sei();
}

ISR(TIMER1_OVF_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_period_elapsed_callback(&htim);
    }
    sei();
}

ISR(TIMER1_COMPA_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_compare_match_A_callback(&htim);
    }
    sei();
}

ISR(TIMER1_COMPB_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_compare_match_B_callback(&htim);
    }
    sei();
}

ISR(TIMER2_OVF_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_period_elapsed_callback(&htim);
    }
    sei();
}

ISR(TIMER2_COMPA_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_compare_match_A_callback(&htim);
    }
    sei();
}

ISR(TIMER2_COMPB_vect) {
    cli();
    if (htim.state == TIM_STATE_BUSY) {
        htim.state = TIM_STATE_READY;
        TIM_compare_match_B_callback(&htim);
    }
    sei();
}