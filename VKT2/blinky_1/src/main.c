//Tavoittelen arvosanaa 1 alkajaiseksi. Koska kaikki tarvittava 1 pistettä varten on tehty, mutten ole vielä kerennyt tehdä 2 tai 3 pisteen osioita

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// Red led thread initialization
#define STACKSIZE 500
#define PRIORITY 5

volatile int led_state = 0;

int init_led(void);
void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);


// Main program
int main(void)
{
	init_led();
	return 0;
}

// Initialize leds
int  init_led() {

	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Led configure failed\n");		
		return ret;
	}

	int green_ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (green_ret < 0) {
		printk("Error: Led configure failed\n");
		return green_ret;
	}

	int blue_ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);
	if (blue_ret < 0) {
		printk("Error: Led configure failed\n");
		return blue_ret;
	}
	// set led off
	gpio_pin_set_dt(&red,0);
	gpio_pin_set_dt(&green,0);
	gpio_pin_set_dt(&blue,0);

	printk("Led initialized ok\n");
	
	return 0;
}

//// Task to handle red led
void red_led_task(void *, void *, void*) {
	
	printk("Red led thread started\n");
	while (true) {
		if (led_state == 0) {
			// 1. set led on 
			gpio_pin_set_dt(&red,1);
			// 2. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			// 3. set led off
			gpio_pin_set_dt(&red,0);

			led_state = 1;
		}
		k_msleep(100);
	}
}

void green_led_task(void *, void *, void*) {
	
	printk("Green led thread started\n");
	while (true) {
		if (led_state == 2) {
		// 1. set led on 
		gpio_pin_set_dt(&green,1);
		// 2. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		// 3. set led off
		gpio_pin_set_dt(&green,0);

		led_state = 0;
		}
		k_msleep(100);
	}
}

void yellow_led_task(void *, void *, void*) {
	
	printk("Yellow led thread started\n");
	while (true) {
		if (led_state == 1) {
		// 1. set led on 
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		// 2. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		// 3. set led off
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);

		led_state = 2;
		}
		k_msleep(100);
	}
}
