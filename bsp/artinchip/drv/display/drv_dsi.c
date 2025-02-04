/*
 * Copyright (c) 2023, Artinchip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <aic_core.h>
#include <aic_hal.h>
#include <aic_hal_dsi.h>
#include <mipi_display.h>

#include "disp_com.h"

struct aic_dsi_comp
{
    void *regs;
    int vc_num;
    u32 ln_assign;
    u32 ln_polrs;
    bool dc_inv;

    u64 sclk_rate;
    u64 pll_disp_rate;

    struct aic_panel *panel;
};
static struct aic_dsi_comp *g_aic_dsi_comp;

static struct aic_dsi_comp *aic_dsi_request_drvdata(void)
{
    return g_aic_dsi_comp;
}

static void aic_dsi_release_drvdata(void)
{

}

static int aic_dsi_clk_enable(void)
{
    struct aic_dsi_comp *comp = aic_dsi_request_drvdata();
    u32 pixclk = comp->panel->timings->pixelclock;

    hal_clk_set_freq(CLK_PLL_FRA2, comp->pll_disp_rate);
    hal_clk_set_rate(CLK_SCLK, comp->sclk_rate, comp->pll_disp_rate);
    hal_clk_set_rate(CLK_PIX, pixclk, comp->sclk_rate);

    hal_clk_enable(CLK_PLL_FRA2);
    hal_clk_enable(CLK_SCLK);
    hal_clk_enable(CLK_MIPIDSI);

    hal_reset_deassert(RESET_MIPIDSI);

    aic_dsi_release_drvdata();
    return 0;
}

static int aic_dsi_clk_disable(void)
{
    hal_reset_assert(RESET_MIPIDSI);

    hal_clk_disable(CLK_MIPIDSI);
    hal_clk_disable(CLK_SCLK);
    return 0;
}

static int aic_dsi_enable(void)
{
    struct aic_dsi_comp *comp = aic_dsi_request_drvdata();
    struct panel_dsi *dsi = comp->panel->dsi;
    ulong lp_rate = MIPI_DSI_LP_RATE;

    reg_set_bit(comp->regs + DSI_CTL, DSI_CTL_EN);

    dsi_set_lane_assign(comp->regs, comp->ln_assign);
    dsi_set_lane_polrs(comp->regs, comp->ln_polrs);
    dsi_set_data_clk_polrs(comp->regs, comp->dc_inv);

    if (lp_rate < 10000000 || lp_rate > 20000000) {
        lp_rate = 10000000;
        pr_warn("Invalid lp rate, use default lp rate: %ld\n", lp_rate);
    }

    dsi_set_clk_div(comp->regs, comp->sclk_rate, lp_rate);
    dsi_pkg_init(comp->regs);
    dsi_phy_init(comp->regs, comp->sclk_rate, dsi->lane_num);
    dsi_hs_clk(comp->regs, 1);

    aic_dsi_release_drvdata();
    return 0;
}

static int aic_dsi_disable(void)
{
    return 0;
}

static int aic_dsi_attach_panel(struct aic_panel *panel)
{
    struct aic_dsi_comp *comp = aic_dsi_request_drvdata();
    u32 div[DSI_MAX_LANE_NUM] = {24, 24, 18, 16};
    u32 pixclk = panel->timings->pixelclock;
    struct panel_dsi *dsi = panel->dsi;
    u64 pll_disp_rate = 0;
    int i = 0;

    comp->panel = panel;

    if (dsi->lane_num <= DSI_MAX_LANE_NUM)
    {
        comp->sclk_rate = pixclk * div[dsi->format] / dsi->lane_num;
    }
    else
    {
        pr_err("Invalid lane number %d\n", dsi->lane_num);
        return -EINVAL;
    }

    pll_disp_rate = comp->sclk_rate;
    while (pll_disp_rate < PLL_DISP_FREQ_MIN)
    {
        pll_disp_rate = comp->sclk_rate * (2 << i);
        i++;
    }
    comp->pll_disp_rate = pll_disp_rate;

    if (dsi->ln_assign)
        comp->ln_assign = dsi->ln_assign;
    else
        comp->ln_assign = LANE_ASSIGNMENTS;

    if (dsi->ln_polrs)
        comp->ln_polrs = dsi->ln_polrs;
    else
        comp->ln_polrs = LANE_POLARITIES;

    if (dsi->dc_inv)
        comp->dc_inv = dsi->dc_inv;
    else
        comp->dc_inv = CLK_INVERSE;

    aic_dsi_release_drvdata();
    return 0;
}

static int aic_dsi_set_vm(const struct display_timing *timing, int enable)
{
    struct aic_dsi_comp *comp = aic_dsi_request_drvdata();
    struct panel_dsi *dsi = comp->panel->dsi;

    if (enable) {
        dsi_dcs_lw(comp->regs, false);
        dsi_set_vm(comp->regs, dsi->mode, dsi->format,
            dsi->lane_num, comp->vc_num, timing);
    } else {
        dsi_set_vm(comp->regs, DSI_MOD_CMD_MODE, dsi->format,
            dsi->lane_num, comp->vc_num, timing);
        dsi_dcs_lw(comp->regs, true);
#if DCS_GET_DISPLAY_ID
        dsi_cmd_wr(comp->regs, MIPI_DSI_DCS_READ, 0,
                (u8[]){ MIPI_DCS_GET_DISPLAY_ID }, 1);
        aic_delay_ms(120);

        pr_info("mipi-dsi screen id: %x\n", readl(comp->regs + DSI_GEN_PD_CFG));
#endif
    }

    aic_dsi_release_drvdata();
    return 0;
}

static void aic_dsi_send_debug_cmd(struct aic_dsi_comp *comp)
{
    struct dsi_command *commands = &comp->panel->dsi->command;
    unsigned int i = 0;

    aic_dsi_set_vm(comp->panel->timings, false);
    while (i < commands->len) {
        u8 command = commands->buf[i++];
        u8 num_parameters = commands->buf[i++];
        const u8 *parameters = &commands->buf[i];

        if (command == 0x00 && num_parameters == 1)
            aic_delay_ms(parameters[0]);
        else
            dsi_cmd_wr(comp->regs, command, comp->vc_num, parameters, num_parameters);

        i += num_parameters;
    }
    aic_dsi_set_vm(comp->panel->timings, true);
}

static int aic_dsi_send_cmd(u32 dt, u32 vc, const u8 *data, u32 len)
{
    struct aic_dsi_comp *comp = aic_dsi_request_drvdata();
    int command_on = comp->panel->dsi->command.command_on;

    if (command_on) {
        aic_dsi_send_debug_cmd(comp);
        return 0;
    }

    dsi_cmd_wr(comp->regs, dt, vc, data, len);
    aic_dsi_release_drvdata();
    return 0;
}

static int aic_dsi_read_cmd(u32 val)
{
    struct aic_dsi_comp *comp = aic_dsi_request_drvdata();

    dsi_cmd_wr(comp->regs, MIPI_DSI_DCS_READ, 0, (u8[]){ val }, 1);
    aic_delay_ms(120);

    return readl(comp->regs + DSI_GEN_PD_CFG);
}

struct di_funcs aic_dsi_func = {
    .clk_enable = aic_dsi_clk_enable,
    .clk_disable = aic_dsi_clk_disable,
    .enable = aic_dsi_enable,
    .disable = aic_dsi_disable,
    .attach_panel = aic_dsi_attach_panel,
    .set_videomode = aic_dsi_set_vm,
    .send_cmd = aic_dsi_send_cmd,
    .read_cmd = aic_dsi_read_cmd,
};

static int aic_dsi_probe(void)
{
    struct aic_dsi_comp *comp;

    comp = aicos_malloc(0, sizeof(*comp));
    if (!comp)
    {
        pr_err("allloc dsi comp failed\n");
        return -ENOMEM;
    }

    memset(comp, 0, sizeof(*comp));

    comp->regs     = (void *)MIPI_DSI_BASE;
    comp->vc_num   = VIRTUAL_CHANNEL;
    g_aic_dsi_comp = comp;

    return 0;
}

static void aic_dsi_remove(void)
{

}

struct platform_driver artinchip_dsi_driver = {
    .name = "artinchip-dsi",
    .component_type = AIC_MIPI_COM,
    .probe = aic_dsi_probe,
    .remove = aic_dsi_remove,
    .di_funcs = &aic_dsi_func,
};

