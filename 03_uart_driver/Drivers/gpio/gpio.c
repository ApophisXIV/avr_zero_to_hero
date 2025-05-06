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
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stddef.h>

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
} GPIO_regs_t;

static GPIO_regs_t GPIO_get_registers(GPIO_port_t port) {
    switch (port) {
    case GPIO_PORTB: return (GPIO_regs_t){.ddrx = &DDRB, .portx = &PORTB, .pinx = &PINB};
    case GPIO_PORTC: return (GPIO_regs_t){.ddrx = &DDRC, .portx = &PORTC, .pinx = &PINC};
    case GPIO_PORTD: return (GPIO_regs_t){.ddrx = &DDRD, .portx = &PORTD, .pinx = &PIND};
    }
    return (GPIO_regs_t){.ddrx = NULL, .portx = NULL, .pinx = NULL};    // This should never happen
}

typedef struct {
    GPIO_pin_state_t state;
    GPIO_mode_t mode;
} GPIO_INTx_t;

static GPIO_INTx_t GPIO_INTx[2] = {
    {GPIO_LOW, GPIO_INPUT_IT_LOW_LEVEL},
    {GPIO_LOW, GPIO_INPUT_IT_LOW_LEVEL},
};

#define GPIO_IT_LOW_LEVEL_VALUE    0
#define GPIO_IT_LEVEL_CHANGE_VALUE 1
#define GPIO_IT_FALLING_VALUE      2
#define GPIO_IT_RISING_VALUE       3

static uint8_t GPIO_IT_get_mode(GPIO_mode_t mode, uint8_t pin) {
    uint8_t offset = pin == GPIO_2 ? ISC00 : ISC10;
    switch (mode) {
    case GPIO_INPUT_IT_LOW_LEVEL: return GPIO_IT_LOW_LEVEL_VALUE << offset;
    case GPIO_INPUT_IT_LOW_LEVEL_WITH_PULLUP: return GPIO_IT_LOW_LEVEL_VALUE << offset;
    case GPIO_INPUT_IT_LEVEL_CHANGE: return GPIO_IT_LEVEL_CHANGE_VALUE << offset;
    case GPIO_INPUT_IT_FALLING: return GPIO_IT_FALLING_VALUE << offset;
    case GPIO_INPUT_IT_RISING: return GPIO_IT_RISING_VALUE << offset;
    }
}

static inline __attribute__((always_inline)) void GPIO_INTx_config(GPIO_port_t port, GPIO_pin_t *pin, GPIO_mode_t mode) {
    if ((*pin & GPIO_2) && (port == GPIO_PORTD)) {
        if (mode == GPIO_INPUT_IT_LOW_LEVEL_WITH_PULLUP) GPIO_INTx[0].state = GPIO_HIGH;
        EICRA = (EICRA & (0xC)) | GPIO_IT_get_mode(mode, *pin);
        EIMSK |= (1 << INT0);
        GPIO_INTx[0].mode = mode;
        *pin &= ~(GPIO_2);
    }
    if ((*pin & GPIO_3) && (port == GPIO_PORTD)) {
        if (mode == GPIO_INPUT_IT_LOW_LEVEL_WITH_PULLUP) GPIO_INTx[1].state = GPIO_HIGH;
        EICRA = (EICRA & (0x3)) | GPIO_IT_get_mode(mode, *pin);
        EIMSK |= (1 << INT1);
        GPIO_INTx[1].mode = mode;
        *pin &= ~(GPIO_3);
    }
}

static inline __attribute__((always_inline)) void GPIO_PCICR_config(GPIO_port_t port, GPIO_pin_t pin) {
    switch (port) {
    case GPIO_PORTB:
        PCICR |= (1 << PCIE0);
        PCMSK0 |= pin;
        break;
    case GPIO_PORTC:
        PCICR |= (1 << PCIE1);
        PCMSK1 |= pin;
        break;
    case GPIO_PORTD:
        PCICR |= (1 << PCIE2);
        PCMSK2 |= pin;
        break;
    default:
        break;
    }
}

