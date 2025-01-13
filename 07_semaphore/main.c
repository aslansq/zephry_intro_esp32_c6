#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>
#include <stdlib.h>

#define DEFAULT_STACK_SIZE (512u)
#define DEFAULT_PRIO (6u)

#define NUM_OF_MSG_THREADS (5u)

K_THREAD_STACK_DEFINE(_stack_setup, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_0, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_1, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_2, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_3, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_4, DEFAULT_STACK_SIZE);

typedef struct {
	struct k_thread t;
	k_tid_t tid;
	struct k_sem *sem;
	k_thread_stack_t * stack;
} _thread_handle_s;

typedef struct {
	_thread_handle_s th_setup;
	_thread_handle_s th_msg[NUM_OF_MSG_THREADS];
	struct k_sem sem; // shared sem
} _app_handle_s;

static void _thread_setup_enter(void *, void *, void *);
static void _thread_msg_enter(void *, void *, void *);

// TODO: try to do this with dynamically allocated stack
static _app_handle_s _app_handle = {
	.th_msg[0].stack = _stack_0,
	.th_msg[1].stack = _stack_1,
	.th_msg[2].stack = _stack_2,
	.th_msg[3].stack = _stack_3,
	.th_msg[4].stack = _stack_4
};

int main(void) {
	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 5; ++i) {
		bsp_rgb_led_change();
		printk("%d\n", (i+1));
		k_msleep(1000);
	}

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

	uint8_t idx;
	char thread_names[NUM_OF_MSG_THREADS][2] = {"A", "B", "C", "D", "E"};
	
	k_sem_init(&app_handle->sem, 0, NUM_OF_MSG_THREADS);

	for(idx = 0; idx < NUM_OF_MSG_THREADS; ++idx) {
		// share the semaphores with other threads
		app_handle->th_msg[idx].sem = &app_handle->sem;
		// create print message threads
		app_handle->th_msg[idx].tid = k_thread_create(
			&app_handle->th_msg[idx].t,
			app_handle->th_msg[idx].stack,
			K_THREAD_STACK_SIZEOF(DEFAULT_STACK_SIZE),
			_thread_msg_enter,
			&app_handle->th_msg[idx], thread_names[idx], NULL,
			DEFAULT_PRIO,
			0,
			K_NO_WAIT
		);
	}

	for(idx = 0; idx < NUM_OF_MSG_THREADS; ) {
		if (k_sem_take(&app_handle->sem, K_NO_WAIT) == 0) {
			idx++;
		}
		k_yield();
	}
	printf("done\n");
	// erase itself
	k_thread_abort(app_handle->th_setup.tid);
}

static void _thread_msg_enter(void *void_th, void *void_msg, void *dummy3) {
	assert(!(void_th == NULL));
	assert(!(void_msg == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	assert(!(th->sem == NULL));
	assert(!(th->stack == NULL));
	char *msg = (char *)void_msg;
	printf("%s\n", msg);
	k_sem_give(th->sem);
	// erase itself
	k_thread_abort(th->tid);
}
