//Tavoittelen arvosanaa 1 alkajaiseksi. Koska kaikki tarvittava 1 pistettä varten on tehty, mutten ole vielä kerennyt tehdä 2 tai 3 pisteen osioita

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#define STACKSIZE 1024
#define PRIORITY 5

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
	char cmd;
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
	init_led();
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
	while (1) {
		if (uart_poll_in(uart_dev, &c) == 0) {
			if (c == 'r' || c == 'g' || c == 'y' || c == 'R' || c == 'G' || c == 'Y') {
				struct fifo_msg *msg = k_malloc(sizeof(struct fifo_msg));
				if (msg != NULL) {
					msg->cmd = c;
					k_fifo_put(&uart_fifo, msg);
				} else {
					printk("Muistin varaus epäonnistui\n");
				}
			} 
		}
		k_msleep(10);
	}
}

void dispatcher_task(void*,void*,void*) {
	printk("Dispatcher käynnistetty\n");

	while (1) {
		
		struct fifo_msg *msg = k_fifo_get(&uart_fifo, K_FOREVER);
		
		char cmd = msg->cmd;
		k_free(msg);

		bool valid_cmd = true;
		switch (cmd) {
			case 'r':
			case 'R':
				k_sem_give(&sem_red);
				break;
			case 'g':
			case 'G':
				k_sem_give(&sem_green);
				break;
			case 'y':
			case 'Y':
				k_sem_give(&sem_yellow);
				break;
			default:
				printk("Tuntematon komento: %c\n", cmd);
				valid_cmd = false;
				break;
		}

		if (valid_cmd) {
			k_sem_take(&release_sem, K_FOREVER);
		}
	}
}

//// Task to handle red led
void red_led_task(void *, void *, void*) {
	
	while(1) {

		k_sem_take(&sem_red, K_FOREVER);

		gpio_pin_set_dt(&red, 1);
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red, 0);

		k_sem_give(&release_sem);
	}
}

void green_led_task(void *, void *, void*) {
	
	while(1) {

		k_sem_take(&sem_green, K_FOREVER);

		gpio_pin_set_dt(&green, 1);
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&green, 0);

		k_sem_give(&release_sem);
	}
}

void yellow_led_task(void *, void *, void*) {
	
	while(1) {

		k_sem_take(&sem_yellow, K_FOREVER);

		gpio_pin_set_dt(&green, 1);
		gpio_pin_set_dt(&red, 1);
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&green, 0);
		gpio_pin_set_dt(&red, 0);

		k_sem_give(&release_sem);
	}
}
