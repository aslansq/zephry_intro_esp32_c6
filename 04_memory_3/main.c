#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include "bsp.h"

#define DEFAULT_STACK_SIZE (1024u)
#define DEFAULT_PRIO (6u)

K_THREAD_STACK_DEFINE(_thread_a_stack,  DEFAULT_STACK_SIZE);

static struct k_thread _thread_a;
static k_tid_t _thread_a_id = NULL;

static void _thread_a_enter(void *, void *, void *);

int main(void)
{
	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 10; ++i) {
		printk("s %d\r\n", (int)i);
		k_msleep(1000);
		bsp_rgb_led_change();
	}

	_thread_a_id = k_thread_create(
		&_thread_a,
		_thread_a_stack,
		K_THREAD_STACK_SIZEOF(_thread_a_stack),
		_thread_a_enter,
		NULL, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	while (1) {
		k_msleep(10000);
	}

	return 0;
}

// demo 3 : ovf heap u32 at a time
static void _thread_a_enter(void *dummy1, void *dummy2, void *dummy3) {
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	int32_t count = 0;

	while(1) {
		// lets ovf heap u32 at a time
		uint32_t *p = k_malloc(sizeof(uint32_t));
		printf("%d\n", count++);
	}
}
