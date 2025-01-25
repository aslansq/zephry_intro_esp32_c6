
#include "bsp.h"
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include "pcr_reg.h"
#include "periph_defs.h"
#include "timer_group_reg.h"
#include <zephyr/devicetree.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

#define PIN_NUM 8

static const struct device *gpio_ct_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
// we need to do delay with macros because required delays are two fast
#define DELAY(x)    for(uint64_t delayIdx = 0; delayIdx < (x); ++delayIdx) {__asm__("nop");}
#define SET_PIN(st) gpio_pin_set_raw(gpio_ct_dev,PIN_NUM,(st))
// magic numbers oh well: I just look with scope to meet timing requirements
// see requirements below
// https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
#define SEND0()     SET_PIN(1);DELAY(8);SET_PIN(0);DELAY(25)
#define SEND1()     SET_PIN(1);DELAY(22);SET_PIN(0);DELAY(9)
#define RESET()     SET_PIN(0);k_usleep(50)

uint8_t bsp_rgb_led_init(void) {
	int32_t ret;
	bsp_rgb_led_color_s val = {
		.r = 0,
		.g = 0,
		.b = 0
	};

	if(!device_is_ready(gpio_ct_dev)) {
		return 1;
	}

	ret = gpio_pin_configure(
		gpio_ct_dev,
		PIN_NUM,
		GPIO_OUTPUT_ACTIVE
	);
	if(ret == 0) {
		RESET();
		bsp_rgb_led_set(&val);
	}

	return ret;
}

void bsp_rgb_led_set(bsp_rgb_led_color_s *val) {
	unsigned int key = irq_lock();
	uint32_t data = 0;
	data |= (val->g << 16);
	data |= (val->r << 8);
	data |= val->b;
	for(uint8_t i = 0; i < 24; ++i) {
		if(data & (0x800000)) { // check 23th bit
			SEND1();
		} else {
			SEND0();
		}
		data = data << 1;
	}
	irq_unlock(key);
	RESET();
}

void bsp_rgb_led_change(void) {
	static bsp_rgb_led_color_s val = {
		.r = 0,
		.g = 0,
		.b = 0
	};
	static uint8_t valIdx = 0;
	val.r = 0;
	val.g = 0;
	val.b = 0;
	if(valIdx >= 3) {
		valIdx = 0;
	}
	switch(valIdx) {
		case 0:
			val.r = 255;
			break;
		case 1:
			val.g = 255;
			break;
		case 2:
			val.b = 255;
			break;
		default:
			val.r = 255;
			val.g = 255;
			val.b = 255;
			break;
	}

	bsp_rgb_led_set(&val);

	valIdx++;
}

char bsp_getchar(void) {
	char c;
	if(uart_poll_in(uart_dev, &c) != 0) {
		c = -1;
	}
	return c;
}

int16_t bsp_getline(char *buf, uint8_t size) {
	int16_t ret = 0;
	char c = bsp_getchar();
	uint8_t bufIdx;
	for(bufIdx = 0; bufIdx < size;) {
		// ignore non ascii chars
		if(c < 0 || c > 127 || c == '\r') {
			// do nothing
		} else if(c == '\n') {
			buf[bufIdx] = '\0';
			bufIdx++;
			ret = bufIdx;
			break;
		} else if(bufIdx == (size-1)){
			ret = -1;
			break;
		} else {
			buf[bufIdx] = c;
			bufIdx++;
		}
		c = bsp_getchar();
		k_yield();
	}
	return ret;
}

extern void timer1_handler(void);

static void _bsp_timer1_handler(void *arg) {
	ARG_UNUSED(arg);
	REG_SET_BIT(TIMG_INT_CLR_TIMERS_REG(1), TIMG_T0_INT_CLR_M);
	REG_SET_BIT(TIMG_T0CONFIG_REG(1), (TIMG_T0_EN_M | TIMG_T0_ALARM_EN_M));
	timer1_handler();
}

void bsp_tm1_int_setup(void) {
	REG_SET_BIT(PCR_TIMERGROUP1_CONF_REG, PCR_TG1_CLK_EN_M);
	REG_CLR_BIT(PCR_TIMERGROUP1_CONF_REG, PCR_TG1_RST_EN_M);
	//printk("pcr cfg %u\n", REG_READ(PCR_TIMERGROUP1_CONF_REG));
	REG_SET_BIT(PCR_TIMERGROUP1_TIMER_CLK_CONF_REG, PCR_TG1_TIMER_CLK_EN_M);
	REG_SET_BIT(PCR_TIMERGROUP1_TIMER_CLK_CONF_REG, (1<<PCR_TG1_TIMER_CLK_SEL_S));
	//printk("pcr clk cfg %u\n", REG_READ(PCR_TIMERGROUP1_TIMER_CLK_CONF_REG));

	REG_WRITE(TIMG_T0CONFIG_REG(1), 0);
	REG_SET_BIT(TIMG_T0CONFIG_REG(1), (
		(2000<<TIMG_T0_DIVIDER_S) | // 80 MHz / 2000 = 40 kHz
		TIMG_T0_DIVCNT_RST_M |
		TIMG_T0_INCREASE_M |
		TIMG_T0_ALARM_EN_M
	));
	//printk("tm1 cfg %u\n", REG_READ(TIMG_T0CONFIG_REG(1)));

	REG_WRITE(TIMG_T0ALARMLO_REG(1), 40000); // 1 sec
	REG_WRITE(TIMG_T0ALARMHI_REG(1), 0);
	//printk("tm1 alm lo %u\n", REG_READ(TIMG_T0ALARMLO_REG(1)));
	//printk("tm1 alm hi %u\n", REG_READ(TIMG_T0ALARMHI_REG(1)));

	REG_SET_BIT(TIMG_INT_ENA_TIMERS_REG(1), TIMG_T0_INT_ENA_M);
	//printk("tm1 int ena %u\n", REG_READ(TIMG_INT_ENA_TIMERS_REG(1)));

	REG_WRITE(TIMG_T0LOADLO_REG(1), 0);
	//printk("tm1 load lo %u\n", REG_READ(TIMG_T0LOADLO_REG(1)));
	REG_WRITE(TIMG_T0LOADHI_REG(1), 0);
	//printk("tm1 load hi %u\n", REG_READ(TIMG_T0LOADHI_REG(1)));

	REG_SET_BIT(TIMG_T0CONFIG_REG(1), (TIMG_T0_EN_M | TIMG_T0_AUTORELOAD_M));
	//printk("tm1 cfg %u\n", REG_READ(TIMG_T0CONFIG_REG(1)));

	IRQ_CONNECT(ETS_TG1_T0_LEVEL_INTR_SOURCE, 2, _bsp_timer1_handler, NULL, 0);
	irq_enable(ETS_TG1_T0_LEVEL_INTR_SOURCE);
}