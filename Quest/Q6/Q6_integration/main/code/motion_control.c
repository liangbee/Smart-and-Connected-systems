#include <esp_system.h>
#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#include "driver/uart.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_types.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/periph_ctrl.h"



#define GPIO_PWM0A_OUT 32   //Set GPIO 32 as PWM0A Enable pin Left
#define GPIO_PWM0B_OUT 14   //Set GPIO 14 as PWM0B Enable pin Right

// Left wheele GPIO input
#define GPIO_LEFT_IN_ONE 15 //left 1
#define GPIO_LEFT_IN_TWO 33 //left 2

//Right wheele GPIO input
#define GPIO_RIGHT_IN_ONE 27 // GPIO 27
#define GPIO_RIGHT_IN_TWO 4 // A5

/* above are pin assignments for motor control. */
#define DEFAULT_VREF    1023  //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64    //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC2_CHANNEL_0;   // GPIO #4 / A5 input
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_2;

static void check_efuse()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}


uint32_t ultrasound()
{
  check_efuse();

  //Configure ADC
  if (unit == ADC_UNIT_1) {
      adc1_config_width(ADC_WIDTH_BIT_12);
      adc1_config_channel_atten(channel, atten);
  } else {
      adc2_config_channel_atten((adc2_channel_t)channel, atten);
  }

  //Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);

  //Continuously sample ADC1
  // while (1) {
  uint32_t adc_reading = 0;
  //Multisampling
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    if (unit == ADC_UNIT_1) {
      adc_reading += adc1_get_raw((adc1_channel_t)channel);
    } else {
      int raw;
      adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
      adc_reading += raw;
    }
  }
  adc_reading /= NO_OF_SAMPLES;
  //Convert adc_reading to voltage in mV
  uint32_t distance = (adc_reading * 5 + 300) / 10;
  printf("Distance: %dcm\n", distance);
  // }
  return distance;

}

static void mcpwm_example_gpio_initialize()
{
    /* Left PWM wave */
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    gpio_set_level(GPIO_PWM0A_OUT, 0);

    /* Right PWM wave */
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
    gpio_set_level(GPIO_PWM0B_OUT, 0);


    /* Left wheele GPIO output */
    gpio_pad_select_gpio(GPIO_LEFT_IN_ONE);
    gpio_set_direction(GPIO_LEFT_IN_ONE, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LEFT_IN_ONE, 0);

    gpio_pad_select_gpio(GPIO_LEFT_IN_TWO);
    gpio_set_direction(GPIO_LEFT_IN_TWO, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LEFT_IN_TWO, 0);

    /* Right wheele GPIO output */
    gpio_pad_select_gpio(GPIO_RIGHT_IN_ONE);
    gpio_set_direction(GPIO_RIGHT_IN_ONE, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_RIGHT_IN_ONE, 0);

    gpio_pad_select_gpio(GPIO_RIGHT_IN_TWO);
    gpio_set_direction(GPIO_RIGHT_IN_TWO, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_RIGHT_IN_TWO, 0);


    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

// go straight, with duty cycle = duty %
static void go_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle, uint32_t distance)
{
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);

    if(distance > 40) {
      gpio_set_level(GPIO_LEFT_IN_ONE, 1);
      gpio_set_level(GPIO_LEFT_IN_TWO, 0);

      gpio_set_level(GPIO_RIGHT_IN_ONE, 1);
      gpio_set_level(GPIO_RIGHT_IN_TWO, 0);

      vTaskDelay(200 / portTICK_RATE_MS);

      gpio_set_level(GPIO_LEFT_IN_ONE, 0);
      gpio_set_level(GPIO_LEFT_IN_TWO, 0);

      gpio_set_level(GPIO_RIGHT_IN_ONE, 0);
      gpio_set_level(GPIO_RIGHT_IN_TWO, 0);

    }
    printf("Going forward\n");
}

// turn_left, with duty cycle = duty %
static void turn_left(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle){
  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);

  gpio_set_level(GPIO_LEFT_IN_ONE, 0);
  gpio_set_level(GPIO_LEFT_IN_TWO, 0);

  gpio_set_level(GPIO_RIGHT_IN_ONE, 1);
  gpio_set_level(GPIO_RIGHT_IN_TWO, 0);

  vTaskDelay(200 / portTICK_RATE_MS);

  gpio_set_level(GPIO_LEFT_IN_ONE, 0);
  gpio_set_level(GPIO_LEFT_IN_TWO, 0);

  gpio_set_level(GPIO_RIGHT_IN_ONE, 0);
  gpio_set_level(GPIO_RIGHT_IN_TWO, 0);

  printf("Turning left\n");
}

// turn_right, with duty cycle = duty %
static void turn_right(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle){
  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);

  gpio_set_level(GPIO_LEFT_IN_ONE, 1);
  gpio_set_level(GPIO_LEFT_IN_TWO, 0);

  gpio_set_level(GPIO_RIGHT_IN_ONE, 0);
  gpio_set_level(GPIO_RIGHT_IN_TWO, 0);

  vTaskDelay(200 / portTICK_RATE_MS);

  gpio_set_level(GPIO_LEFT_IN_ONE, 0);
  gpio_set_level(GPIO_LEFT_IN_TWO, 0);

  gpio_set_level(GPIO_RIGHT_IN_ONE, 0);
  gpio_set_level(GPIO_RIGHT_IN_TWO, 0);

  printf("Turning right\n");
}


// go back, with duty cycle = duty %
static void go_back(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle){
  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);

  gpio_set_level(GPIO_LEFT_IN_ONE, 0);
  gpio_set_level(GPIO_LEFT_IN_TWO, 1);

  gpio_set_level(GPIO_RIGHT_IN_ONE, 0);
  gpio_set_level(GPIO_RIGHT_IN_TWO, 1);

  vTaskDelay(200 / portTICK_RATE_MS);

  gpio_set_level(GPIO_LEFT_IN_ONE, 0);
  gpio_set_level(GPIO_LEFT_IN_TWO, 0);

  gpio_set_level(GPIO_RIGHT_IN_ONE, 0);
  gpio_set_level(GPIO_RIGHT_IN_TWO, 0);

  printf("Going backward\n");
}


void app_main()
{
    // Initialize car
    check_efuse();
    mcpwm_example_gpio_initialize();

    while(1) {
      go_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 70, ultrasound());
      vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
