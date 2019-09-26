#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/queue.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define CONFIG_BUTTON_PIN 26
#define CONFIG_LED_PIN_1 32
#define CONFIG_LED_PIN_2 14
#define CONFIG_LED_PIN_3 33
#define CONFIG_LED_PIN_4 4
#define GPIO_PIN_INTR_ENABLE 1
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_OUTPUT_PIN_SEL (1ULL<<CONFIG_BUTTON_PIN)


int count = 0;
int state;

// interrupt service routine, called when the button is pressed
static void IRAM_ATTR button_isr_handler(void* arg) {

    // notify the button task
		state = 1;
		count = count + 1;
		if (count>3)
		 	count = 0;
}

void int_blink()
{
	//gpio_pad_select_gpio(CONFIG_BUTTON_PIN);
	gpio_pad_select_gpio(CONFIG_LED_PIN_1);
	gpio_pad_select_gpio(CONFIG_LED_PIN_2);
	gpio_pad_select_gpio(CONFIG_LED_PIN_3);
	gpio_pad_select_gpio(CONFIG_LED_PIN_4);


	// set the correct direction
	// gpio_set_direction(CONFIG_BUTTON_PIN, GPIO_MODE_INPUT);
	gpio_set_direction(CONFIG_LED_PIN_1, GPIO_MODE_OUTPUT);
	gpio_set_direction(CONFIG_LED_PIN_2, GPIO_MODE_OUTPUT);
	gpio_set_direction(CONFIG_LED_PIN_3, GPIO_MODE_OUTPUT);
	gpio_set_direction(CONFIG_LED_PIN_4, GPIO_MODE_OUTPUT);

	while(1){
		switch (count){
				vTaskDelay(100/portTICK_RATE_MS);

				case 0:
					gpio_set_level(CONFIG_LED_PIN_1, 1);
					gpio_set_level(CONFIG_LED_PIN_2, 0);
					gpio_set_level(CONFIG_LED_PIN_3, 0);
					gpio_set_level(CONFIG_LED_PIN_4, 0);
					break;

				case 1:
					gpio_set_level(CONFIG_LED_PIN_1, 0);
					gpio_set_level(CONFIG_LED_PIN_2, 1);
					gpio_set_level(CONFIG_LED_PIN_3, 0);
					gpio_set_level(CONFIG_LED_PIN_4, 0);
					break;

				case 2:
					gpio_set_level(CONFIG_LED_PIN_1, 0);
					gpio_set_level(CONFIG_LED_PIN_2, 0);
					gpio_set_level(CONFIG_LED_PIN_3, 1);
					gpio_set_level(CONFIG_LED_PIN_4, 0);
					break;

				case 3:
					gpio_set_level(CONFIG_LED_PIN_1, 0);
					gpio_set_level(CONFIG_LED_PIN_2, 0);
					gpio_set_level(CONFIG_LED_PIN_3, 0);
					gpio_set_level(CONFIG_LED_PIN_4, 1);
					break;
				}

		}
}

void app_main()
{
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_PIN_INTR_ENABLE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);
	gpio_intr_enable(CONFIG_BUTTON_PIN);
	gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
	gpio_isr_handler_add(CONFIG_BUTTON_PIN, button_isr_handler, (void*) CONFIG_BUTTON_PIN);
	int_blink();

}
