
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "sdkconfig.h"


#define DEFAULT_VREF    3300        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

/* above is IR definition*/

#define BLINK_GPIO_0 26 // green
#define BLINK_GPIO_1 25 // blue
#define BLINK_GPIO_2 4 // red
/* LED pinout */


float kp = 0.5;
float ki = 0.5;
float kd = 0.5;

float current_position = 0;
float target_position = 0;
float error = 0;
float integral = 0;
float derivative = 0;
float pwd = 0;
float last_error = 0;
double pid = 0;

/******* IR functoin define *******/
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

float IR()
{
    //Check if Two Point or Vref are burned into eFuse
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
    int voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    int distance = 146060 * (pow(voltage, -1.126));
    printf("distance: %dcm\n", distance);
    vTaskDelay(pdMS_TO_TICKS(1000));
    return distance;
}

/******* LED *******/

void led_ini(){
/* Selecting pin */
  gpio_pad_select_gpio(BLINK_GPIO_0);
  gpio_pad_select_gpio(BLINK_GPIO_1);
  gpio_pad_select_gpio(BLINK_GPIO_2);


/* Set the GPIO as a push/pull output */
  gpio_set_direction(BLINK_GPIO_0, GPIO_MODE_OUTPUT);
  gpio_set_direction(BLINK_GPIO_1, GPIO_MODE_OUTPUT);
  gpio_set_direction(BLINK_GPIO_2, GPIO_MODE_OUTPUT);

}

void  red () {
  gpio_set_level(BLINK_GPIO_2, 1);
  gpio_set_level(BLINK_GPIO_0, 0);
  gpio_set_level(BLINK_GPIO_1, 0);

}

void  green () {
  gpio_set_level(BLINK_GPIO_0, 1);
  gpio_set_level(BLINK_GPIO_1, 0);
  gpio_set_level(BLINK_GPIO_2, 0);
}

void  blue () {
  gpio_set_level(BLINK_GPIO_1, 1);
  gpio_set_level(BLINK_GPIO_0, 0);
  gpio_set_level(BLINK_GPIO_2, 0);
}


/****** PID *******/

void app_main() {

  led_ini();

  while(1){

    current_position = IR();

    error = 70 - current_position;

    integral = integral + error;

    derivative = error - last_error;

    pid = (kp * error) + (ki * integral) + (kd * derivative);

    printf("This is pid: %f\n", pid);

    if (pid < 0){
      //red led
      red();
    }

    if (pid == 0){
      //green led
      green();
    }

    if (pid > 0){
      //blue led
      blue();
    }

    last_error = error;
  }

}
