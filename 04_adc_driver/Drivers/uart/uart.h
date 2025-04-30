/**
 * @file uart.h
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-26
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UART_H
#define UART_H

#include <stdio.h>

#define BAUD 9600

struct uart;
typedef struct uart UART_handle_t;

void UART_init(void);

#endif    // UART_H