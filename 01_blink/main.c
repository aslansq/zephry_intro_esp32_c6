#include <stdint.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// see esp32c6_devkitc.overlay for pin definition
static const struct gpio_dt_spec led = 
	GPIO_DT_SPEC_GET(DT_NODELABEL(blinking_led), gpios);

int main(void)
{
	int ret;
	int count = 0;

	if(!device_is_ready(led.port)) {
		printf("Fail: ready\n");
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

	if(ret != 0) {
		printf("Fail: cfg\n");
	}

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if(ret != 0) {
			printf("Fail: toggle\n");
		}
		printf("%d\n", count);
		k_msleep(1000);
		count++;
	}

	return 0;
}
