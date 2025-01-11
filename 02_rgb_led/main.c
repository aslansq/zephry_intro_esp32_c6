/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include "bsp_rgb_led.h"

int main(void)
{
	bsp_rgb_led_color_s val = {
		.r = 0,
		.g = 0,
		.b = 0
	};
	bsp_rgb_led_init();

	while (1) {
		val.r = 255;
		val.g = 0;
		val.b = 0;
		bsp_rgb_led_set(&val);
		k_msleep(1000);
		val.r = 0;
		val.g = 255;
		val.b = 0;
		bsp_rgb_led_set(&val);
		k_msleep(1000);
		val.r = 0;
		val.g = 0;
		val.b = 255;
		bsp_rgb_led_set(&val);
		k_msleep(1000);
	}

	return 0;
}
