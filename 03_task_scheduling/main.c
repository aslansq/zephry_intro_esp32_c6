#include <stdio.h>
#include <zephyr/kernel.h>
#include <bsp.h>

#define DEFAULT_THREAD_STACK_SIZE (1024u)
// lower number, higher priority
#define LOOP_PRIO    (6u)
#define THREAD1_PRIO (5u)
#define THREAD2_PRIO (4u)

K_THREAD_STACK_DEFINE(_thread1_stack, DEFAULT_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(_thread2_stack, DEFAULT_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(_thread_loop_stack, DEFAULT_THREAD_STACK_SIZE);

struct k_thread _thread1, _thread2, _thread_loop;
k_tid_t _thread1_id = NULL, _thread2_id = NULL, _thread_loop_id = NULL;

static void _thread1_enter(void *, void *, void *);
static void _thread2_enter(void *, void *, void *);
static void _thread_loop_enter(void *, void *, void *);

int main(void)
{
	bsp_rgb_led_init();

	_thread1_id = k_thread_create(
		&_thread1,
		_thread1_stack,
		K_THREAD_STACK_SIZEOF(_thread1_stack),
		_thread1_enter,
		NULL, NULL, NULL,
		THREAD1_PRIO,
		0,
		K_NO_WAIT
	);

	k_thread_name_set(&_thread1, "1");

	_thread2_id = k_thread_create(
		&_thread2,
		_thread2_stack,
		K_THREAD_STACK_SIZEOF(_thread2_stack),
		_thread2_enter,
		NULL, NULL, NULL,
		THREAD2_PRIO,
		0,
		K_NO_WAIT
	);

	k_thread_name_set(&_thread2, "2");

	_thread_loop_id = k_thread_create(
		&_thread_loop,
		_thread_loop_stack,
		K_THREAD_STACK_SIZEOF(_thread_loop_stack),
		_thread_loop_enter,
		NULL, NULL, NULL,
		LOOP_PRIO,
		0,
		K_NO_WAIT
	);

	k_thread_name_set(&_thread_loop, "l");

	while (1) {
		k_msleep(1000);
	}

	return 0;
}

static void _slow_write_char(const char c) {
	int j;
	for(j = 0; j < 1000000; j++) {
		__asm__("nop");
	}
	putchar(c);
}

static void _slow_write_line(const char *s) {
	for (uint8_t i = 0; s[i] != '\0'; i++) {
		_slow_write_char(s[i]);
	}
	_slow_write_char('\n');
}

static void _thread1_enter(void *dummy1, void *dummy2, void *dummy3) {
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	while(1) {
		_slow_write_line("Barkadeer brig Arr booty rum.");
		k_msleep(1000);
	}
}
static void _thread2_enter(void *dummy1, void *dummy2, void *dummy3) {
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	while(1) {
		_slow_write_char('*');
		k_msleep(100);
	}
}
static void _thread_loop_enter(void *dummy1, void *dummy2, void *dummy3) {
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	uint8_t i = 0;
	while(1) {
		for(i = 0; i < 3; ++i) {
			k_thread_suspend(_thread2_id);
			k_msleep(2000);
			k_thread_resume(_thread2_id);
			k_msleep(2000);
		}

		if(_thread1_id != NULL) {
			k_thread_abort(_thread1_id);
			_thread1_id = NULL;
			printk("thread1 abort\n");
		}
	}
}
