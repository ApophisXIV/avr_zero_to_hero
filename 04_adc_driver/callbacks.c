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

#ifdef USE_GPIO
#include "Drivers/gpio/gpio.h"

__attribute__((weak)) void GPIO_EXTI_callback(GPIO_port_t port, GPIO_pin_t pin, GPIO_pin_state_t state) {
}
#endif

#ifdef USE_UART
#include "Drivers/uart/uart.h"

__attribute__((weak)) void UART_tx_callback(UART_handle_t *huart) {
}

__attribute__((weak)) void UART_rx_callback(UART_handle_t *huart) {
}
#endif

#ifdef USE_TIMER
#include "Drivers/timer/timer.h"

__attribute__((weak)) void TIM_period_elapsed_callback(TIM_handle_t *htim) {
}

__attribute__((weak)) void TIM_CTC_callback(TIM_handle_t *htim) {
}
#endif

#ifdef USE_ADC
#include "Drivers/adc/adc.h"

__attribute__((weak)) void ADC_EOC_callback(ADC_handle_t *hadc, uint16_t value) {
}

#endif