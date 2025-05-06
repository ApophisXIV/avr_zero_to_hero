/**
 * @file GPIO.h
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-20
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

typedef enum {
    GPIO_PORTB,
    GPIO_PORTC,
    GPIO_PORTD,
} GPIO_port_t;

typedef enum {
    GPIO_OUTPUT,
    GPIO_OUTPUT_INITIAL_HIGH,
    GPIO_OUTPUT_INITIAL_LOW,
    GPIO_INPUT,
    GPIO_INPUT_PULLUP,
    GPIO_INPUT_IT_LEVEL_CHANGE,
    GPIO_INPUT_IT_LOW_LEVEL,
    GPIO_INPUT_IT_LOW_LEVEL_WITH_PULLUP,
    GPIO_INPUT_IT_FALLING,
    GPIO_INPUT_IT_RISING,
} GPIO_mode_t;

typedef enum {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1,
    GPIO_EDGE_RISING,
    GPIO_EDGE_FALLING,
} GPIO_pin_state_t;

typedef enum {
    GPIO_0 = (1 << 0),
    GPIO_1 = (1 << 1),
    GPIO_2 = (1 << 2),
    GPIO_3 = (1 << 3),
    GPIO_4 = (1 << 4),
    GPIO_5 = (1 << 5),
    GPIO_6 = (1 << 6),
    GPIO_7 = (1 << 7),
} GPIO_pin_t;

void GPIO_config(GPIO_port_t port, GPIO_pin_t pins, GPIO_mode_t mode);

void GPIO_write_pin(GPIO_port_t port, GPIO_pin_t pin, GPIO_pin_state_t state);

void GPIO_toggle_pin(GPIO_port_t port, GPIO_pin_t pin);

GPIO_pin_state_t GPIO_read_pin(GPIO_port_t port, GPIO_pin_t pin);

extern void GPIO_EXTI_callback(GPIO_port_t port, GPIO_pin_t pin, GPIO_pin_state_t state);

#endif    // GPIO_H