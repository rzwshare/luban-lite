/*
 * Copyright (C) 2022-2023 ArtinChip Technology Co., Ltd.
 * Authors:  Ning Fang <ning.fang@artinchip.com>
 */

#include "lvgl.h"
#include "aic_ui.h"
#include "aic_osal.h"
#ifdef AIC_LVGL_SDBATTERY
#include "sdbattery_ui.h"
#endif

#ifdef AIC_LVGL_AIOBATTERY
#include "aiobattery_ui.h"
#endif
// #define AIC_LVGL_QC_TEST_DEMO

void aic_ui_init()
{
/* qc test demo is only for aic internal qc testing, please ignore it. */
#ifdef AIC_LVGL_QC_TEST_DEMO
    extern void qc_test_init();
    qc_test_init();
    return;
#endif

#ifdef AIC_LVGL_BASE_DEMO
#include "base_ui.h"
    base_ui_init();
#endif

#ifdef AIC_LVGL_METER_DEMO
#include "meter_ui.h"
    meter_ui_init();
#endif

#ifdef AIC_LVGL_LAUNCHER_DEMO
    extern void launcher_ui_init();
    launcher_ui_init();
#endif

#ifdef AIC_LVGL_MUSIC_DEMO
    extern void lv_demo_music(void);
    lv_demo_music();
#endif

#ifdef AIC_LVGL_DASHBOARD_DEMO
    extern void dashboard_ui_init(void);
    dashboard_ui_init();
#endif
#ifdef AIC_LVGL_SDBATTERY
//#include "sdbattery_ui.h"
    sdbattery_ui_init();
#endif

#ifdef AIC_LVGL_AIOBATTERY
//#include "aiobattery_ui.h"
	aiobattery_ui_init();
#endif
    return;
}
