/**
 * @file ils94202.h
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-05-13
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */
#ifndef ILS94202_H
#define ILS94202_H

#include "../../Drivers/gpio/gpio.h"
#include "../../Drivers/i2c/i2c.h"
#include <stdbool.h>
#include <stdint.h>

#define ILS94202_SLAVE_ADDRESS_DEFAULT 0x50
#define ILS94202_SLAVE_ADDRESS_ALT     0x52

typedef struct {
    uint8_t address;
    I2C_freq_t i2c_freq;
    GPIO_pin_t sda_pin, scl_pin;
    GPIO_port_t i2c_port;
} ILS94202_init_t;

struct ILS94202_handle;
typedef struct ILS94202_handle ILS94202_handle_t;

ILS94202_handle_t *ILS94202_init(ILS94202_init_t *ILS94202_cfg);
void ILS94202_set_slave_address(ILS94202_handle_t *hILS94202, uint8_t slave_address);
void ILS94202_set_power_down_mode(ILS94202_handle_t *hILS94202);
bool ILS94202_is_power_down(ILS94202_handle_t *hILS94202);

#endif    // ILS94202_H