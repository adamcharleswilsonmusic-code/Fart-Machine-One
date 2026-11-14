/* SPDX-License-Identifier: GPL-3.0-only */
/* hal/fm1_flash_id.h: which JEDEC IDs the firmware accepts as the FM-1's flash. An accepted ID is
 * what lets the app store to flash and take updates; with any other it runs RAM only. */
#include <stdio.h>
#include <stdint.h>
#include "fm1_flash_id.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL " __VA_ARGS__); printf("\n"); fails++; } } while (0)

int main(void)
{
    CHECK(fl_jedec_known(0x856014u), "the Puya P25Q80H of early FM-1s is not accepted");
    CHECK(fl_jedec_known(0xB36014u), "the UCUN UC25HQ80 of FM-1s sold from autumn 2026 is not accepted");
    CHECK(!fl_jedec_known(0u), "a failed ID read (0) is accepted");
    CHECK(!fl_jedec_known(0xFFFFFFu), "an absent chip (all ones) is accepted");
    CHECK(!fl_jedec_known(0x856015u), "a 2 MiB part is accepted: the driver's ranges assume 1 MiB");
    printf(fails ? "flash_id: %d FAILED\n" : "flash_id: ok\n", fails);
    return fails != 0;
}
