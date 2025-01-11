#ifndef BSP_RGB_LED_H
#define BSP_RGB_LED_H

#include <stdint.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} bsp_rgb_led_color_s;

uint8_t bsp_rgb_led_init(void);
void bsp_rgb_led_set(bsp_rgb_led_color_s *val);

#endif // BSP_RGB_LED_H

