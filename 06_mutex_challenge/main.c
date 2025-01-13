#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>
#include <stdlib.h>

#define DEFAULT_STACK_SIZE (1024u)
#define DEFAULT_PRIO (6u)

K_THREAD_STACK_DEFINE(_stack_setup,  DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_blink,  DEFAULT_STACK_SIZE);

typedef struct {
	struct k_thread t;
	k_tid_t tid;
	int *var;
	struct k_mutex *mutex;
} _thread_handle_s;

typedef struct {
	_thread_handle_s th_setup;
	_thread_handle_s th_blink;
	struct k_mutex mutex; // shared mutex
} _app_handle_s;

static void _thread_setup_enter(void *, void *, void *);
static void _thread_blink_enter(void *, void *, void *);

static _app_handle_s _app_handle = {
	.th_setup.mutex = &_app_handle.mutex,
	.th_blink.mutex = &_app_handle.mutex
};

int main(void) {
	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 5; ++i) {
		bsp_rgb_led_change();
		printk("%d\n", (i+1));
		k_msleep(1000);
	}

	k_mutex_init(&_app_handle.mutex);

	_app_handle.th_setup.tid = k_thread_create(
		&_app_handle.th_setup.t,
		_stack_setup,
		K_THREAD_STACK_SIZEOF(_stack_setup),
		_thread_setup_enter,
		&_app_handle, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);



	while (1) {
		k_msleep(10000);
	}
	
}

// this thread is special it gets access to all app handle
static void _thread_setup_enter(void *void_app_handle, void *dummy2, void *dummy3) {
	assert(!(void_app_handle == NULL));
	_app_handle_s *app_handle = (_app_handle_s *) void_app_handle;
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	// get delay value from user
	char buf[100];
	int32_t delay = 1000;
	printf("Enter delay:");
	bsp_getline(buf, sizeof(buf));
	delay = atoi(buf);
	printf("\nDelay set to %d\n", (int)delay);

	k_mutex_lock(app_handle->th_setup.mutex, K_FOREVER);

	app_handle->th_blink.tid = k_thread_create(
		&app_handle->th_blink.t,
		_stack_blink,
		K_THREAD_STACK_SIZEOF(_stack_blink),
		_thread_blink_enter,
		&app_handle->th_blink, &delay, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	// wait until blink releases the lock
	k_mutex_lock(app_handle->th_setup.mutex, K_FOREVER);

	// erase itself
	k_thread_abort(app_handle->th_setup.tid);
}

static void _thread_blink_enter(void *void_th, void *void_delay, void *dummy3) {
	assert(!(void_th == NULL));
	assert(!(void_delay == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	int32_t delay = *((int32_t *)void_delay);
	ARG_UNUSED(dummy3);
	k_mutex_unlock(th->mutex);
	for( ; ; ) {
		bsp_rgb_led_change();
		k_msleep(delay);
	}
}
