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

uint8_t I2C_start(uint8_t addr) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    if ((TWSR & 0xF8) != TW_START)
        return 1;

    TWDR = (addr << 1);    // modo escritura
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    return ((TWSR & 0xF8) == TW_MT_SLA_ACK) ? 0 : 2;
}

uint8_t I2C_start_read(uint8_t addr) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    if ((TWSR & 0xF8) != TW_START)
        return 1;

    TWDR = (addr << 1) | 0x01;    // modo lectura
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    return ((TWSR & 0xF8) == TW_MR_SLA_ACK) ? 0 : 2;
}

uint8_t I2C_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return ((TWSR & 0xF8) == TW_MT_DATA_ACK) ? 0 : 1;
}

uint8_t I2C_read_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t I2C_read_nack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

void I2C_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);    // REVIEW - Revisar si se apaga por poner un 1 con OR o limpiando
    while (TWCR & (1 << TWSTO));
}
