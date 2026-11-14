/* SPDX-License-Identifier: GPL-3.0-only */
/* The JEDEC IDs of the 1 MiB SPI NOR parts fitted to the FM-1. The app stores to flash and takes
 * updates only on a part this accepts. No SFRs here, so a host test can include it. */
#pragma once
#include <stdint.h>

#define FL_JEDEC_P25Q80H  0x856014u                /* Puya */
#define FL_JEDEC_UC25HQ80 0xB36014u                /* UCUN */

static inline int fl_jedec_known(uint32_t id)
{
    return id == FL_JEDEC_P25Q80H || id == FL_JEDEC_UC25HQ80;
}
