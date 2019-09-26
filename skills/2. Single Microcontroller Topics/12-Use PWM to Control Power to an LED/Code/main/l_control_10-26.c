/* servo motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#include "driver/uart.h"

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 10 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2400 //Maximum pulse width in microsecond change the brightness of the led
#define SERVO_MAX_DEGREE 180 //Maximum angle in degree upto which servo can rotate

#define BUF_SIZE (1024)

static void mcpwm_example_gpio_initialize()
{
    printf("Initialize GPIO 13\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 13);    //Set GPIO 13 as PWM0A, to which servo is connected
}


// static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
// {
//     uint32_t cal_pulsewidth = 0;
//     cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation) )/ SERVO_MAX_PULSEWIDTH));
//     return cal_pulsewidth;
// }

void mcpwm_example_servo_control(int input)
{
    uint32_t var, count;
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of led......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //fadjusting the speed for a cycle
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings

    while (1) {
        for (count = 0; count < input; count++) {
            printf("%s\n", "enter");
            var = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (count))/ SERVO_MAX_PULSEWIDTH));;
            printf("pulse width: %dus\n", var);
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, var);
            vTaskDelay(10);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }
    }
}

static int * echo_task()
{
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    static int arr[4];
    int i = 0;
    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        if (len > 0){
           printf("Input time: %c \n", * data);
           int temp = atoi((const int*) data);
           arr[i] = temp;
           i++;
           if (i == 4){
           return arr; // when the declared arr is called it auto matically set to a pointer
           }
      }

    }

}

void app_main()
{
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_0, &uart_config);
  uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

  printf("Testing led .......\n");

    //take user input
  int *point = echo_task();
    // get user input
  int a = point[0];
  int b = point[1];
  int c = point[2];
  int d = point[3];

  int input = (a * 1000) + (b * 100) + (c * 10) + d;
  //int input = (d * 1000) + (c * 100) + (b * 10) + a;

  mcpwm_example_servo_control(input);
}
