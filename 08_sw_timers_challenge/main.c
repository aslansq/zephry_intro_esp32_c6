#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>

#define DEFAULT_STACK_SIZE (512u)
#define DEFAULT_PRIO (6u)

static void _back_light_timer_handler(struct k_timer *);
static void _back_light_work_handler(struct k_work *);
static void _user_io_thread_enter(void *, void *, void *);

K_THREAD_STACK_DEFINE(_user_io_thread_stack,  DEFAULT_STACK_SIZE);
K_TIMER_DEFINE(_back_light_timer, _back_light_timer_handler, NULL);
K_WORK_DEFINE(_back_light_work, _back_light_work_handler);

bsp_rgb_led_color_s _back_light_white = {
	.r = 255u,
	.g = 255u,
	.b = 255u
};

bsp_rgb_led_color_s _back_light_off = {
	.r = 0u,
	.g = 0u,
	.b = 0u
};

struct k_thread _user_io_thread;
k_tid_t _user_io_thread_id = NULL;

int main(void) {
	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 5; ++i) {
		bsp_rgb_led_change();
		printk("%d\n", (i+1));
		k_msleep(1000);
	}

	_user_io_thread_id = k_thread_create(
		&_user_io_thread,
		_user_io_thread_stack,
		K_THREAD_STACK_SIZEOF(_user_io_thread_stack),
		_user_io_thread_enter,
		NULL, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	bsp_rgb_led_set(&_back_light_off);

	while (1) {
		k_msleep(10000);
	}
	
}

static void _user_io_thread_enter(void *dummy1, void *dummy2, void *dummy3) {
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	char c;
	for( ; ; ) {
		c = bsp_getchar();
		if(BSP_IS_ASCII(c)) {
			putchar(c);
			bsp_rgb_led_set(&_back_light_white); // turn on backlight
			k_timer_start(&_back_light_timer, K_SECONDS(5), K_NO_WAIT); // inactivity timer
		}
		k_yield();
	}
}

static void _back_light_work_handler(struct k_work *dummy) {
	ARG_UNUSED(dummy);
	bsp_rgb_led_set(&_back_light_off);
}

static void _back_light_timer_handler(struct k_timer *dummy) {
	ARG_UNUSED(dummy);
	k_work_submit(&_back_light_work);
}
