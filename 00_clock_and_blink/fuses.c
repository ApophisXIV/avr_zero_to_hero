/**
 * @file fuses.c
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @brief
 * @version 0.1
 * @date 2025-04-19
 *
 * @copyright Copyright (c) 2025. All rights reserved.
 *
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 *
 */

#include "board.h"
#include <avr/io.h>

#if (BOARD == ARDUINO_NANO_16MHZ_F_CPU_DIV8) || (BOARD == ATMEGA328P_EXT_16MHZ_F_CPU_DIV8)
FUSES = {
    .low      = FUSE_CKDIV8 & FUSE_SUT1 & FUSE_CKSEL0 & FUSE_CKOUT,
    .high     = FUSE_SPIEN & FUSE_BOOTSZ1 & FUSE_BOOTSZ0,
    .extended = FUSE_BODLEVEL1,
};
#elif (BOARD == ATMEGA328P_EXT_8MHZ_F_CPU_DIV8 || BOARD == ATMEGA328P_EXT_4MHZ_F_CPU_DIV8)
FUSES = {
    .low      = FUSE_CKDIV8 & FUSE_SUT1 & FUSE_CKSEL1 & FUSE_CKSEL0,
    .high     = FUSE_SPIEN & FUSE_BOOTSZ1 & FUSE_BOOTSZ0,
    .extended = FUSE_BODLEVEL1,
};
#elif (BOARD == ARDUINO_NANO_16MHZ) || (BOARD == ATMEGA328P_EXT_16MHZ)
FUSES = {
    .low      = FUSE_SUT1 & FUSE_CKSEL0,
    .high     = FUSE_SPIEN & FUSE_BOOTSZ1 & FUSE_BOOTSZ0,
    .extended = FUSE_BODLEVEL1,
};
#elif (BOARD == ATMEGA328P_EXT_8MHZ || BOARD == ATMEGA328P_EXT_4MHZ)
FUSES = {
    .low      = FUSE_SUT1 & FUSE_CKSEL1 & FUSE_CKSEL0,
    .high     = FUSE_SPIEN & FUSE_BOOTSZ1 & FUSE_BOOTSZ0,
    .extended = FUSE_BODLEVEL1,
};
#endif