void GPIO_config(GPIO_port_t port, GPIO_pin_t pins, GPIO_mode_t mode) {
    ASSERT_GPIO_PORT(port, );
    GPIO_regs_t reg = GPIO_get_registers(port);

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
    case GPIO_INPUT_IT_LOW_LEVEL:
        GPIO_INTx_config(port, &pins, GPIO_INPUT_IT_LOW_LEVEL);
        if (pins) {
            *reg.ddrx &= ~pins;
            *reg.portx &= ~pins;
        }
        break;
    case GPIO_INPUT_IT_LOW_LEVEL_WITH_PULLUP:
        GPIO_INTx_config(port, &pins, GPIO_INPUT_IT_LOW_LEVEL_WITH_PULLUP);
        if (pins) {
            *reg.ddrx &= ~pins;
            *reg.portx |= pins;
        }
        break;
    case GPIO_INPUT_IT_LEVEL_CHANGE:
        GPIO_INTx_config(port, &pins, GPIO_INPUT_IT_LEVEL_CHANGE);
        if (pins) {
            *reg.ddrx &= ~pins;
            *reg.portx &= ~pins;
            GPIO_PCICR_config(port, pins);
        }
        break;
    case GPIO_INPUT_IT_FALLING:
        GPIO_INTx_config(port, &pins, GPIO_INPUT_IT_FALLING);
        if (pins) {
            *reg.ddrx &= ~pins;
            *reg.portx &= ~pins;
        }
        break;
    case GPIO_INPUT_IT_RISING:
        GPIO_INTx_config(port, &pins, GPIO_INPUT_IT_RISING);
        if (pins) {
            *reg.ddrx &= ~pins;
            *reg.portx &= ~pins;
        }
        break;
    }
}

void GPIO_write_pin(GPIO_port_t port, GPIO_pin_t pin, GPIO_pin_state_t state) {
    ASSERT_GPIO_PORT(port, );
    GPIO_regs_t reg = GPIO_get_registers(port);

    if (state == GPIO_HIGH) {
        *reg.portx |= pin;
    } else {
        *reg.portx &= ~pin;
    }
}

void GPIO_toggle_pin(GPIO_port_t port, GPIO_pin_t pin) {
    ASSERT_GPIO_PORT(port, );
    GPIO_regs_t reg = GPIO_get_registers(port);
    *reg.portx ^= pin;
}

GPIO_pin_state_t GPIO_read_pin(GPIO_port_t port, GPIO_pin_t pin) {
    ASSERT_GPIO_PORT(port, GPIO_LOW);
    GPIO_regs_t reg = GPIO_get_registers(port);
    return (*reg.pinx & pin) ? GPIO_HIGH : GPIO_LOW;
}

ISR(INT0_vect) {
    GPIO_pin_state_t current_state = GPIO_read_pin(GPIO_PORTD, GPIO_2);
    switch (GPIO_INTx[0].mode) {
    case GPIO_INPUT_IT_FALLING:
        GPIO_EXTI_callback(GPIO_PORTD, GPIO_2, GPIO_EDGE_FALLING);
        GPIO_INTx[0].state = GPIO_EDGE_FALLING;
        break;
    case GPIO_INPUT_IT_RISING:
        GPIO_EXTI_callback(GPIO_PORTD, GPIO_2, GPIO_EDGE_RISING);
        GPIO_INTx[0].state = GPIO_EDGE_RISING;
        break;
    case GPIO_INPUT_IT_LEVEL_CHANGE:
        GPIO_EXTI_callback(GPIO_PORTD, GPIO_2, current_state);
        GPIO_INTx[0].state = current_state;
    case GPIO_INPUT_IT_LOW_LEVEL:
    case GPIO_INPUT_IT_LOW_LEVEL_WITH_PULLUP:
        GPIO_EXTI_callback(GPIO_PORTD, GPIO_2, GPIO_LOW);
        GPIO_INTx[0].state = GPIO_LOW;
    default:
        break;
    }
}

