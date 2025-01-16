#ifndef BSP_H
#define BSP_H

#include <stdint.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} bsp_rgb_led_color_s;

#define BSP_IS_ASCII(c) (((c) >= 0 && c <= 127) ? 1 : 0)

uint8_t bsp_rgb_led_init(void);
void bsp_rgb_led_set(bsp_rgb_led_color_s *val);
void bsp_rgb_led_change(void);
char bsp_getchar(void);
int16_t bsp_getline(char *buf, uint8_t size);

#endif // BSP_H

