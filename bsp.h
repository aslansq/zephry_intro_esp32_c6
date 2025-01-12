#ifndef BSP_H
#define BSP_H

#include <stdint.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} bsp_rgb_led_color_s;

uint8_t bsp_rgb_led_init(void);
void bsp_rgb_led_set(bsp_rgb_led_color_s *val);
void bsp_rgb_led_change(void);
char bsp_getchar(void);
int16_t bsp_getline(char *buf, uint8_t size);

#endif // BSP_H

