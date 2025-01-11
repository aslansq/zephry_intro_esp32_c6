/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>

int main(void)
{
	while (1) {
		printf("Hello World! %s\n", CONFIG_BOARD_TARGET);
		k_msleep(1000);
	}

	return 0;
}
