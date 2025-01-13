#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>

#define DEFAULT_STACK_SIZE (1024u)
#define DEFAULT_PRIO (6u)

#define MSG_BUF_SIZE (100u)
#define USER_INPUT_BUF_SIZE (100u)

#define BLINKED_MSG_EVERY_NTH (100u)

#define PIPE_BUF_NUM_BYTES (10 * sizeof(uint32_t)) // 10 uint32

K_THREAD_STACK_DEFINE(_stack_a,  DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_b,  DEFAULT_STACK_SIZE);

typedef uint16_t delay_t;


typedef struct {
	struct k_thread t;
	k_tid_t tid;
	struct k_pipe *produce_pipe;
	struct k_pipe *consume_pipe;
} _thread_handle_s;

typedef struct {
	_thread_handle_s th_a; // thread handle print
	_thread_handle_s th_b;
	uint8_t __aligned(4) pipe1_buf[PIPE_BUF_NUM_BYTES];
	struct k_pipe pipe1; // shared pipe
	uint8_t __aligned(4) pipe2_buf[PIPE_BUF_NUM_BYTES];
	struct k_pipe pipe2; // shared pipe
} _app_handle_s;

static _app_handle_s _app_handle = {
	.th_a = {
		.produce_pipe = &_app_handle.pipe1,
		.consume_pipe = &_app_handle.pipe2
	},
	.th_b = {
		.produce_pipe = &_app_handle.pipe2,
		.consume_pipe = &_app_handle.pipe1
	}
};

static void _thread_a_print_msgs(_thread_handle_s *th);
static void _thread_a_handle_user_in(_thread_handle_s *th);
static void _thread_a_enter(void *, void *, void *);
static void _thread_b_enter(void *, void *, void *);

int main(void) {
	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 5; ++i) {
		bsp_rgb_led_change();
		printk("%d\n", (i+1));
		k_msleep(1000);
	}

	k_pipe_init(
		&_app_handle.pipe1,
		(unsigned char *)_app_handle.pipe1_buf,
		sizeof(_app_handle.pipe1_buf)
	);

	k_pipe_init(
		&_app_handle.pipe2,
		(unsigned char *)_app_handle.pipe2_buf,
		sizeof(_app_handle.pipe2_buf)
	);

	_app_handle.th_a.tid = k_thread_create(
		&_app_handle.th_a.t,
		_stack_a,
		K_THREAD_STACK_SIZEOF(_stack_a),
		_thread_a_enter,
		&_app_handle.th_a, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	_app_handle.th_b.tid = k_thread_create(
		&_app_handle.th_b.t,
		_stack_b,
		K_THREAD_STACK_SIZEOF(_stack_b),
		_thread_b_enter,
		&_app_handle.th_b, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	while (1) {
		k_msleep(10000);
	}
	
}

static void _thread_a_handle_user_in(_thread_handle_s *th) {
	static char userIn[USER_INPUT_BUF_SIZE];
	static uint32_t userInIdx = 0;
	int delay = -1;
	char c;
	int ret;
	size_t written_num_bytes;
	c = bsp_getchar();

	while(c >= 0 && c <= 127) { // execute while ascii char
		if(c == '\n' || c == '\r') {
			// I dont care put two new lines if necessary
			// some terminals send \r some dont
			printf("\n");
		} else {
			putchar(c);
		}

		if(userInIdx >= USER_INPUT_BUF_SIZE) {
			printf("user input buf ovf\n");
		} else if(c == '\r'){
			// just ignore this
		} else if(c != '\n') {
			userIn[userInIdx] = c;
			userInIdx++;
		} else if(userInIdx < 7){ // min expected num of chars 'delay 1'
			printf("malformed user input\n");
			// resetting everything
			userIn[0] = '\0';
			userInIdx = 0;
		} else {
			putchar('\n');
			userIn[userInIdx] = '\0';
			userInIdx++;
			sscanf(userIn, "delay %d", &delay);
			if(delay >= 1) {
				delay_t delayT = (delay_t)delay;

				ret = k_pipe_put(
					th->produce_pipe,
					&delayT,
					sizeof(delay_t),
					&written_num_bytes,
					sizeof(delay_t),
					K_NO_WAIT
				);

			} else {
				printf("ignored\n");
			}
			// resetting everything
			userIn[0] = '\0';
			userInIdx = 0;
		}
		c = bsp_getchar();
	}
}

static void _thread_a_print_msgs(_thread_handle_s *th) {
	static char msg[MSG_BUF_SIZE];
	static uint32_t msgIdx = 0;
	int ret; // receive return value
	char c;
	size_t read_num_bytes;

	ret = k_pipe_get(
		th->consume_pipe,
		&c,
		sizeof(char),
		&read_num_bytes,
		sizeof(char),
		K_NO_WAIT
	);

	if(ret != 0) {
		// do nothing
	} else if(msgIdx >= MSG_BUF_SIZE) {
		printf("msg buf ovf\n");
		while(1)
			;
	} else if(c != '\0'){
		msg[msgIdx] = c;
		msgIdx++;
	} else {
		msg[msgIdx] = c;
		printf("%s\n", msg);
		msgIdx = 0;
		msg[0] = '\0';
	}

}

static void _thread_a_enter(void *void_th, void *dummy2, void *dummy3) {
	assert(!(void_th == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	while(1) {
		_thread_a_handle_user_in(th);
		_thread_a_print_msgs(th);
		k_yield();
	}
}

static void _thread_b_enter(void *void_th, void *dummy2, void *dummy3) {
	assert(!(void_th == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	int ret;
	delay_t delayTmp;
	delay_t delay = 1000;
	size_t read_num_bytes;
	uint16_t nth = 0;
	uint16_t i;
	static char msg[] = "blinked";
	size_t written_num_bytes;

	while(1) {
		ret = k_pipe_get(
			th->consume_pipe,
			&delayTmp,
			sizeof(delay_t),
			&read_num_bytes,
			sizeof(delay_t),
			K_NO_WAIT
		);

		if(ret == 0) {
			delay = delayTmp;
		}

		k_msleep(delay);
		bsp_rgb_led_change();
		nth++;
		if(nth == BLINKED_MSG_EVERY_NTH) {
			nth = 0;
			for(i = 0; i < (sizeof(msg)/sizeof(char)); ++i) {
				ret = k_pipe_put(
					th->produce_pipe,
					&msg[i],
					sizeof(char),
					&written_num_bytes,
					sizeof(char),
					K_NO_WAIT
				);
			}
		}
		k_yield();
	}
}