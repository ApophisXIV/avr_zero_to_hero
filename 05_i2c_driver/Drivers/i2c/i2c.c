/**
 * @file i2c.c
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
#include "i2c.h"
#include "../../board.h"
#include <avr/io.h>
#include <util/twi.h>

void I2C_init(I2C_freq_t freq) {
    TWSR = 0x00;    // Prescaler = 1
    TWBR = ((F_CPU / freq) - 16) / 2;
    TWCR = (1 << TWEN);
}

void I2C_reset(void) {
    TWCR &= ~(1 << TWEN);
    TWCR |= (1 << TWEN);
}

#define TIMEOUT 10000

static inline __attribute__((always_inline)) uint8_t wait_for_twint(void) {
    uint16_t count = 0;
    while (!(TWCR & (1 << TWINT))) {
        if (++count > TIMEOUT) return 0;
    }
    return 1;
}

I2C_status_t I2C_start(uint8_t addr) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    if (!wait_for_twint()) return I2C_ERR_TIMEOUT;

    if ((TWSR & 0xF8) != TW_START)
        return I2C_ERR_START;

    TWDR = (addr << 1);    // modo escritura
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!wait_for_twint()) return I2C_ERR_TIMEOUT;

    return ((TWSR & 0xF8) == TW_MT_SLA_ACK) ? I2C_OK : I2C_ERR_SLA_NACK;
}

I2C_status_t I2C_start_read(uint8_t addr) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    if (!wait_for_twint()) return I2C_ERR_TIMEOUT;

    if ((TWSR & 0xF8) != TW_START)
        return I2C_ERR_START;

    TWDR = (addr << 1) | 0x01;    // modo lectura
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!wait_for_twint()) return I2C_ERR_TIMEOUT;

    return ((TWSR & 0xF8) == TW_MR_SLA_ACK) ? I2C_OK : I2C_ERR_SLA_NACK;
}

I2C_status_t I2C_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!wait_for_twint()) return I2C_ERR_TIMEOUT;

    return ((TWSR & 0xF8) == TW_MT_DATA_ACK) ? I2C_OK : I2C_ERR_DATA_NACK;
}

I2C_status_t I2C_read_ack(uint8_t *data) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    if (!wait_for_twint()) return I2C_ERR_TIMEOUT;

    *data = TWDR;
    return I2C_OK;
}

I2C_status_t I2C_read_nack(uint8_t *data) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!wait_for_twint()) return I2C_ERR_TIMEOUT;

    *data = TWDR;
    return I2C_OK;
}

void I2C_stop(void) {
    TWCR             = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    uint16_t timeout = 1000;
    while ((TWCR & (1 << TWSTO)) && --timeout);
}
