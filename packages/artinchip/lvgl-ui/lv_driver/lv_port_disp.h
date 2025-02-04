/*
 * Copyright (c) 2022-2023, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

void lv_port_disp_init(void);

void lv_port_disp_exit(void);

int fbdev_draw_fps();

int disp_is_swap(void);
void lv_uart(void);
void lv_touchkey(void);
void lv_watchdog(void);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_PORT_DISP_H*/
