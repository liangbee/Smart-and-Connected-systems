/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
// #define BLINK_GPIO1 26
// #define BLINK_GPIO2 32
#define BLINK_GPIO1 26
#define BLINK_GPIO2 32

void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO1);
    gpio_pad_select_gpio(BLINK_GPIO2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO1, GPIO_MODE_INPUT);
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO2, 0);
        int result = gpio_get_level(BLINK_GPIO1);
        printf("before result is: %d\n", result);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO2, 1);
        result = gpio_get_level(BLINK_GPIO1);
        printf("after result is: %d \n", result);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}
