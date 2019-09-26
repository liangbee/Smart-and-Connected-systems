/* UART Echo Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_types.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_intr_alloc.h"


/**
 * This is an example which echos any data it receives on UART1 back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)
#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC   (1) // seconds timer

#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload
#define BUF_SIZE (1024)
#define DATA_LENGTH                        512              /*!<Data buffer length for test buffer*/
#define RW_TEST_LENGTH                     129              /*!<Data length for r/w test, any value from 0-DATA_LENGTH*/
#define DELAY_TIME_BETWEEN_ITEMS_MS        1234             /*!< delay time between different test items */

#define I2C_EXAMPLE_SLAVE_SCL_IO           22               /*!<gpio number for i2c slave clock  */
#define I2C_EXAMPLE_SLAVE_SDA_IO           23               /*!<gpio number for i2c slave data */
#define I2C_EXAMPLE_SLAVE_NUM              I2C_NUM_0        /*!<I2C port number for slave dev */
#define I2C_EXAMPLE_SLAVE_TX_BUF_LEN       (2*DATA_LENGTH)  /*!<I2C slave tx buffer size */
#define I2C_EXAMPLE_SLAVE_RX_BUF_LEN       (2*DATA_LENGTH)  /*!<I2C slave rx buffer size */

#define I2C_EXAMPLE_MASTER_SCL_IO          22               /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO          23               /*!< gpio number for I2C master data  */
#define SWITCH_PORT                        26               // GPIO port for the stopwatch pushbutton.
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */

#define ESP_SLAVE_ADDR                     0x28             /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

#define BUTTON_GPIO 26
SemaphoreHandle_t print_mux = NULL;
static xQueueHandle gpio_evt_queue = NULL;
static xQueueHandle timer_queue;

int press = 1;
// in timer interruption, update the press value

typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;



void IRAM_ATTR timer_group0_isr(void *para)
{
    int timer_idx = (int) para;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[timer_idx].update = 1;
    uint64_t timer_counter_value =
        ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32
        | TIMERG0.hw_timer[timer_idx].cnt_low;

    /* Prepare basic event data
       that will be then sent back to the main program task */
    timer_event_t evt;
    evt.timer_group = 0;
    evt.timer_idx = timer_idx;
    evt.timer_counter_value = timer_counter_value;

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
    if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {
        evt.type = TEST_WITHOUT_RELOAD;
        TIMERG0.int_clr_timers.t0 = 1;
        timer_counter_value += (uint64_t) (TIMER_INTERVAL0_SEC * TIMER_SCALE);
        TIMERG0.hw_timer[timer_idx].alarm_high = (uint32_t) (timer_counter_value >> 32);
        TIMERG0.hw_timer[timer_idx].alarm_low = (uint32_t) timer_counter_value;
    } else {
        evt.type = -1; // not supported even type
    }


    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;

    /* Now just send the event data back to the main program task */
    xQueueSendFromISR(timer_queue, &evt, NULL);
}

 

uint8_t parser (int a){
    uint8_t result = 0x3F;
    switch (a){
        case 0 : {result = 0x3F; // 0 
            break;}
        case 1 : result = 0x06; // 1 
            break;
        case 2 : result = 0x5B;
            break;
        case 3 : result = 0x4F; /* 3 */
            break;
        case 4 : result = 0x66; /* 4 */
            break;
        case 5 : result = 0x6D;/* 5 */
            break;
        case 6 : result = 0x7D; /* 6 */
            break;
        case 7 : result = 0x07; /* 7 */
            break;
        case 8 : result = 0x7F; /* 8 */
            break;
        case 9 : result = 0x6F; /* 9 */
            break;
    }
    return result;

}


static void i2c_display(timer_event_t evt)
{
    int s_0,s_1,s_2,s_3;
    int sec = evt.timer_counter_value /5000010; // the value from timer.
    esp_err_t ret;
    s_3 = sec % 10;
    s_2 = (sec % 100 - s_3) / 10;
    s_1 = (sec % 1000 - s_2 - s_3) / 100;
    s_0 = (sec - s_1 - s_2 - s_3) / 1000;
    uint8_t s0;
    uint8_t s1;
    uint8_t s2;
    uint8_t s3;
    s0 = parser(s_0);
    s1 = parser(s_1);
    s2 = parser(s_2);
    s3 = parser(s_3);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | WRITE_BIT, ACK_CHECK_EN); 
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, s0, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00 , ACK_CHECK_EN);
    i2c_master_write_byte(cmd, s1, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, s2, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN); 
    i2c_master_write_byte(cmd, s3, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN); 
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        printf("OK");
    }
}

