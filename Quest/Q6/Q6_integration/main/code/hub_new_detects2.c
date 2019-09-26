
#include <esp_system.h>
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "app_wifi.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "esp_http_client.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/mcpwm.h"
#include "driver/uart.h"
#include "soc/timer_group_struct.h"
#include "driver/timer.h"
#include "esp_types.h"

// RMT definitions
#define RMT_TX_CHANNEL    1     // RMT channel for transmitter
#define RMT_TX_GPIO_NUM   25    // GPIO number for transmitter signal -- A1
#define RMT_CLK_DIV       100   // RMT counter clock divider
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   // RMT counter value for 10 us.(Source clock is APB clock)
#define rmt_item32_tIMEOUT_US   9500     // RMT receiver timeout value(us)
// UART pin definitions
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 34 // A2
#define BUF_SIZE (1024)

#define WEB_SERVER "192.168.1.113"
#define WEB_PORT 1111
#define WEB_URL "192.168.1.113"

static const char *TAG = "example";

// above: RX

define GPIO_PWM0A_OUT 32   //Set GPIO 32 as PWM0A Enable pin Left
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

// above: car control 


char start = 0x1B;

char *buffer1 = "whizzer.bu";
char *buffer2 = ".edu/team-";
char *buffer3 = "quests/pri";
char *buffer4 = "mary/test0";
// char *message;
char message [41];
int f1 = 1; // fragment
int f2 = 1;
int f3 = 1;
int f4 = 1;


// Semaphores (for signaling), Mutex (for resources)
SemaphoreHandle_t xSemaphore = NULL;
SemaphoreHandle_t mux = NULL;
static xQueueHandle gpio_evt_queue = NULL;

static void http_post(char* input){
  esp_http_client_config_t config = {
        .url = "http://192.168.1.113",
        .port = 1111,
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    const char *post_data = input;
    esp_http_client_set_url(client, "http://192.168.1.113:1111/car");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

}
// Button interrupt handler -- add to queue
static void IRAM_ATTR gpio_isr_handler(void* arg){
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Configure UART for IR light
static void uart_init() {
  // Basic configs
  uart_config_t uart_config = {
      .baud_rate = 1200, // Slow BAUD rate
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_1, &uart_config);

  // Set UART pins using UART0 default pins
  uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Reverse receive logic line
  uart_set_line_inverse(UART_NUM_1,UART_INVERSE_RXD);

  // Install UART driver
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

char * parser(char* input){
  int c = 0;
  int pos = 3;
  static char str[11];
  while (c < 10){
    str[c] = input[pos+c-1];
    c++;
  }
  str[c] = '\0';
  return &str;
}

// Receives task -- looks for Start byte then stores received values
void recv_task(){
  // Buffer for input data
  printf("listening...\n");
  uint8_t *data_in = (uint8_t *) malloc(BUF_SIZE);
  while (1) {
    //int len_in = uart_read_bytes(UART_NUM_1, data_in, BUF_SIZE, 20 / portTICK_RATE_MS);
    if (f1 == 1 && f2 == 1 && f3 == 1 && f4 == 1){
      strcpy(message, buffer1);
      strcat(message, buffer2);
      strcat(message, buffer3);
      strcat(message, buffer4);
      printf("%s \n", message);
       http_post(message);
    }
    // if (len_in > 0) {
     // printf("detected \n");
      // for (int i=0; i < 24; i++) {
      //   if (data_in[i] == 0x1B) {
      //     if (data_in[i+1] == '1'){
      //      buffer1 = parser(data_in);
      //      f1 = 1;
      //     }
      //     else if (data_in[i+1] == '2'){
      //       buffer2 = parser(data_in);
      //       f2 = 1;
      //     }
      //     else if (data_in[i+1] == '3'){
      //       buffer3 = parser(data_in);
      //       f3 = 1;
      //     }
      //     else if (data_in[i+1] == '4'){
      //       buffer4 = parser(data_in);
      //       f4 = 1;
      //     }
      //     xSemaphoreGive(xSemaphore);
      //     break;
      //   }
      // }
    // }
   /* else{
      printf("Nothing received.\n");
    }
    */
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  free(data_in);

}

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


void app_main() {

    // Create the semaphore to signal
    vSemaphoreCreateBinary( xSemaphore );
    // Mutex for current values when sending and during election
    mux = xSemaphoreCreateMutex();
    check_efuse();
    mcpwm_example_gpio_initialize();
   esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    app_wifi_initialise();
    
    // uart_init();
     xTaskCreate(recv_task, "uart_rx_task", 1024*4, NULL, configMAX_PRIORITIES, NULL); // receive from raspi
     
   /*   strcpy(message, buffer1);
      strcat(message, buffer2);
      strcat(message, buffer3);
      strcat(message, buffer4);
      http_post(message);
      */
}
