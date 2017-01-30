/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2017 Amaury Pouly
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#ifndef __BLACKFIN_BOOT__
#define __BLACKFIN_BOOT__

#include <stdint.h>

struct bfin_boot_header_t
{
    uint32_t code;
    uint32_t target;
    uint32_t count;
    uint32_t arg;
} __attribute__((packed));

#define BCODE(x)            ((x) & 0xf)
#define BFLAG(x)            (((x) >> 4) & 0xfff)
#define BFLAG_SAVE          (1 << 4)
#define BFLAG_AUX           (1 << 5)
#define BFLAG_FORWARD       (1 << 7)
#define BFLAG_FILL          (1 << 8)
#define BFLAG_QUICKBOOT     (1 << 9)
#define BFLAG_CALLBACK      (1 << 10)
#define BFLAG_INIT          (1 << 11)
#define BFLAG_IGNORE        (1 << 12)
#define BFLAG_INDIRECT      (1 << 13)
#define BFLAG_FIRST         (1 << 14)
#define BFLAG_FINAL         (1 << 15)
#define HDRSIGN(x)          (((x) >> 24) & 0xff)
#define HDRSIGN_BF609       0xad

#endif /* __BLACKFIN_BOOT__ */