static void i2c_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    int32_t err = i2c_param_config(i2c_master_port, &conf);
    if (err == ESP_OK) printf("\n master param_config check: OK \n");
     err = i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    if (err == ESP_OK) printf("\n i2c_driver_install check: OK \n");
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    printf("test123\n");
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | WRITE_BIT, ACK_CHECK_EN); // 0x70 is the address of LED display
    i2c_master_write_byte(cmd, 0x21, ACK_CHECK_EN); // turning on oscillator
    i2c_master_stop(cmd);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | WRITE_BIT, ACK_CHECK_EN); // 0x70 is the address of LED display
    i2c_master_write_byte(cmd, 0x81, ACK_CHECK_EN); // Turning on display and blink
    i2c_master_stop(cmd);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | WRITE_BIT, ACK_CHECK_EN); // 0x70 is the address of LED display
    i2c_master_write_byte(cmd, 0xEF, ACK_CHECK_EN); // set brightness
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        printf("OK\n");
    }
    esp_err_t ret2;
    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, 0x70 << 1 | WRITE_BIT, ACK_CHECK_EN); 
    i2c_master_write_byte(cmd2, 0x00, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, 0x3F, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, 0x00 , ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, 0x3F, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, 0x00, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, 0x3F, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, 0x00, ACK_CHECK_EN); 
    i2c_master_write_byte(cmd2, 0x3F, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, 0x00, ACK_CHECK_EN); 
    i2c_master_stop(cmd2);
    ret2 = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd2);
    printf("OK2\n");
    if (ret2 != ESP_OK) {
        printf("OK3 \n");
    }
}


static void example_tg0_timer_init(int timer_idx,
    bool auto_reload, double timer_interval_sec, uint64_t init_counter_value)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, init_counter_value);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

static void timer_example_evt_task(void *arg)
{
    while (press == 1) {
        timer_event_t evt;
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);
        int test;
        test = evt.timer_counter_value / 5000010;

        /* Print information that the timer reported an event */
        if (evt.timer_idx == 0 && evt.timer_group == 0) {
            printf("\n    Seconds here\n");
            i2c_display(evt);
            printf("display called");
        } 
        else {
            printf("\n    UNKNOWN EVENT TYPE\n");
        }

    }
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    press += 1;
    //xQueueSendFromISR(gpio_evt_queue, &press, NULL);
}

void print_press() {
    printf("Count: %d\n",press);
}

 static void gpio_evt_task(){
   printf("button pressed");

    while(1){
         int prs = 1;
        xQueueReceive(gpio_evt_queue, &prs, portMAX_DELAY);
    if (prs == 0){
        // do nothing
    }
    else if (prs == 1){
        example_tg0_timer_init(TIMER_0,TEST_WITHOUT_RELOAD, TIMER_INTERVAL0_SEC,0); // seconds interrupt
        xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);
    }
    else if (prs == 2){
        timer_pause(0,0);
    }
    else if (prs == 3){
        timer_set_counter_value(0,0, 0x00000000ULL);
    }
    else if (prs == 4){
        timer_start(0,0);
        press = 1;
    }
 }
}

void gpio_init(){
    printf("gpio init begin \n");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = 1ULL << 4;
    io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.pull_down_en = 0;  
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    gpio_intr_enable(4);
    printf("gpio pad\n");
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add(4, gpio_isr_handler, (void*) 4);
    printf("gpio init done\n");
}
void app_main()
{
    printf("running \n");
   // timer_queue = xQueueCreate(10, sizeof(timer_event_t));
    //i2c_master_init();
    gpio_init();
    //gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //xTaskCreate(gpio_evt_task, "gpio_evt_task", 2048, NULL, 10, NULL);
    while(1){
   // printf("press is : ");
    // gpio_evt_task();
    //printf ("%d \n", press);
        vTaskDelay(100/portTICK_RATE_MS);
        print_press();
}
  
}

