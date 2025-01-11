/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define PIN_NUM 10

static const struct device *gpio_ct_dev = 
	DEVICE_DT_GET(DT_NODELABEL(gpio0));

int main(void)
{
	int32_t ret;
	uint8_t st = 0;
	int count = 0;

	if(!device_is_ready(gpio_ct_dev)) {
		printf("Fail: ready\n");
	}

	ret = gpio_pin_configure(
		gpio_ct_dev,
		PIN_NUM,
		GPIO_OUTPUT_ACTIVE
	);

	if(ret != 0) {
		printf("Fail: cfg\n");
	}

	while (1) {
		st = (st == 0) ? 1 : 0;
		ret = gpio_pin_set_raw(
			gpio_ct_dev,
			PIN_NUM,
			st
		);
		if(ret != 0) {
			printf("Fail: set\n");
		}
		printf("%d\n", count);
		k_msleep(1000);
		count++;
	}

	return 0;
}
