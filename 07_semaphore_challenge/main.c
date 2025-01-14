#include <zephyr/kernel.h>
#include "bsp.h"
#include <assert.h>
#include <stdlib.h>
#include "ringbuffer.h"

#define DEFAULT_STACK_SIZE (512u)
#define DEFAULT_PRIO (6u)
#define RING_BUF_SIZE (32u)
#define NUM_OF_PRODUCER_THREADS (5u)

K_THREAD_STACK_DEFINE(_stack_cons_a, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_cons_b, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_0, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_1, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_2, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_3, DEFAULT_STACK_SIZE);
K_THREAD_STACK_DEFINE(_stack_4, DEFAULT_STACK_SIZE);

typedef struct {
	struct k_thread t;
	k_tid_t tid;
	struct k_sem *sem;
	struct k_mutex *mutex;
	k_thread_stack_t * stack;
	char name_char;
	uint8_t num_of_send;
	ring_buffer_t *ring_buf;
} _thread_handle_s;

typedef struct {
	_thread_handle_s th_cons_a;
	_thread_handle_s th_cons_b;
	_thread_handle_s th_producers[NUM_OF_PRODUCER_THREADS];
	struct k_sem sem; // shared sem
	struct k_mutex mutex; // shared mutex
	ring_buffer_t ring_buf;
	char ring_buf_memory[RING_BUF_SIZE];
} _app_handle_s;

static void _thread_consumer_enter(void *, void *, void *);
static void _thread_producer_enter(void *, void *, void *);

// TODO: try to do this with dynamically allocated stack
static _app_handle_s _app_handle = {
	.th_cons_a.stack = _stack_cons_a,
	.th_cons_b.stack = _stack_cons_b,
	.th_producers[0].stack = _stack_0,
	.th_producers[1].stack = _stack_1,
	.th_producers[2].stack = _stack_2,
	.th_producers[3].stack = _stack_3,
	.th_producers[4].stack = _stack_4
};

int main(void) {
	uint8_t idx;

	bsp_rgb_led_init();
	// give it 10 sec for debugger and monitor to connect
	for(int i = 0; i < 5; ++i) {
		bsp_rgb_led_change();
		printk("%d\n", (i+1));
		k_msleep(1000);
	}

	k_sem_init(&_app_handle.sem, 0, RING_BUF_SIZE);
	k_mutex_init(&_app_handle.mutex);
	ring_buffer_init(&_app_handle.ring_buf, _app_handle.ring_buf_memory, sizeof(_app_handle.ring_buf_memory));

	// share the semaphore
	_app_handle.th_cons_a.sem = &_app_handle.sem;
	_app_handle.th_cons_a.mutex = &_app_handle.mutex;
	_app_handle.th_cons_a.ring_buf = &_app_handle.ring_buf;
	_app_handle.th_cons_a.name_char = 'a';
	_app_handle.th_cons_a.tid = k_thread_create(
		&_app_handle.th_cons_a.t,
		_stack_cons_a,
		K_THREAD_STACK_SIZEOF(_stack_cons_a),
		_thread_consumer_enter,
		&_app_handle.th_cons_a, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	// share the semaphore
	_app_handle.th_cons_b.sem = &_app_handle.sem;
	_app_handle.th_cons_b.mutex = &_app_handle.mutex;
	_app_handle.th_cons_b.ring_buf = &_app_handle.ring_buf;
	_app_handle.th_cons_b.name_char = 'b';
	_app_handle.th_cons_b.tid = k_thread_create(
		&_app_handle.th_cons_b.t,
		_stack_cons_b,
		K_THREAD_STACK_SIZEOF(_stack_cons_b),
		_thread_consumer_enter,
		&_app_handle.th_cons_b, NULL, NULL,
		DEFAULT_PRIO,
		0,
		K_NO_WAIT
	);

	for(idx = 0; idx < NUM_OF_PRODUCER_THREADS; ++idx) {
		_app_handle.th_producers[idx].ring_buf = &_app_handle.ring_buf;
		_app_handle.th_producers[idx].num_of_send = 0;
		_app_handle.th_producers[idx].name_char = '0' + idx;
		// share the semaphores with other threads
		_app_handle.th_producers[idx].sem = &_app_handle.sem;
		// share the mutex with other threads
		_app_handle.th_producers[idx].mutex = &_app_handle.mutex;
		// create print message threads
		_app_handle.th_producers[idx].tid = k_thread_create(
			&_app_handle.th_producers[idx].t,
			_app_handle.th_producers[idx].stack,
			K_THREAD_STACK_SIZEOF(DEFAULT_STACK_SIZE),
			_thread_producer_enter,
			&_app_handle.th_producers[idx], NULL, NULL,
			DEFAULT_PRIO,
			0,
			K_NO_WAIT
		);
	}

	while (1) {
		k_msleep(10000);
	}
	
}

// this thread is special it gets access to all app handle
static void _thread_consumer_enter(void *void_th, void *dummy2, void *dummy3) {
	assert(!(void_th == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	assert(!(th->sem == NULL));
	assert(!(th->mutex == NULL));
	assert(!(th->stack == NULL));
	assert(!(th->ring_buf == NULL));
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	char c;

	for( ; ; ) {
		if (k_sem_take(th->sem, K_NO_WAIT) == 0) {
			k_mutex_lock(th->mutex,K_FOREVER);
			ring_buffer_dequeue(th->ring_buf, &c);
			printf("%c %c\n", th->name_char, c);
			k_mutex_unlock(th->mutex);
		}
		k_yield();
	}
}

static void _thread_producer_enter(void *void_th, void *dummy2, void *dummy3) {
	assert(!(void_th == NULL));
	_thread_handle_s *th = (_thread_handle_s *) void_th;
	assert(!(th->sem == NULL));
	assert(!(th->mutex == NULL));
	assert(!(th->stack == NULL));
	assert(!(th->ring_buf == NULL));
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	for( ; ; ) {
		if(th->num_of_send >= 3) {
			// already achieved our goal
			printf("%c finito\n", th->name_char);
			break;
		} else if (k_mutex_lock(th->mutex,K_NO_WAIT) != 0) {
			printf("%c no luck\n", th->name_char);
			 // do nothing if I can not get access
		} else if(ring_buffer_is_full(th->ring_buf)) {
			printf("%c ring full\n", th->name_char);
			// if it is full just release
			k_mutex_unlock(th->mutex);
		} else {
			printf("%c send\n", th->name_char);
			ring_buffer_queue(th->ring_buf, th->name_char);
			th->num_of_send++;
			k_mutex_unlock(th->mutex);
			k_sem_give(th->sem);
		}
		k_yield();
	}

	k_thread_abort(th->tid);
}
