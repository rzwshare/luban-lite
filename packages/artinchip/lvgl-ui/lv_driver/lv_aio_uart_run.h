/*
 * Copyright (c) 2022-2023, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef LV_TPC_RUN
#define LV_TPC_RUN

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#if 1


typedef struct
{
	int photovoltaic_panel_power_generation;//0 Photovoltaic panel power generation 
	int rated_power_generation_value_of_photovoltaic_panels;//1 Rated power generation value of photovoltaic panels
	int photovoltaic_panel_power_generation_power_return_value;//2 Photovoltaic panel power generation power return value
	int photovoltaic_panels_generate_electricity;//3 Photovoltaic panels generate electricity
	int draw_power_from_the_grid;//4 Draw power from the grid
	
	int sell_electricity_to_the_grid;//5 Sell electricity to the grid
	int rated_power_of_the_grid_to_take_or_sell_electricity;//6 The rated power of the grid to take/sell electricity
	int grid_power_take_or_sell_power_return_variance;//7 Grid power take/sell power return variance
	int electricity_is_taken_from_the_grid;//8 Electricity is taken from the grid on the same day
	int sell_electricity_to_the_grid_on_the_same_day ;//9 Sell electricity to the grid on the same day

	
	int tram_charging_power;//10 Tram charging power
	int rated_charging_power_value_of_the_tram;//11 The rated charging power value of the tram
	int tram_charging_power_return_difference;//12 Tram charging power return difference
	int amount_of_charging_the_tram;//13 The amount of charging the tram on the same day
	int electricity_power_for_household_loads;//14 Electricity power for household loads

	int rated_power_value_of_the_household_load;//15 The rated power value of the household load
	
	int power_return_value_of_household_load_power;//16 The power return value of household load power return
	int household_load_electricity_consumption_per_day;//17 Household load electricity consumption per day
	int household_loads_are_rated_for_daily_electricity_consumption;//18 Household loads are rated for daily electricity consumption

	int soc_5ko_smart_system;//19 5K0 Smart System SOC

	
	
	int bt_state;//bt_state
	
	int wifi_state;//wifi_state
	int system_fault_state;//system_fault_state
	int system_alarm_state;//system_alarm_state

	int panel_state;//panel_state
	
	int grid_state;//grid_state
	int auto_state;//auto_state
	int home_state;//home_state
	
	
	int system_alarm_nums;
	int system_fault_nums;

	char system_alarms[2048];
	char system_faults[2048];

	short dataisvalid ;
	
		
} AioEventDisplay;



#endif

#include <rtconfig.h>

#ifdef KERNEL_RTTHREAD
#include <rtthread.h>
#include <rtdevice.h>

int aio_uart_run(const char *name);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_TPC_RUN */
