/**
 * @file callbacks.c
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-29
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#include "board.h"

#ifdef USE_UART
#include "Drivers/uart/uart.h"

extern void UART_tx_callback(UART_handle_t *huart) {
}

extern void UART_rx_callback(UART_handle_t *huart) {
}
#endif

#ifdef USE_TIMER
#include "Drivers/timer/timer.h"

extern void TIM_period_elapsed_callback(TIM_handle_t *htim) {
}

extern void TIM_CTC_callback(TIM_handle_t *htim) {
}
#endif

#ifdef USE_ADC
#include "Drivers/adc/adc.h"

extern void ADC_EOC_callback(ADC_handle_t *hadc) {
}
#endif