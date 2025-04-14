/**
 * @file board.h
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
#ifndef BOARD_H
#define BOARD_H

/* ---------------------------- Board definitions --------------------------- */
#define ARDUINO_NANO_16MHZ              0
#define ARDUINO_NANO_16MHZ_F_CPU_DIV8   1
#define ATMEGA328P_EXT_4MHZ             2
#define ATMEGA328P_EXT_4MHZ_F_CPU_DIV8  3
#define ATMEGA328P_EXT_8MHZ             4
#define ATMEGA328P_EXT_8MHZ_F_CPU_DIV8  5
#define ATMEGA328P_EXT_16MHZ            6
#define ATMEGA328P_EXT_16MHZ_F_CPU_DIV8 7

/* ----------------------------- Board selection ---------------------------- */
#define BOARD ARDUINO_NANO_16MHZ_F_CPU_DIV8
/* -------------------------------------------------------------------------- */

#if BOARD == ATMEGA328P_EXT_4MHZ || BOARD == ATMEGA328P_EXT_4MHZ_F_CPU_DIV8 ||   \
    BOARD == ATMEGA328P_EXT_8MHZ || BOARD == ATMEGA328P_EXT_8MHZ_F_CPU_DIV8 ||   \
    BOARD == ATMEGA328P_EXT_16MHZ || BOARD == ATMEGA328P_EXT_16MHZ_F_CPU_DIV8 || \
    BOARD == ARDUINO_NANO_16MHZ || BOARD == ARDUINO_NANO_16MHZ_F_CPU_DIV8
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif
#else
#error "BOARD value must be selected"
#endif

#if BOARD == ARDUINO_NANO_16MHZ || BOARD == ARDUINO_NANO_16MHZ_F_CPU_DIV8
#define F_CLK_HZ 16000000
#elif BOARD == ATMEGA328P_EXT_4MHZ || BOARD == ATMEGA328P_EXT_4MHZ_F_CPU_DIV8
#define F_CLK_HZ 4000000
#elif BOARD == ATMEGA328P_EXT_8MHZ || BOARD == ATMEGA328P_EXT_8MHZ_F_CPU_DIV8
#define F_CLK_HZ 8000000
#error "F_CLK_HZ is not defined for selected BOARD"
#endif

#if BOARD == ARDUINO_NANO_16MHZ_F_CPU_DIV8 || BOARD == ATMEGA328P_EXT_16MHZ_F_CPU_DIV8 || \
    BOARD == ATMEGA328P_EXT_8MHZ_F_CPU_DIV8 || BOARD == ATMEGA328P_EXT_4MHZ_F_CPU_DIV8
#define F_CPU_HZ F_CLK_HZ / 8
#else
#define F_CPU_HZ F_CLK_HZ
#endif

#ifdef F_CLK_HZ
#define F_CLK_KHZ F_CLK_HZ / 1000
// Compatibility with avr/delay.h
#define F_CPU F_CLK_HZ
#endif

// #define USE_CPU_CLOCK_PRESCALER_AT_RUNTIME
#ifdef USE_CPU_CLOCK_PRESCALER_AT_RUNTIME
static float f_cpu_hz = F_CPU_HZ;
#undef F_CPU_HZ
#endif

#endif    // BOARD_H