ISR(INT1_vect) {
    GPIO_pin_state_t current_state = GPIO_read_pin(GPIO_PORTD, GPIO_3);
    switch (GPIO_INTx[1].mode) {
    case GPIO_INPUT_IT_FALLING:
        GPIO_EXTI_callback(GPIO_PORTD, GPIO_3, GPIO_EDGE_FALLING);
        GPIO_INTx[1].state = GPIO_EDGE_FALLING;
        break;
    case GPIO_INPUT_IT_RISING:
        GPIO_EXTI_callback(GPIO_PORTD, GPIO_3, GPIO_EDGE_RISING);
        GPIO_INTx[1].state = GPIO_EDGE_RISING;
        break;
    case GPIO_INPUT_IT_LEVEL_CHANGE:
        GPIO_EXTI_callback(GPIO_PORTD, GPIO_3, current_state);
        GPIO_INTx[1].state = current_state;
    case GPIO_INPUT_IT_LOW_LEVEL:
    case GPIO_INPUT_IT_LOW_LEVEL_WITH_PULLUP:
        GPIO_EXTI_callback(GPIO_PORTD, GPIO_3, GPIO_LOW);
        GPIO_INTx[1].state = GPIO_LOW;
    default:
        break;
    }
}

static volatile uint8_t PORTB_last_value = 0;
static volatile uint8_t PORTC_last_value = 0;
static volatile uint8_t PORTD_last_value = 0;

ISR(PCINT0_vect) {
    uint8_t port_changes = (PINB & PCMSK0) ^ PORTB_last_value;
    if (port_changes == 0) return;    // None pin changed (should never happen)

    for (GPIO_pin_t pin = GPIO_0; pin <= GPIO_7; pin <<= 1) {
        if ((port_changes & pin) && (~PORTB_last_value & pin)) {
            GPIO_EXTI_callback(GPIO_PORTB, pin, GPIO_EDGE_RISING);
        } else if ((port_changes & pin) && (PORTB_last_value & ~pin)) {
            GPIO_EXTI_callback(GPIO_PORTB, pin, GPIO_EDGE_FALLING);
        }
    }
    PORTB_last_value = PINB & PCMSK0;
}

ISR(PCINT1_vect) {
    uint8_t port_changes = (PINC & PCMSK1) ^ PORTB_last_value;
    if (port_changes == 0) return;    // None pin changed (should never happen)

    for (GPIO_pin_t pin = GPIO_0; pin <= GPIO_6; pin <<= 1) {
        if ((port_changes & pin) && (~PORTB_last_value & pin)) {
            GPIO_EXTI_callback(GPIO_PORTC, pin, GPIO_EDGE_RISING);
        } else if ((port_changes & pin) && (PORTC_last_value & ~pin)) {
            GPIO_EXTI_callback(GPIO_PORTC, pin, GPIO_EDGE_FALLING);
        }
    }
    PORTC_last_value = PINC & PCMSK1;
}

ISR(PCINT2_vect) {
    uint8_t port_changes = (PIND & PCMSK2) ^ PORTD_last_value;
    if (port_changes == 0) return;    // None pin changed (should never happen)

    for (GPIO_pin_t pin = GPIO_0; pin <= GPIO_7; pin <<= 1) {
        if ((port_changes & pin) && (~PORTD_last_value & pin)) {
            GPIO_EXTI_callback(GPIO_PORTD, pin, GPIO_EDGE_RISING);
        } else if ((port_changes & pin) && (PORTD_last_value & ~pin)) {
            GPIO_EXTI_callback(GPIO_PORTD, pin, GPIO_EDGE_FALLING);
        }
    }
    PORTD_last_value = PIND & PCMSK2;
}
