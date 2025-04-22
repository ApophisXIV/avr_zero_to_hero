/**
 * @file gpio.h
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
} gpio_port_t;

typedef enum {
    GPIO_OUTPUT,
    GPIO_OUTPUT_INITIAL_HIGH,
    GPIO_OUTPUT_INITIAL_LOW,
    GPIO_INPUT,
    GPIO_INPUT_PULLUP
} gpio_mode_t;

typedef enum {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1,
} gpio_pin_state_t;

typedef enum {
    GPIO_0 = (1 << 0),
    GPIO_1 = (1 << 1),
    GPIO_2 = (1 << 2),
    GPIO_3 = (1 << 3),
    GPIO_4 = (1 << 4),
    GPIO_5 = (1 << 5),
    GPIO_6 = (1 << 6),
    GPIO_7 = (1 << 7),
} gpio_pin_t;

void gpio_config(gpio_port_t port, gpio_pin_t pins, gpio_mode_t mode);

void gpio_write_pin(gpio_port_t port, gpio_pin_t pin, gpio_pin_state_t state);

void gpio_toggle_pin(gpio_port_t port, gpio_pin_t pin);

gpio_pin_state_t gpio_read_pin(gpio_port_t port, gpio_pin_t pin);

#endif    // GPIO_H