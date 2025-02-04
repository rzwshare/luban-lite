/*
 * Copyright (c) 2023, Artinchip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Wu Dehuang <dehuang.wu@artinchip.com>
 */

#include <rtconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <console.h>
#include <aic_common.h>
#include <aic_errno.h>
#include <spinand_port.h>
#include <mtd.h>
#include <image.h>
#include <boot.h>
#include <aic_utils.h>
#include "fitimage.h"

#define APPLICATION_PART "os"
#define PAGE_MAX_SIZE    4096

#ifdef AIC_AB_SYSTEM_INTERFACE
#include <absystem.h>

char target[32] = { 0 };
#endif

static int do_nand_boot(int argc, char *argv[])
{
    int ret = 0;
    struct mtd_dev *mtd;
    ulong entry_point;
    struct spl_load_info info;

    mtd_probe();

#ifdef AIC_AB_SYSTEM_INTERFACE
    ret = aic_ota_check();
    if (ret) {
        printf("Aic ota check error.\n");
    }

    ret = aic_get_os_to_startup(target);
    if (ret) {
        printf("Aic get os fail, startup from %s default.\n", APPLICATION_PART);
        mtd = mtd_get_device(APPLICATION_PART);
    } else {
        mtd = mtd_get_device(target);
        printf("Start-up from %s\n", target);
    }
#else
    mtd = mtd_get_device(APPLICATION_PART);
#endif

    if (!mtd) {
        printf("Failed to get application partition.\n");
        return -1;
    }

    info.dev = (void *)mtd;
    info.bl_len = mtd->writesize;
    info.dev_type = DEVICE_SPINAND;

    ret = spl_load_simple_fit(&info, &entry_point);
    if (ret < 0)
        goto out;

    boot_app((void *)entry_point);

out:
    return ret;
}

CONSOLE_CMD(nand_boot, do_nand_boot, "Boot from NAND.");
