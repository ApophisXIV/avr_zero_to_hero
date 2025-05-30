/**
 * @file I2C.h
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
#ifndef I2C_H
#define I2C_H

#include "../../board.h"
#ifdef USE_I2C

#include <stdint.h>

/**
 * @brief Velocidades estandar del bus I2C
 */
typedef enum {
    I2C_FREQ_100KHZ = 100000UL,
    I2C_FREQ_400KHZ = 400000UL,
} I2C_freq_t;

typedef enum {
    I2C_OK            = 0,
    I2C_ERR_START     = 1,
    I2C_ERR_SLA_NACK  = 2,
    I2C_ERR_DATA_NACK = 3,
    I2C_ERR_TIMEOUT   = 4,
} I2C_status_t;
/**
 * @brief Inicializa el periférico TWI con la frecuencia deseada
 *
 * @param freq Frecuencia del bus (100kHz o 400kHz)
 */
void I2C_init(I2C_freq_t freq);

/**
 * @brief Envía condición de START y dirección del esclavo en modo escritura
 *
 * @param addr Dirección de 7 bits del esclavo
 * @return 0 en éxito, otro valor si falla
 */
I2C_status_t I2C_start(uint8_t addr);

/**
 * @brief Envía condición de START y dirección del esclavo en modo lectura
 *
 * @param addr Dirección de 7 bits del esclavo
 * @return 0 en éxito, otro valor si falla
 */
I2C_status_t I2C_start_read(uint8_t addr);

/**
 * @brief Escribe un byte en el bus I2C
 *
 * @param data Byte a escribir
 * @return 0 en éxito, otro valor si falla
 */
I2C_status_t I2C_write(uint8_t data);

/**
 * @brief Lee un byte y responde con ACK
 *
 * @return Byte leído
 */
I2C_status_t I2C_read_ack(uint8_t *data);

/**
 * @brief Lee un byte y responde con NACK
 *
 * @return Byte leído
 */
I2C_status_t I2C_read_nack(uint8_t *data);

/**
 * @brief Envía condición de STOP
 */
void I2C_stop(void);

/**
 * @brief Reinicia el periferico
 */
void I2C_reset(void);

#endif
#endif    // I2C_H