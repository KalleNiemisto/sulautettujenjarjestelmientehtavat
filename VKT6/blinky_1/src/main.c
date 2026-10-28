//Tavoittelen arvosanaa 1 alkajaiseksi. Koska kaikki tarvittava 1 pistettä varten on tehty, mutten ole vielä kerennyt tehdä 2 tai 3 pisteen osioita

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/timing/timing.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "TimeParser.h"

#define STACKSIZE 1024
#define PRIORITY 5

int parse_seconds = 1;


// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

K_SEM_DEFINE(sem_red, 0, 1);
K_SEM_DEFINE(sem_yellow, 0, 1);
K_SEM_DEFINE(sem_green, 0, 1);
K_SEM_DEFINE(release_sem, 0, 1);

K_FIFO_DEFINE(uart_fifo);

struct fifo_msg {
	void *fifo_reserved; 
	char time_str[7]; 
};

int init_led(void);
void uart_rx_task(void *, void *, void*);
void dispatcher_task(void *, void *, void*);
void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);

K_THREAD_DEFINE(uart_rx_thread, STACKSIZE, uart_rx_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(dispatcher_thread, STACKSIZE, dispatcher_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(red_led_thread, STACKSIZE, red_led_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(green_led_thread, STACKSIZE, green_led_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(yellow_led_thread, STACKSIZE, yellow_led_task, NULL, NULL, NULL, PRIORITY, 0, 0);

// Main program
int main(void)
{
	timing_init();
	init_led();

	k_msleep(100);
	printk("Blinky ohjelma käynnistetty\n");

	while (true) {
		k_msleep(100);
	}
	return 0;
}

// Initialize leds
int  init_led() {

	if(!gpio_is_ready_dt(&red) || !gpio_is_ready_dt(&green) || !gpio_is_ready_dt(&blue)) {
		printk("Led GPIO ei valmiina\n");
		return -1;
	}

	gpio_pin_configure_dt(&red, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&green, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&blue, GPIO_OUTPUT_INACTIVE);

	return 0;
}

void uart_rx_task(void*, void*, void*){
	if (!device_is_ready(uart_dev)) {
		printk("UART laite ei valmiina\n");
		return;
	}

	unsigned char c;
	char rx_buf[7];
	int buf_index = 0;
	memset(rx_buf, 0, sizeof(rx_buf));

	while (1) {
		if (uart_poll_in(uart_dev, &c) == 0) {
			
			if(c == 'X' || c == 'x') {
				if (buf_index > 0){
					rx_buf[buf_index] = '\0';
					struct fifo_msg *msg = k_malloc(sizeof(struct fifo_msg));
					if (msg != NULL) {
						strncpy(msg->time_str, rx_buf, 7);
						k_fifo_put(&uart_fifo, msg);
					}
				}
				buf_index = 0;
				memset(rx_buf, 0, sizeof(rx_buf));

			} else {
				if (buf_index < 19) {
					rx_buf[buf_index++] = c;
				}
			}
		}
		k_msleep(10);
	}
}

void dispatcher_task(void*,void*,void*) {
	printk("Dispatcher käynnistetty\n");
	int colo_counter = 0;

	while (1) {
		
		struct fifo_msg *msg = k_fifo_get(&uart_fifo, K_FOREVER);
		
		int seconds = time_parse(msg->time_str);

		if (seconds >= 0) {

			printk("%dX\n", seconds);

			parse_seconds = seconds;
			k_free(msg);
			
			if (colo_counter == 0) {
				k_sem_give(&sem_red);
				colo_counter++;
			} else if (colo_counter == 1) {
				k_sem_give(&sem_yellow);
				colo_counter++;
			} else {
				k_sem_give(&sem_green);
				colo_counter = 0;
			}
			colo_counter = (colo_counter + 1) % 3;

		} else{
			
			printk("%dX\n", seconds);
			k_free(msg);
		}
	}
}

//// Task to handle red led
void red_led_task(void *, void *, void*) {
	
	while(1) {
		
		k_sem_take(&sem_red, K_FOREVER);

		gpio_pin_set_dt(&red, 1);
		k_sleep(K_SECONDS(parse_seconds));
		gpio_pin_set_dt(&red, 0);

		k_sem_give(&release_sem);
	}
}

void green_led_task(void *, void *, void*) {
	
	while(1) {
		
		k_sem_take(&sem_green, K_FOREVER);

		gpio_pin_set_dt(&green, 1);
		k_sleep(K_SECONDS(parse_seconds));
		gpio_pin_set_dt(&green, 0);

		k_sem_give(&release_sem);
	}
}

void yellow_led_task(void *, void *, void*) {
	
	while(1) {

		k_sem_take(&sem_yellow, K_FOREVER);

		gpio_pin_set_dt(&green, 1);
		gpio_pin_set_dt(&red, 1);
		k_sleep(K_SECONDS(parse_seconds));
		gpio_pin_set_dt(&green, 0);
		gpio_pin_set_dt(&red, 0);

		k_sem_give(&release_sem);
	}
}


