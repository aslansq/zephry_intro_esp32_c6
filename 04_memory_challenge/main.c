#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>
#include <string.h>

#define DEFAULT_STACK_SIZE (1024u)
#define THREAD_A_PRIO (5u)
#define THREAD_B_PRIO (5u)

K_THREAD_STACK_DEFINE(_thread_a_stack,  DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_thread_b_stack,  DEFAULT_STACK_SIZE);

typedef struct {
	void *stack;
	struct k_thread t; // thread
	k_tid_t tid;
	uint8_t *msgRdy;
	char **msg;
} _thread_handle_s;

typedef struct {
	_thread_handle_s a;
	_thread_handle_s b;
	char *msg; // shared resource between threads
	char msgRdy; // shared resource between threads
} _app_handle_s;

static void _thread_a_enter(void *, void *, void *);
static void _thread_b_enter(void *, void *, void *);

// architecture decision. instead of having a lot of global variable
// there is only one in a struct
_app_handle_s _app_handle = {
	.a = {
		.stack = _thread_a_stack,
		.tid = NULL,
		.msgRdy = &_app_handle.msgRdy,
		.msg = &_app_handle.msg
	},
	.b = {
		.stack = _thread_b_stack,
		.tid = NULL,
		.msgRdy = &_app_handle.msgRdy,
		.msg = &_app_handle.msg
	},
	.msg = NULL,
	.msgRdy = 0
};


int main(void)
{
	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 10; ++i) {
		printk("s %d\r\n", (int)i);
		k_msleep(1000);
		bsp_rgb_led_change();
	}

	_app_handle.a.tid = k_thread_create(
		&_app_handle.a.t,
		_app_handle.a.stack,
		K_THREAD_STACK_SIZEOF(_app_handle.a.stack),
		_thread_a_enter,
		&_app_handle.a, NULL, NULL,
		THREAD_A_PRIO,
		0,
		K_NO_WAIT
	);

	_app_handle.b.tid = k_thread_create(
		&_app_handle.b.t,
		_app_handle.b.stack,
		K_THREAD_STACK_SIZEOF(_app_handle.b.stack),
		_thread_b_enter,
		&_app_handle.b, NULL, NULL,
		THREAD_B_PRIO,
		0,
		K_NO_WAIT
	);

	while (1) {
		k_msleep(10000);
	}

	return 0;
}

static void _thread_a_enter(void *void_handle, void *dummy2, void *dummy3) {
	assert(!(void_handle == NULL));
	_thread_handle_s *t_handle = (_thread_handle_s *) void_handle;
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	static char buf[100];
	int16_t read;
	while(1) {
		printf("\nta in:");
		read = bsp_getline(buf,100);
		if(read > 0) {
			(*t_handle->msg) = k_malloc(sizeof(char)*read);
			memcpy((*t_handle->msg), buf, sizeof(char)*read);
			printf("\nta buf: %s", buf);
			printf("\nta handle msg: %s", (*t_handle->msg));
			*t_handle->msgRdy = 1;
		}
		k_yield();
	}
}

static void _thread_b_enter(void *void_handle, void *dummy2, void *dummy3) {
	assert(!(void_handle == NULL));
	_thread_handle_s *t_handle = (_thread_handle_s *) void_handle;
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	while(1) {
		if(*t_handle->msgRdy && (*t_handle->msg) != NULL) {
			printf("\ntb: %s", (*t_handle->msg));
			k_free((*t_handle->msg));
			(*t_handle->msg) = NULL;
			*t_handle->msgRdy = 0;
		}
		k_yield();
	}
}
