/**
 * @file gpio.c
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

#include "gpio.h"
#include "../../board.h"
#include <avr/io.h>

#define ASSERT_GPIO_PORT(port, err_value)                                     \
    do {                                                                      \
        if (port != GPIO_PORTB && port != GPIO_PORTC && port != GPIO_PORTD) { \
            return err_value;                                                 \
        }                                                                     \
    } while (0)

typedef struct {
    volatile uint8_t *ddrx;     // Data direction (input/output)
    volatile uint8_t *portx;    // Port manipulation (set high/low)
    volatile uint8_t *pinx;     // Pin read value (high/low)
} gpio_regs_t;

static gpio_regs_t get_gpio_registers(gpio_port_t port) {
    switch (port) {
    case GPIO_PORTB: return (gpio_regs_t){.ddrx = &DDRB, .portx = &PORTB, .pinx = &PINB};
    case GPIO_PORTC: return (gpio_regs_t){.ddrx = &DDRC, .portx = &PORTC, .pinx = &PINC};
    case GPIO_PORTD: return (gpio_regs_t){.ddrx = &DDRD, .portx = &PORTD, .pinx = &PIND};
    }
}

void gpio_config(gpio_port_t port, gpio_pin_t pins, gpio_mode_t mode) {

    ASSERT_GPIO_PORT(port, );

    gpio_regs_t reg = get_gpio_registers(port);

    switch (mode) {
    case GPIO_INPUT:
        *reg.ddrx &= ~pins;
        *reg.portx &= ~pins;
        break;
    case GPIO_INPUT_PULLUP:
        *reg.ddrx &= ~pins;
        *reg.portx |= pins;
        break;
    case GPIO_OUTPUT:
        *reg.ddrx |= pins;
        break;
    case GPIO_OUTPUT_INITIAL_LOW:
        *reg.ddrx |= pins;
        *reg.portx &= ~pins;
        break;
    case GPIO_OUTPUT_INITIAL_HIGH:
        *reg.ddrx |= pins;
        *reg.portx |= pins;
        break;
    }
}

void gpio_write_pin(gpio_port_t port, gpio_pin_t pin, gpio_pin_state_t state) {

    ASSERT_GPIO_PORT(port, );

    gpio_regs_t reg = get_gpio_registers(port);

    if (state == GPIO_HIGH) {
        *reg.portx |= pin;
    } else {
        *reg.portx &= ~pin;
    }
}

void gpio_toggle_pin(gpio_port_t port, gpio_pin_t pin) {

    ASSERT_GPIO_PORT(port, );

    gpio_regs_t reg = get_gpio_registers(port);

    *reg.portx ^= pin;
}

gpio_pin_state_t gpio_read_pin(gpio_port_t port, gpio_pin_t pin) {

    ASSERT_GPIO_PORT(port, GPIO_LOW);

    gpio_regs_t reg = get_gpio_registers(port);

    return (*reg.pinx & pin) ? GPIO_HIGH : GPIO_LOW;
}
