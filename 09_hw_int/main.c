#include <zephyr/kernel.h>
#include <zephyr/drivers/counter.h>
#include "bsp.h"
#include <assert.h>

#define ALARM_US (100000u)
#define ALARM_CHANNEL_ID 0
#define TIMER DT_NODELABEL(timer0)

static struct counter_alarm_cfg _alarm_cfg;
static int volatile _isr_counter = 0;

static void _counter_interrupt_fn(const struct device *counter_dev,
				      uint8_t chan_id, uint32_t ticks,
				      void *user_data) {
	unsigned int key = irq_lock();
	_isr_counter++;
	irq_unlock(key);
	_alarm_cfg.ticks = counter_us_to_ticks(counter_dev, ALARM_US);
	counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					&_alarm_cfg);
}


int main(void) {
	int err;
	unsigned int key;
	const struct device *const counter_dev = DEVICE_DT_GET(TIMER);
	if (!device_is_ready(counter_dev)) {
		printk("device not ready.\n");
		return 1;
	}
	counter_start(counter_dev);

	_alarm_cfg.flags = 0;
	_alarm_cfg.ticks = counter_us_to_ticks(counter_dev, ALARM_US);
	_alarm_cfg.callback = _counter_interrupt_fn;
	_alarm_cfg.user_data = &_alarm_cfg;

	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					&_alarm_cfg);

	printk("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(counter_dev,
					   _alarm_cfg.ticks) / USEC_PER_SEC),
	       _alarm_cfg.ticks);


	if (-EINVAL == err) {
		printk("Alarm settings invalid\n");
	} else if (-ENOTSUP == err) {
		printk("Alarm setting request not supported\n");
	} else if (err != 0) {
		printk("Error\n");
	}

	while(1) {
		while(_isr_counter > 0) {
			printf("%d\n", _isr_counter);
			key = irq_lock();
			_isr_counter--;
			irq_unlock(key);
		}
		printf("\nsleepy\n");
		k_msleep(1100);
	}
	return 0;
}