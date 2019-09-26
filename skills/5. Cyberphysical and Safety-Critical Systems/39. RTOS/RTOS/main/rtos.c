#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "sdkconfig.h"

#define BLINK_GPIO1 26 // Red led
#define BLINK_GPIO2 25 // Green led
#define BLINK_GPIO3 4 // Blue led 4

#define DEFAULT_VREF    3300        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

 static esp_adc_cal_characteristics_t *adc_chars;
 static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
 static const adc_atten_t atten = ADC_ATTEN_DB_0;
 static const adc_unit_t unit = ADC_UNIT_1;

uint32_t adc_reading = 0;
int twenty_readings[20];
int average = 0;
int prev_average = 0;
int led_status = 0; // 0 = no change green, 1 = positive red, -1 = negative blue
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

 static void thermistor_task() //task for thermistor
{
  printf("thermistor is working \n");
  //Continuously sample ADC1
  while (1) {
    for (int i = 0; i <20; i++){//Multisampling
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
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    twenty_readings[i] = voltage/6;
    //printf("Raw: %d\t Temperature: %d degree Celsius \n", adc_reading, voltage/3);
    vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay
  }

  }
}

void compare_task(){
  while(1){uint32_t sum = 0;
  for (int i = 0; i < 20; i++){
    sum += twenty_readings[i];
  }
  prev_average = average; // updates previous average
  average = sum / 20; // updates current prev_average
  printf("Average is, %d \n", average);
  printf("Previous average is, %d \n", prev_average);
  if (average > prev_average){
    led_status = 1; // pos Red
    printf("red\n");
  } 
  else if (average == prev_average){
    led_status = 0;
    printf("green\n");
  }  // no change Green
  else if (average < prev_average){
    led_status = -1;
    printf("blue\n"); // cooling blue
  } 
  vTaskDelay(2000 / portTICK_PERIOD_MS);} 
}

void led_task(void *pvParameter){
  gpio_pad_select_gpio(BLINK_GPIO1);
  gpio_pad_select_gpio(BLINK_GPIO2);
  gpio_pad_select_gpio(BLINK_GPIO3);
  // Set the GPIO as a push/pull output 
  gpio_set_direction(BLINK_GPIO1, GPIO_MODE_OUTPUT);
  gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);
  gpio_set_direction(BLINK_GPIO3, GPIO_MODE_OUTPUT);
  // Blink off (output low)
  gpio_set_level(BLINK_GPIO1, 0);
  gpio_set_level(BLINK_GPIO2, 0);
  gpio_set_level(BLINK_GPIO3, 0);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  gpio_set_level(BLINK_GPIO2, 1);
  gpio_set_level(BLINK_GPIO1, 1);
  gpio_set_level(4, 1);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  while (1){ if(led_status == 0){ // no change green
      gpio_set_level(BLINK_GPIO1, 0);
      gpio_set_level(BLINK_GPIO2, 1);
      gpio_set_level(BLINK_GPIO3, 0);
  }
  else if(led_status == 1){ // red
      gpio_set_level(BLINK_GPIO1, 1);
      gpio_set_level(BLINK_GPIO2, 0);
      gpio_set_level(BLINK_GPIO3, 0);
  }
  else if(led_status == -1){ // blue
      gpio_set_level(BLINK_GPIO1, 0);
      gpio_set_level(BLINK_GPIO2, 0);
      gpio_set_level(BLINK_GPIO3, 1);
  }
}
}
 
  

void adc_init(){
  //Check if Two Point or Vref are burned into eFuse
  check_efuse();
  //Configure ADC
  //Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);

  if (unit == ADC_UNIT_1) {
      adc1_config_width(ADC_WIDTH_BIT_12);
      adc1_config_channel_atten(channel, atten);
  } else {
      adc2_config_channel_atten((adc2_channel_t)channel, atten);
  }
}

void app_main()
{
  adc_init();
  //gpio_init();
  xTaskCreate(thermistor_task, "thermistor_task",1024*2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(compare_task,"compare_task",1024*2, NULL, configMAX_PRIORITIES-1, NULL);
  // xTaskCreate(&led_task, "led_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
  xTaskCreate(&led_task,"led_task",1024*2, NULL, configMAX_PRIORITIES-2, NULL);
}			    		// Instantiate tasks with priorites and stack size
