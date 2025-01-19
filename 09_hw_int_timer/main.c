#include <zephyr/kernel.h>
#include <zephyr/drivers/counter.h>
#include "bsp.h"
#include <assert.h>

#define DELAY 1000000
#define ALARM_CHANNEL_ID 0
#define TIMER DT_NODELABEL(timer0)

struct counter_alarm_cfg alarm_cfg;

static void counter_interrupt_fn(const struct device *counter_dev,
				      uint8_t chan_id, uint32_t ticks,
				      void *user_data) {
	int err;
	uint32_t now_ticks;
	uint64_t now_usec;
	int now_sec;
	err = counter_get_value(counter_dev, &now_ticks);
	if (!counter_is_counting_up(counter_dev)) {
		now_ticks = counter_get_top_value(counter_dev) - now_ticks;
	}

	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = (int)(now_usec / USEC_PER_SEC);

	printk("!!! Alarm !!!\n");
	printk("Now: %u %llu\n", now_sec, now_usec);

	alarm_cfg.ticks = counter_us_to_ticks(counter_dev, DELAY);
	counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					&alarm_cfg);
}


int main(void) {
	int err;
	const struct device *const counter_dev = DEVICE_DT_GET(TIMER);
	if (!device_is_ready(counter_dev)) {
		printk("device not ready.\n");
		return 1;
	}
	counter_start(counter_dev);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(counter_dev, DELAY);
	alarm_cfg.callback = counter_interrupt_fn;
	alarm_cfg.user_data = &alarm_cfg;

	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					&alarm_cfg);

	printk("Set alarm in %u sec (%u ticks)\n",
	       (uint32_t)(counter_ticks_to_us(counter_dev,
					   alarm_cfg.ticks) / USEC_PER_SEC),
	       alarm_cfg.ticks);


	if (-EINVAL == err) {
		printk("Alarm settings invalid\n");
	} else if (-ENOTSUP == err) {
		printk("Alarm setting request not supported\n");
	} else if (err != 0) {
		printk("Error\n");
	}

	while(1) {
		k_sleep(K_FOREVER);
	}
	return 0;
}