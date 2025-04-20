/**
 * @file clock.c
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-14
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#include "clock.h"

#ifdef USE_CPU_CLOCK_PRESCALER_AT_RUNTIME

#include <avr/interrupt.h>
#include <avr/io.h>

void clock_prescaler_config(clk_prescaler_t prescaler) {
    cli();
    CLKPR = (1 << CLKPCE);
    CLKPR = prescaler;

    f_cpu_hz = F_CLK_HZ / (1 << prescaler);
}

#endif