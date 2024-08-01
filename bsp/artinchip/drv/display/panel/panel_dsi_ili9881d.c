/*
 * Copyright (c) 2023, Artinchip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "panel_com.h"
#include "panel_dsi.h"

#define RESET       "PB.9"
#define EN      	"PB.8"

static struct gpio_desc reset;
static struct gpio_desc en;

static int panel_enable(struct aic_panel *panel)
{
    int ret;

	panel_get_gpio(&en, EN);
	panel_gpio_set_value(&en, 1);
	aic_delay_ms(120);

	panel_get_gpio(&reset, RESET);
	//panel_gpio_set_value(&reset, 1);
	//aic_delay_ms(120);
	//panel_gpio_set_value(&reset, 0);
	//aic_delay_ms(120);
	panel_gpio_set_value(&reset, 1);
	aic_delay_ms(120);
	

    panel_di_enable(panel, 0);
    panel_dsi_send_perpare(panel);
  
        panel_dsi_dcs_send_seq(panel,0xFF,0x98,0x81,0x03);
        panel_dsi_dcs_send_seq(panel,0x01,0x00);
        panel_dsi_dcs_send_seq(panel,0x02,0x00);
        panel_dsi_dcs_send_seq(panel,0x03,0x73);
        panel_dsi_dcs_send_seq(panel,0x04,0x00);
        panel_dsi_dcs_send_seq(panel,0x05,0x00);
	panel_dsi_dcs_send_seq(panel,0x06,0x0C);//0x0C
	panel_dsi_dcs_send_seq(panel,0x07,0x00);
	panel_dsi_dcs_send_seq(panel,0x08,0x00);
	panel_dsi_dcs_send_seq(panel,0x09,0x01);

	 panel_dsi_dcs_send_seq(panel,0x0A,0x01);
	 panel_dsi_dcs_send_seq(panel,0x0B,0x01);
	 panel_dsi_dcs_send_seq(panel,0x0C,0x01);
	 panel_dsi_dcs_send_seq(panel,0x0D,0x01);
	 panel_dsi_dcs_send_seq(panel,0x0E,0x01);
	 panel_dsi_dcs_send_seq(panel,0x0F,0x00);
	 panel_dsi_dcs_send_seq(panel,0x10,0x00);
	 panel_dsi_dcs_send_seq(panel,0x11,0x00);
	 panel_dsi_dcs_send_seq(panel,0x12,0x00);
	 panel_dsi_dcs_send_seq(panel,0x13,0x00);
	 panel_dsi_dcs_send_seq(panel,0x14,0x00);
	 panel_dsi_dcs_send_seq(panel,0x15,0x00);//0x00

	 panel_dsi_dcs_send_seq(panel,0x16,0x00);//0x00
	 panel_dsi_dcs_send_seq(panel,0x17,0x00);
	 panel_dsi_dcs_send_seq(panel,0x18,0x00);
	 panel_dsi_dcs_send_seq(panel,0x19,0x00);
	 panel_dsi_dcs_send_seq(panel,0x1A,0x00);
	 panel_dsi_dcs_send_seq(panel,0x1B,0x00);
	 panel_dsi_dcs_send_seq(panel,0x1C,0x00);
	 panel_dsi_dcs_send_seq(panel,0x1D,0x00);
	 panel_dsi_dcs_send_seq(panel,0x1E,0x40);
	 panel_dsi_dcs_send_seq(panel,0x1F,0xC0);

	 panel_dsi_dcs_send_seq(panel,0x20,0x0A);
	 panel_dsi_dcs_send_seq(panel,0x21,0x05);
	 panel_dsi_dcs_send_seq(panel,0x22,0x0A);//0x0A
	 panel_dsi_dcs_send_seq(panel,0x23,0x00);
	 panel_dsi_dcs_send_seq(panel,0x24,0x8C);//0x8C
	 panel_dsi_dcs_send_seq(panel,0x25,0x8C);//0x8C
	 panel_dsi_dcs_send_seq(panel,0x26,0x00);
	 panel_dsi_dcs_send_seq(panel,0x27,0x00);
	 panel_dsi_dcs_send_seq(panel,0x28,0x33);
	 panel_dsi_dcs_send_seq(panel,0x29,0x03);
	 panel_dsi_dcs_send_seq(panel,0x2A,0x00);
	 panel_dsi_dcs_send_seq(panel,0x2B,0x00);

	 panel_dsi_dcs_send_seq(panel,0x2C,0x00);
	 panel_dsi_dcs_send_seq(panel,0x2D,0x00);
	 panel_dsi_dcs_send_seq(panel,0x2E,0x00);
	 panel_dsi_dcs_send_seq(panel,0x2F,0x00);
	 panel_dsi_dcs_send_seq(panel,0x30,0x00);
	 panel_dsi_dcs_send_seq(panel,0x31,0x00);
	 panel_dsi_dcs_send_seq(panel,0x32,0x00);
	 panel_dsi_dcs_send_seq(panel,0x33,0x00);
	 panel_dsi_dcs_send_seq(panel,0x34,0x00);
	 panel_dsi_dcs_send_seq(panel,0x35,0x00);

	 panel_dsi_dcs_send_seq(panel,0x36,0x00);
	 panel_dsi_dcs_send_seq(panel,0x37,0x00);
	 panel_dsi_dcs_send_seq(panel,0x38,0x00);
	 panel_dsi_dcs_send_seq(panel,0x39,0x35);
	 panel_dsi_dcs_send_seq(panel,0x3A,0x01);
	 panel_dsi_dcs_send_seq(panel,0x3B,0x40);
	 panel_dsi_dcs_send_seq(panel,0x3C,0x00);
	 panel_dsi_dcs_send_seq(panel,0x3D,0x01);
	 panel_dsi_dcs_send_seq(panel,0x3E,0x00);
	 panel_dsi_dcs_send_seq(panel,0x3F,0x00);
	 panel_dsi_dcs_send_seq(panel,0x40,0x35);//0x34
	 panel_dsi_dcs_send_seq(panel,0x41,0xA8);//0xA8
	 panel_dsi_dcs_send_seq(panel,0x42,0x00);
	 panel_dsi_dcs_send_seq(panel,0x43,0x40);
	 panel_dsi_dcs_send_seq(panel,0x44,0x3f);

	 panel_dsi_dcs_send_seq(panel,0x45,0x20);
	 panel_dsi_dcs_send_seq(panel,0x46,0x00);
	 panel_dsi_dcs_send_seq(panel,0x50,0x01);
	 panel_dsi_dcs_send_seq(panel,0x51,0x23);
	 panel_dsi_dcs_send_seq(panel,0x52,0x45);
	 panel_dsi_dcs_send_seq(panel,0x53,0x67);
	 panel_dsi_dcs_send_seq(panel,0x54,0x89);
	 panel_dsi_dcs_send_seq(panel,0x55,0xAB);
	 panel_dsi_dcs_send_seq(panel,0x56,0x01);
	 panel_dsi_dcs_send_seq(panel,0x57,0x23);
	 panel_dsi_dcs_send_seq(panel,0x58,0x45);
	 panel_dsi_dcs_send_seq(panel,0x59,0x67);	
	 panel_dsi_dcs_send_seq(panel,0x5A,0x89);	
	 panel_dsi_dcs_send_seq(panel,0x5B,0xAB);	
	 panel_dsi_dcs_send_seq(panel,0x5C,0xCD);	
	 panel_dsi_dcs_send_seq(panel,0x5D,0xEF);
	 panel_dsi_dcs_send_seq(panel,0x5E,0x11);
	 panel_dsi_dcs_send_seq(panel,0x5F,0x0C);

	 panel_dsi_dcs_send_seq(panel,0x60,0x0D);
	 panel_dsi_dcs_send_seq(panel,0x61,0x0E);
	 panel_dsi_dcs_send_seq(panel,0x62,0x0F);
	 panel_dsi_dcs_send_seq(panel,0x63,0x06);
	 panel_dsi_dcs_send_seq(panel,0x64,0x07);
	 panel_dsi_dcs_send_seq(panel,0x65,0x02);
	 panel_dsi_dcs_send_seq(panel,0x66,0x02);
	 panel_dsi_dcs_send_seq(panel,0x67,0x02);
	 panel_dsi_dcs_send_seq(panel,0x68,0x02);
	 panel_dsi_dcs_send_seq(panel,0x69,0x02);//0x07
	 panel_dsi_dcs_send_seq(panel,0x6A,0x02);
	 panel_dsi_dcs_send_seq(panel,0x6B,0x02);
	 panel_dsi_dcs_send_seq(panel,0x6C,0x02);
	 panel_dsi_dcs_send_seq(panel,0x6D,0x02);

	 panel_dsi_dcs_send_seq(panel,0x6E,0x02);
	 panel_dsi_dcs_send_seq(panel,0x6F,0x02);
	 panel_dsi_dcs_send_seq(panel,0x70,0x02);
	 panel_dsi_dcs_send_seq(panel,0x71,0x02);
	 panel_dsi_dcs_send_seq(panel,0x72,0x02);
	 panel_dsi_dcs_send_seq(panel,0x73,0x01);
	 panel_dsi_dcs_send_seq(panel,0x74,0x00);
	 panel_dsi_dcs_send_seq(panel,0x75,0x0C);
	 panel_dsi_dcs_send_seq(panel,0x76,0x0D);
	 panel_dsi_dcs_send_seq(panel,0x77,0x0E);
	 panel_dsi_dcs_send_seq(panel,0x78,0x0F);
	 panel_dsi_dcs_send_seq(panel,0x79,0x06);
	 panel_dsi_dcs_send_seq(panel,0x7A,0x07);
	 panel_dsi_dcs_send_seq(panel,0x7B,0x02);
	 panel_dsi_dcs_send_seq(panel,0x7C,0x02);

	 panel_dsi_dcs_send_seq(panel,0x7D,0x02);
	 panel_dsi_dcs_send_seq(panel,0x7E,0x02);
	 panel_dsi_dcs_send_seq(panel,0x7F,0x02);
	 panel_dsi_dcs_send_seq(panel,0x80,0x02);
	 panel_dsi_dcs_send_seq(panel,0x81,0x02);
	 panel_dsi_dcs_send_seq(panel,0x82,0x02);

	 panel_dsi_dcs_send_seq(panel,0x83,0x02);
	 panel_dsi_dcs_send_seq(panel,0x84,0x02);
	 panel_dsi_dcs_send_seq(panel,0x85,0x02);
	 panel_dsi_dcs_send_seq(panel,0x86,0x02);
	 panel_dsi_dcs_send_seq(panel,0x87,0x02);
	 panel_dsi_dcs_send_seq(panel,0x88,0x02);
	 panel_dsi_dcs_send_seq(panel,0x89,0x01);
	 panel_dsi_dcs_send_seq(panel,0x8A,0x00);//0x02

	 panel_dsi_dcs_send_seq(panel,0xFF,0x98,0x81,0x04);
	 panel_dsi_dcs_send_seq(panel,0x68,0xDB);
	 panel_dsi_dcs_send_seq(panel,0x6D,0x08);
	 panel_dsi_dcs_send_seq(panel,0x70,0x00);
	 panel_dsi_dcs_send_seq(panel,0x71,0x00);
	 panel_dsi_dcs_send_seq(panel,0x66,0x1E);
	 panel_dsi_dcs_send_seq(panel,0x3A,0x24);
	 panel_dsi_dcs_send_seq(panel,0x82,0x09);//VGH_MOD clamp level=15v
	 panel_dsi_dcs_send_seq(panel,0x84,0x09);//VGH clamp level 15V
	 panel_dsi_dcs_send_seq(panel,0x85,0x12);//VGL clamp level (-10V)
	 panel_dsi_dcs_send_seq(panel,0x32,0xAC);//
	 panel_dsi_dcs_send_seq(panel,0x8C,0x80);
	 panel_dsi_dcs_send_seq(panel,0x3C,0xF5);
	 panel_dsi_dcs_send_seq(panel,0x3A,0x24);
	 panel_dsi_dcs_send_seq(panel,0xB5,0x02);
	 panel_dsi_dcs_send_seq(panel,0x31,0x25);
	 panel_dsi_dcs_send_seq(panel,0x88,0x33);
	 panel_dsi_dcs_send_seq(panel,0x38,0x01);
	 panel_dsi_dcs_send_seq(panel,0x39,0x00);
	 
	// panel_dsi_dcs_send_seq(panel,0xFF,0x98,0x81,0x04); 
	// panel_dsi_dcs_send_seq(panel,0x2c,0xFF);
	// panel_dsi_dcs_send_seq(panel,0x2f,0x01);

	 panel_dsi_dcs_send_seq(panel,0xFF,0x98,0x81,0x01);
	 panel_dsi_dcs_send_seq(panel,0x22,0x0A);//BGR SS GS
	 panel_dsi_dcs_send_seq(panel,0x31,0x00);//column inversion
	 panel_dsi_dcs_send_seq(panel,0x50,0x5C);
	 panel_dsi_dcs_send_seq(panel,0x51,0x5C);
	 panel_dsi_dcs_send_seq(panel,0x53,0x42);//VCOM1  //42//48
	 panel_dsi_dcs_send_seq(panel,0x55,0x4A);//VCOM2  //4A//4F
	 panel_dsi_dcs_send_seq(panel,0x60,0x2B);//VREG1OUT=5V
	 panel_dsi_dcs_send_seq(panel,0x61,0x00);//VREG2OUT=-5V
	 panel_dsi_dcs_send_seq(panel,0x62,0x19);//EQT Time setting
	 panel_dsi_dcs_send_seq(panel,0x63,0x00);

	 panel_dsi_dcs_send_seq(panel,0xA0,0x00);
	 panel_dsi_dcs_send_seq(panel,0xA1,0x13);
	 panel_dsi_dcs_send_seq(panel,0xA2,0x20);
	 panel_dsi_dcs_send_seq(panel,0xA3,0x14);
	 panel_dsi_dcs_send_seq(panel,0xA4,0x15);
	 panel_dsi_dcs_send_seq(panel,0xA5,0x27);
	 panel_dsi_dcs_send_seq(panel,0xA6,0x1C);

	 panel_dsi_dcs_send_seq(panel,0xA7,0x1E);
	 panel_dsi_dcs_send_seq(panel,0xA8,0x81);
	 panel_dsi_dcs_send_seq(panel,0xA9,0x1D);
	 panel_dsi_dcs_send_seq(panel,0xAA,0x29);
	 panel_dsi_dcs_send_seq(panel,0xAB,0x77);
	 panel_dsi_dcs_send_seq(panel,0xAC,0x19);
	 panel_dsi_dcs_send_seq(panel,0xAD,0x17);
	 panel_dsi_dcs_send_seq(panel,0xAE,0x4B);
	 panel_dsi_dcs_send_seq(panel,0xAF,0x21);

	 panel_dsi_dcs_send_seq(panel,0xB0,0x27);
	 panel_dsi_dcs_send_seq(panel,0xB1,0x52);
	 panel_dsi_dcs_send_seq(panel,0xB2,0x64);
	 panel_dsi_dcs_send_seq(panel,0xB3,0x39);
	 panel_dsi_dcs_send_seq(panel,0xC0,0x00);
	 panel_dsi_dcs_send_seq(panel,0xC1,0x13);
	 panel_dsi_dcs_send_seq(panel,0xC2,0x20);
	 panel_dsi_dcs_send_seq(panel,0xC3,0x14);
	 panel_dsi_dcs_send_seq(panel,0xC4,0x15);

	 panel_dsi_dcs_send_seq(panel,0xC5,0x27);
	 panel_dsi_dcs_send_seq(panel,0xC6,0x1C);
	 panel_dsi_dcs_send_seq(panel,0xC7,0x1E);
	 panel_dsi_dcs_send_seq(panel,0xC8,0x81);
	 panel_dsi_dcs_send_seq(panel,0xC9,0x1D);
	 panel_dsi_dcs_send_seq(panel,0xCA,0x29);
	 panel_dsi_dcs_send_seq(panel,0xCB,0x77);
	 panel_dsi_dcs_send_seq(panel,0xCC,0x19);
	 panel_dsi_dcs_send_seq(panel,0xCD,0x17);
	 panel_dsi_dcs_send_seq(panel,0xCE,0x4B);
	 panel_dsi_dcs_send_seq(panel,0xCF,0x21);
	 panel_dsi_dcs_send_seq(panel,0xD0,0x27);
	 panel_dsi_dcs_send_seq(panel,0xD1,0x52);
	 panel_dsi_dcs_send_seq(panel,0xD2,0x64);
	 panel_dsi_dcs_send_seq(panel,0xD3,0x39);


	 panel_dsi_dcs_send_seq(panel,0xFF,0x98,0x81,0x00);
	 panel_dsi_dcs_send_seq(panel,0x35,0x00);
	 panel_dsi_dcs_send_seq(panel,0x36,0x00);

	 panel_dsi_dcs_send_seq(panel,0x3A,0x77);
    

    ret = panel_dsi_dcs_exit_sleep_mode(panel);
    if (ret < 0) {
        pr_err("Failed to exit sleep mode: %d\n", ret);
        return ret;
    }

    aic_delay_ms(120);

    ret = panel_dsi_dcs_set_display_on(panel);
    if (ret < 0) {
        pr_err("Failed to set display on: %d\n", ret);
        return ret;
    }

    aic_delay_ms(120);

    panel_dsi_setup_realmode(panel);

    panel_de_timing_enable(panel, 0);
    panel_backlight_enable(panel, 0);
    return 0;
}

static struct aic_panel_funcs panel_funcs = {
    .disable = panel_default_disable,
    .unprepare = panel_default_unprepare,
    .prepare = panel_default_prepare,
    .enable = panel_enable,
    .register_callback = panel_register_callback,
};

static struct display_timing ili9881d_timing = {
    .pixelclock = 80000000,
    .hactive = 720,
    .hfront_porch = 80,
    .hback_porch = 80,
    .hsync_len = 20,
    .vactive = 1280,
    .vfront_porch = 16,
    .vback_porch = 24,
    .vsync_len = 8,
};

struct panel_dsi dsi = {
    .mode = DSI_MOD_VID_BURST,
    .format = DSI_FMT_RGB888,
    .lane_num = 4,
};

struct aic_panel dsi_ili9881d = {
    .name = "panel-ili9881d",
    .timings = &ili9881d_timing,
    .funcs = &panel_funcs,
    .dsi = &dsi,
    .connector_type = AIC_MIPI_COM,
};
