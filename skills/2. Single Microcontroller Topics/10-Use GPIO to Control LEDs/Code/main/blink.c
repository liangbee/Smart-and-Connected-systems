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
#define BLINK_GPIO_4 26
#define BLINK_GPIO_3 25
#define BLINK_GPIO_2 4
#define BLINK_GPIO_1 12

void binary(int sum)
{
  if (sum == 0)
  {
  gpio_set_level(BLINK_GPIO_4, 0);
  gpio_set_level(BLINK_GPIO_3, 0);
  gpio_set_level(BLINK_GPIO_2, 0);
  gpio_set_level(BLINK_GPIO_1, 0);
  }

  else if (sum == 1)
  {
  gpio_set_level(BLINK_GPIO_4, 0);
  gpio_set_level(BLINK_GPIO_3, 0);
  gpio_set_level(BLINK_GPIO_2, 0);
  gpio_set_level(BLINK_GPIO_1, 1);
  }
  else if (sum == 2)
  {
  gpio_set_level(BLINK_GPIO_4, 0);
  gpio_set_level(BLINK_GPIO_3, 0);
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_1, 0);
  }
  else if (sum == 3)
  {
  gpio_set_level(BLINK_GPIO_4, 0);
  gpio_set_level(BLINK_GPIO_3, 0);
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_1, 1);
  }
  else if (sum == 4)
  {
  gpio_set_level(BLINK_GPIO_4, 0);
  gpio_set_level(BLINK_GPIO_3, 1);
  gpio_set_level(BLINK_GPIO_2, 0);
  gpio_set_level(BLINK_GPIO_1, 0);
  }
  else if (sum == 5)
  {
  gpio_set_level(BLINK_GPIO_4, 0);
  gpio_set_level(BLINK_GPIO_3, 1);
  gpio_set_level(BLINK_GPIO_2, 0);
  gpio_set_level(BLINK_GPIO_1, 1);
  }
  else if (sum == 6)
  {
  gpio_set_level(BLINK_GPIO_4, 0);
  gpio_set_level(BLINK_GPIO_3, 1);
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_1, 0);
  }
  else if (sum == 7)
  {
  gpio_set_level(BLINK_GPIO_4, 0);
  gpio_set_level(BLINK_GPIO_3, 1);
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_1, 1);
  }
  else if (sum == 8)
  {
  gpio_set_level(BLINK_GPIO_4, 1);
  gpio_set_level(BLINK_GPIO_3, 0);
  gpio_set_level(BLINK_GPIO_2, 0);
  gpio_set_level(BLINK_GPIO_1, 0);
  }
  else if (sum == 9)
  {
  gpio_set_level(BLINK_GPIO_4, 1);
  gpio_set_level(BLINK_GPIO_3, 0);
  gpio_set_level(BLINK_GPIO_2, 0);
  gpio_set_level(BLINK_GPIO_1, 1);
  }
  else if (sum == 10)
  {
  gpio_set_level(BLINK_GPIO_4, 1);
  gpio_set_level(BLINK_GPIO_3, 0);
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_1, 0);
  }
  else if (sum == 11)
  {
  gpio_set_level(BLINK_GPIO_4, 1);
  gpio_set_level(BLINK_GPIO_3, 0);
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_1, 1);
  }
    else if (sum == 12)
  {
  gpio_set_level(BLINK_GPIO_4, 1);
  gpio_set_level(BLINK_GPIO_3, 1);
  gpio_set_level(BLINK_GPIO_2, 0);
  gpio_set_level(BLINK_GPIO_1, 0);
  }
    else if (sum == 13)
  {
  gpio_set_level(BLINK_GPIO_4, 1);
  gpio_set_level(BLINK_GPIO_3, 1);
  gpio_set_level(BLINK_GPIO_2, 0);
  gpio_set_level(BLINK_GPIO_1, 1);
  }  else if (sum == 14)
  {
  gpio_set_level(BLINK_GPIO_4, 1);
  gpio_set_level(BLINK_GPIO_3, 1);
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_1, 0);
  }
  else if (sum == 15)
  {
  gpio_set_level(BLINK_GPIO_4, 1);
  gpio_set_level(BLINK_GPIO_3, 1);
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_1, 1);
  }




}


void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    int binary_sum = 0;
    gpio_pad_select_gpio(BLINK_GPIO_4);
    gpio_pad_select_gpio(BLINK_GPIO_3);
    gpio_pad_select_gpio(BLINK_GPIO_2);
    gpio_pad_select_gpio(BLINK_GPIO_1);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_1, GPIO_MODE_OUTPUT);
    while(1) {

        if (binary_sum <= 15){
          binary(binary_sum);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          binary_sum++;
        }
        else if (binary_sum > 15) {
          binary_sum = 0;
          binary(binary_sum);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          binary_sum++;
    }
}
}
void app_main()
{
    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}
