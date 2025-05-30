/**
 * @file ils94202.c
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

#include "ils94202.h"
#include "../../Drivers/gpio/gpio.h"
#include "../../Drivers/i2c/i2c.h"

#define SLAVE_SCL_FREQ_DEFAULT 400e3

#define CTRL3_REG_ADDRESS  0x88
#define CTRL3_REG_PWDN_BIT 1 << 3

struct ILS94202_handle {
    GPIO_pin_t sda_pin, scl_pin;
    GPIO_port_t i2c_port;
    I2C_freq_t i2c_freq;
    uint8_t address;
    bool is_pullup_external;
};

static ILS94202_handle_t ILS94202_handle = {0};

ILS94202_handle_t *ILS94202_init(ILS94202_init_t *ILS94202_cfg) {
    ILS94202_handle.address            = ILS94202_cfg->address;
    ILS94202_handle.sda_pin            = ILS94202_cfg->sda_pin;
    ILS94202_handle.scl_pin            = ILS94202_cfg->scl_pin;
    ILS94202_handle.i2c_port           = ILS94202_cfg->i2c_port;
    ILS94202_handle.i2c_freq           = ILS94202_cfg->i2c_freq;
    ILS94202_handle.is_pullup_external = ILS94202_cfg->use_external_pullup;
    I2C_init(ILS94202_cfg->i2c_freq);

    if (!ILS94202_cfg->use_external_pullup) {
        GPIO_config(ILS94202_cfg->i2c_port, ILS94202_cfg->sda_pin, GPIO_INPUT_PULLUP);
        GPIO_config(ILS94202_cfg->i2c_port, ILS94202_cfg->scl_pin, GPIO_INPUT_PULLUP);
    }

    return &ILS94202_handle;
}

void ILS94202_bus_reset(ILS94202_handle_t *hILS94202) {
    I2C_reset();
}

void ILS94202_set_slave_address(ILS94202_handle_t *hILS94202, uint8_t slave_address) {
    if (slave_address != ILS94202_SLAVE_ADDRESS_DEFAULT || slave_address != ILS94202_SLAVE_ADDRESS_ALT) return;
    hILS94202->address = slave_address;
}

I2C_status_t ILS94202_set_power_down_mode(ILS94202_handle_t *hILS94202) {
    I2C_status_t status;

    // NOTE: Asi verifica error
    // status = I2C_start(hILS94202->address);
    // if (status != I2C_OK) {
    //     I2C_stop();
    //     return status;
    // }

    // status = I2C_write(CTRL3_REG_ADDRESS);
    // if (status != I2C_OK) {
    //     I2C_stop();
    //     return status;
    // }

    // status = I2C_write(CTRL3_REG_PWDN_BIT);
    // I2C_stop();

    // MODO sin verificacion de rrores
    I2C_start(hILS94202->address);
    I2C_write(CTRL3_REG_ADDRESS);
    I2C_write(CTRL3_REG_PWDN_BIT);
    I2C_stop();
    return status;
}

bool ILS94202_is_not_power_down(ILS94202_handle_t *hILS94202) {
    // TODO: Revisar si basta con ver que me responda 0
    I2C_status_t status = I2C_start(hILS94202->address);
    I2C_stop();
    return status == I2C_OK;

    // OPCION CON PINES GPIO
    // GPIO_pin_state_t sda_level = GPIO_read_pin(hILS94202->i2c_port, hILS94202->sda_pin);
    // GPIO_pin_state_t scl_level = GPIO_read_pin(hILS94202->i2c_port, hILS94202->scl_pin);
    // // I2C_start(hILS94202->i2c_freq);
    // return sda_level == GPIO_HIGH && scl_level == GPIO_HIGH;
}
