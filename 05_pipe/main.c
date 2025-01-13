#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>

#define DEFAULT_STACK_SIZE (1024u)
#define DEFAULT_PRIO (6u)

#define PRINT_MSG_DELAY (1000)
#define SEND_MSG_DELAY  (100)

#define PIPE_BUF_NUM_BYTES (10 * sizeof(uint32_t)) // 10 uint32

K_THREAD_STACK_DEFINE(_stack_print,  DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_send,  DEFAULT_STACK_SIZE);

typedef struct {
	struct k_thread t;
	k_tid_t tid;
	struct k_pipe *pipe;
} _thread_handle_s;

typedef struct {
	_thread_handle_s th_print; // thread handle print
	_thread_handle_s th_send;
	uint8_t __aligned(4) pipe_buf[PIPE_BUF_NUM_BYTES];
	struct k_pipe pipe; // shared pipe
} _app_handle_s;

static _app_handle_s _app_handle = {
	.th_print.pipe = &_app_handle.pipe,
	.th_send.pipe = &_app_handle.pipe
};

static void _thread_print_msgs_enter(void *, void *, void *);
static void _thread_send_msg_enter(void *, void *, void *);

int main(void) {
	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 10; ++i) {
		bsp_rgb_led_change();
		printk("%d\n", (i+1));
		k_msleep(1000);
	}

	k_pipe_init(
		&_app_handle.pipe,
		(unsigned char *)_app_handle.pipe_buf,
		PIPE_BUF_NUM_BYTES
	);

	_app_handle.th_print.tid = k_thread_create(
		&_app_handle.th_print.t,
		_stack_print,
		K_THREAD_STACK_SIZEOF(_stack_print),
		_thread_print_msgs_enter,
		&_app_handle.th_print, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	_app_handle.th_send.tid = k_thread_create(
		&_app_handle.th_send.t,
		_stack_send,
		K_THREAD_STACK_SIZEOF(_stack_send),
		_thread_send_msg_enter,
		&_app_handle.th_send, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	while (1) {
		k_msleep(10000);
	}
	
}

static void _thread_print_msgs_enter(void *void_th, void *dummy2, void *dummy3) {
	assert(!(void_th == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	ARG_UNUSED(dummy3);
	ARG_UNUSED(dummy3);
	uint32_t num;
	int ret;
	size_t read_num_bytes;
	while(1) {
		ret = k_pipe_get(
			th->pipe,
			&num,
			sizeof(uint32_t),
			&read_num_bytes,
			sizeof(uint32_t),
			K_MSEC(10)
		);
		if(ret == 0) {
			printk("\nread %d", num);
		} else if(ret == -EAGAIN) {
			printk("\nread TO");
		} else {
			printk("\nread fail");
		}
		k_msleep(PRINT_MSG_DELAY);
	}
}

static void _thread_send_msg_enter(void *void_th, void *dummy2, void *dummy3) {
	assert(!(void_th == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	ARG_UNUSED(dummy3);
	ARG_UNUSED(dummy3);
	uint32_t num = 0;
	int ret;
	size_t written_num_bytes;
	while(1) {
		ret = k_pipe_put(
			th->pipe,
			&num,
			sizeof(uint32_t),
			&written_num_bytes,
			sizeof(uint32_t),
			K_MSEC(10)
		);
		if(ret == 0) {
			printk("\nsent");
		} else if(ret == -EAGAIN) {
			printk("\nsend TO");
		} else {
			printk("\nsend fail");
		}
		printk(" %d", num);
		num++;
		k_msleep(SEND_MSG_DELAY);
	}
}