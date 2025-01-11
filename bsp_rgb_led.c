
#include "bsp_rgb_led.h"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define PIN_NUM 8

static const struct device *gpio_ct_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
// we need to do delay with macros because required delays are two fast
#define DELAY(x)    for(uint64_t delayIdx = 0; delayIdx < (x); ++delayIdx) {__asm__("nop");}
#define SET_PIN(st) gpio_pin_set_raw(gpio_ct_dev,PIN_NUM,(st))
// magic numbers oh well: I just look with scope to meet timing requirements
// see requirements below
// https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
#define SEND0()     SET_PIN(1);DELAY(8);SET_PIN(0);DELAY(25)
#define SEND1()     SET_PIN(1);DELAY(22);SET_PIN(0);DELAY(9)
#define RESET()     SET_PIN(0);DELAY(1500)

uint8_t bsp_rgb_led_init(void) {
	int32_t ret;

	if(!device_is_ready(gpio_ct_dev)) {
		return 1;
	}

	ret = gpio_pin_configure(
		gpio_ct_dev,
		PIN_NUM,
		GPIO_OUTPUT_ACTIVE
	);

	return ret;
}

void bsp_rgb_led_set(bsp_rgb_led_color_s *val) {
	uint32_t data = 0;
	data |= (val->g << 16);
	data |= (val->r << 8);
	data |= val->b;
	RESET();
	for(uint8_t i = 0; i < 24; ++i) {
		if(data & (0x800000)) { // check 23th bit
			SEND1();
		} else {
			SEND0();
		}
		data = data << 1;
	}
}