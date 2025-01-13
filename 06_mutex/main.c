#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>

#define DEFAULT_STACK_SIZE (1024u)
#define DEFAULT_PRIO (6u)

K_THREAD_STACK_DEFINE(_stack_a,  DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_b,  DEFAULT_STACK_SIZE);

typedef struct {
	struct k_thread t;
	k_tid_t tid;
	int *var;
	struct k_mutex *mutex;
} _thread_handle_s;

typedef struct {
	_thread_handle_s th_a;
	_thread_handle_s th_b;
	int var; // shared var
	struct k_mutex mutex; // shared mutex
} _app_handle_s;

static void _thread_inc_enter(void *, void *, void *);

static _app_handle_s _app_handle = {
	.th_a.var = &_app_handle.var,
	.th_a.mutex = &_app_handle.mutex,
	.th_b.var = &_app_handle.var,
	.th_b.mutex = &_app_handle.mutex
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

	_app_handle.th_a.tid = k_thread_create(
		&_app_handle.th_a.t,
		_stack_a,
		K_THREAD_STACK_SIZEOF(_stack_a),
		_thread_inc_enter,
		&_app_handle.th_a, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	_app_handle.th_b.tid = k_thread_create(
		&_app_handle.th_b.t,
		_stack_b,
		K_THREAD_STACK_SIZEOF(_stack_b),
		_thread_inc_enter,
		&_app_handle.th_b, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	while (1) {
		k_msleep(10000);
	}
	
}

// ported from freertos code maybe calling random function in zephyr is okay
// but it is not important for demo purposes
static int _my_rand(void) {
	static int ran_dur_of_op_lut[] = {973, 594, 28, 651, 73, 680, 452, 504}; // random duration of operation look up table
	static int ran_idx = 0;
	ran_idx++;
	ran_idx = ran_idx < 8 ? ran_idx : 0;
	// looks random enough, this particular environment is not happy calling rand function from stdlib
	return ran_dur_of_op_lut[ran_idx % 8];
}

static void _thread_inc_enter(void *void_th, void *dummy2, void *dummy3) {
	assert(!(void_th == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	int local_var = 0;
	int ran_dur_of_op = 0;
	for( ; ; ) {
		if (k_mutex_lock(th->mutex, K_MSEC(10)) == 0) {
			// this is simulating non-atomic increase operation
			// intentionally waiting for huge time in read and write operations to make race condition occur
			ran_dur_of_op = _my_rand();
			local_var = *th->var;
			local_var++;
			k_msleep(ran_dur_of_op);
			*th->var = local_var;
			printf("%d\n", *th->var);
			k_mutex_unlock(th->mutex);
		}
		k_yield();
	}
}