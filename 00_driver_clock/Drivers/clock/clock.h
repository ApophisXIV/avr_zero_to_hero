/**
 * @file clock.h
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief Clock driver
 * @version 0.1
 * @date 2025-04-14
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef CLOCK_H
#define CLOCK_H

#include "../../board.h"

#ifdef USE_CPU_CLOCK_PRESCALER_AT_RUNTIME

typedef enum {
    CLK_DIV_1   = 0b0000,
    CLK_DIV_2   = 0b0001,
    CLK_DIV_4   = 0b0010,
    CLK_DIV_8   = 0b0011,
    CLK_DIV_16  = 0b0100,
    CLK_DIV_32  = 0b0101,
    CLK_DIV_64  = 0b0110,
    CLK_DIV_128 = 0b0111,
    CLK_DIV_256 = 0b1000
} clk_prescaler_t;

/**
 * @brief  Set dinamicaly the CPU clock prescaler
 * @param  prescaler: CLK_DIV_1 to CLK_DIV_256
 * @pre    The prescaler must be a valid clk_prescaler_t value
 * @post   The globals interrupts was disabled for safety reason and must be re-enabled if nedded
 * @post   The CPU clock (in Hz) will be F_CPU_HZ divided by the selected prescaler
 * @retval None
 */
void clock_prescaler_config(clk_prescaler_t prescaler);

#endif

#endif    // CLOCK_H