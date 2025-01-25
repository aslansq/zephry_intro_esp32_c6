#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>

#define DEFAULT_STACK_SIZE (512u)
#define DEFAULT_PRIO (6u)

static void _print_val_thread_enter(void *, void *, void *);

K_THREAD_STACK_DEFINE(_print_val_thread_stack,  DEFAULT_STACK_SIZE);

struct k_thread _print_val_thread;
k_tid_t _print_val_thread_id = NULL;
static volatile uint32_t _isrCounter = 0;

int main(void) {
	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 5; ++i) {
		bsp_rgb_led_change();
		printk("%d\n", (i+1));
		k_msleep(1000);
	}
	bsp_tm1_int_setup();

	_print_val_thread_id = k_thread_create(
		&_print_val_thread,
		_print_val_thread_stack,
		K_THREAD_STACK_SIZEOF(_print_val_thread_stack),
		_print_val_thread_enter,
		NULL, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	while (1) {
		k_sleep(K_FOREVER);
	}
	
}

void timer1_handler(void) {
	unsigned int key = irq_lock();
	_isrCounter++;
	irq_unlock(key);
}

static void _print_val_thread_enter(void *dummy1, void *dummy2, void *dummy3) {
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	for( ; ; ) {
		while(_isrCounter > 0) {
			k_msleep(1300);
			printk("%d\n", _isrCounter);
			unsigned int key = irq_lock();
			_isrCounter--;
			irq_unlock(key);
		}
		printf("\r\n");
	}
}
