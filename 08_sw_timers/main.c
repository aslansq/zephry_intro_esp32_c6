#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>

#define DEFAULT_STACK_SIZE (512u)
#define DEFAULT_PRIO (6u)

void _auto_reload_timer_handler(struct k_timer *);
void _auto_reload_work_handler(struct k_work *);
void _one_shot_timer_handler(struct k_timer *);
void _one_shot_work_handler(struct k_work *);

K_TIMER_DEFINE(_auto_reload_timer, _auto_reload_timer_handler, NULL);
K_WORK_DEFINE(_auto_reload_work, _auto_reload_work_handler);
K_TIMER_DEFINE(_one_shot_timer, _one_shot_timer_handler, NULL);
K_WORK_DEFINE(_one_shot_work, _one_shot_work_handler);

int main(void) {
	uint8_t idx;

	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 5; ++i) {
		bsp_rgb_led_change();
		printk("%d\n", (i+1));
		k_msleep(1000);
	}

	k_timer_start(&_one_shot_timer, K_SECONDS(2), K_NO_WAIT);
	k_timer_start(&_auto_reload_timer, K_SECONDS(1), K_SECONDS(1));

	while (1) {
		k_msleep(10000);
	}
	
}

void _one_shot_work_handler(struct k_work *dummy) {
	ARG_UNUSED(dummy);
	printf("one shot tm expired.\n");
}

void _one_shot_timer_handler(struct k_timer *dummy) {
	ARG_UNUSED(dummy);
	k_work_submit(&_one_shot_work);
}

void _auto_reload_work_handler(struct k_work *dummy) {
	ARG_UNUSED(dummy);
	printf("auto reload tm expired.\n");
}

void _auto_reload_timer_handler(struct k_timer *dummy) {
	ARG_UNUSED(dummy);
	k_work_submit(&_auto_reload_work);
}