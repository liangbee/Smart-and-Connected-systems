/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Lin ul
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/uart.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "freertos/queue.h" // Lin ul
#include "freertos/semphr.h" // Lin ul
#include "esp_err.h" // Lin ul
#include "esp_log.h" // Lin ul
#include "driver/rmt.h" // Lin ul
#include "driver/periph_ctrl.h" // Lin ul
#include "soc/rmt_reg.h" // Lin ul


// * below for speed monitor use
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling
static esp_adc_cal_characteristics_t *adc_chars_optic;
static const adc_channel_t channel_optic = ADC_CHANNEL_3;     //GPIO34 if ADC1, GPIO14 if ADC2 a2 - chanel6 a3 - chanel3
static const adc_atten_t atten_optic = ADC_ATTEN_DB_0;
static const adc_unit_t unit_optic = ADC_UNIT_1;
// // * below for ir use
static esp_adc_cal_characteristics_t *adc_chars_ir;
static const adc_channel_t channel_ir = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten_ir = ADC_ATTEN_DB_0;
static const adc_unit_t unit_ir = ADC_UNIT_1;
// * below for lidar use
#define ECHO_TEST_TXD  16
#define ECHO_TEST_RXD  17
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)
#define BUF_SIZE (1024)

// * below for motor use
#define GPIO_PWM0A_OUT 32   //Set GPIO 15 as PWM0A Enable pin Left
#define GPIO_PWM0B_OUT 14  //Set GPIO A0 as PWM0B Enable pin Right

// Left wheele GPIO input
#define GPIO_LEFT_IN_ONE 15 //left 1
#define GPIO_LEFT_IN_TWO 33 //left 2

//Right wheele GPIO input
#define GPIO_RIGHT_IN_ONE 27 //A5
#define GPIO_RIGHT_IN_TWO 4

// *below for ir use
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling
// *below for timer use
#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC   (2) // sample test interval for the first timer
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

//******************************************************************************
static const char* NEC_TAG = "NEC";
#define RMT_RX_SELF_TEST   1

#if RMT_RX_SELF_TEST
#define RMT_RX_ACTIVE_LEVEL  1   /*!< Data bit is active high for self test mode */
#define RMT_TX_CARRIER_EN    0   /*!< Disable carrier for self test mode  */
#else
//Test with infrared LED, we have to enable carrier for transmitter
//When testing via IR led, the receiver waveform is usually active-low.
#define RMT_RX_ACTIVE_LEVEL  0   /*!< If we connect with a IR receiver, the data is active low */
#define RMT_TX_CARRIER_EN    1   /*!< Enable carrier for IR transmitter test with IR led */
#endif

#define RMT_TX_CHANNEL    1     /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM  25 //    /*!< GPIO number for transmitter signal */
#define RMT_RX_CHANNEL    0     /*!< RMT channel for receiver */
#define RMT_RX_GPIO_NUM  26 //    /*!< GPIO number for receiver */
#define RMT_CLK_DIV      100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

#define NEC_HEADER_HIGH_US    9000                         /*!< NEC protocol header: positive 9ms */
#define NEC_HEADER_LOW_US     4500                         /*!< NEC protocol header: negative 4.5ms*/
#define NEC_BIT_ONE_HIGH_US    560                         /*!< NEC protocol data bit 1: positive 0.56ms */
#define NEC_BIT_ONE_LOW_US    (2250-NEC_BIT_ONE_HIGH_US)   /*!< NEC protocol data bit 1: negative 1.69ms */
#define NEC_BIT_ZERO_HIGH_US   560                         /*!< NEC protocol data bit 0: positive 0.56ms */
#define NEC_BIT_ZERO_LOW_US   (1120-NEC_BIT_ZERO_HIGH_US)  /*!< NEC protocol data bit 0: negative 0.56ms */
#define NEC_BIT_END            560                         /*!< NEC protocol end: positive 0.56ms */
#define NEC_BIT_MARGIN         20                          /*!< NEC parse margin time */

#define NEC_ITEM_DURATION(d)  ((d & 0x7fff)*10/RMT_TICK_10_US)  /*!< Parse duration time from memory register value */
#define NEC_DATA_ITEM_NUM   34  /*!< NEC code item number: header + 32bit data + end */
#define RMT_TX_DATA_NUM  100    /*!< NEC tx test data number */
#define rmt_item32_tIMEOUT_US  9500   /*!< RMT receiver timeout value(us) */
//******************************************************************************
double num = 0;
double d = 0;
double speed = 0;
double v_ir = 0;
int result = 0;

int time = 0;
xQueueHandle timer_queue;
typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;

static void inline print_timer_counter(uint64_t counter_value)
{
    // printf("Counter: 0x%08x%08x\n", (uint32_t) (counter_value >> 32),
    //                                 (uint32_t) (counter_value));
    // printf("Time   : %.8f s\n", (double) counter_value / TIMER_SCALE);
}

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
    while (1) {
        timer_event_t evt;
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        /* Print information that the timer reported an event */
        if (evt.timer_idx == 0 && evt.timer_group == 0) {
            time++;

            // printf("Distance %d\n", result);
            // printf("Op_speed %f\n", speed);
            // printf("IR_Distance %f\n", v_ir);
            printf("Ultrasound %f\n", d);

        } else {
            // printf("\n    UNKNOWN EVENT TYPE\n");
        }
    }
}

//*************************************************************************************

static void mcpwm_example_gpio_initialize()
{
    // Left PWM wave
    // printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    gpio_set_level(GPIO_PWM0A_OUT, 0);

    // Right PWM wave
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
    gpio_set_level(GPIO_PWM0B_OUT, 0);


    // Set left wheele GPIO output
    gpio_pad_select_gpio(GPIO_LEFT_IN_ONE);
    gpio_set_direction(GPIO_LEFT_IN_ONE, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LEFT_IN_ONE, 0);

    gpio_pad_select_gpio(GPIO_LEFT_IN_TWO);
    gpio_set_direction(GPIO_LEFT_IN_TWO, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LEFT_IN_TWO, 0);

    //Right wheele GPIO output
    gpio_pad_select_gpio(GPIO_RIGHT_IN_ONE);
    gpio_set_direction(GPIO_RIGHT_IN_ONE, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_RIGHT_IN_ONE, 0);

    gpio_pad_select_gpio(GPIO_RIGHT_IN_TWO);
    gpio_set_direction(GPIO_RIGHT_IN_TWO, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_RIGHT_IN_TWO, 0);
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    // mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
    // mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);

    // Set GPIO One 1 high 1 low
    gpio_set_level(GPIO_LEFT_IN_ONE, 1);
    gpio_set_level(GPIO_LEFT_IN_TWO, 0);

    // Set GPIO Two to high 1 low
    gpio_set_level(GPIO_RIGHT_IN_ONE, 1);
    gpio_set_level(GPIO_RIGHT_IN_TWO, 0);

}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    // mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state

    // Set GPIO One 1 high 1 low
    gpio_set_level(GPIO_LEFT_IN_ONE, 0);
    gpio_set_level(GPIO_LEFT_IN_TWO, 1);

    // Set GPIO Two i high 1 low
    gpio_set_level(GPIO_RIGHT_IN_ONE, 0);
    gpio_set_level(GPIO_RIGHT_IN_TWO, 1);
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);

    // Set GPIO One both low
    gpio_set_level(GPIO_LEFT_IN_ONE, 0);
    gpio_set_level(GPIO_LEFT_IN_TWO, 0);

    // Set GPIO Two both low
    gpio_set_level(GPIO_RIGHT_IN_ONE, 0);
    gpio_set_level(GPIO_RIGHT_IN_TWO, 0);
}




static void check_efuse_ir()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        // printf("eFuse Two Point: Supported\n");
    } else {
        // printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        // printf("eFuse Vref: Supported\n");
    } else {
        // printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type_ir(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        // printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        // printf("Characterized using eFuse Vref\n");
    } else {
        // printf("Characterized using Default Vref\n");
    }
}

static void check_efuse_optic()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        // printf("eFuse Two Point: Supported\n");
    } else {
        // printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        // printf("eFuse Vref: Supported\n");
    } else {
        // printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        // printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        // printf("Characterized using eFuse Vref\n");
    } else {
        // printf("Characterized using Default Vref\n");
    }
}


// ultrasound
static inline void nec_fill_item_level(rmt_item32_t* item, int high_us, int low_us)
{
    item->level0 = 1;
    item->duration0 = (high_us) / 10 * RMT_TICK_10_US;
    item->level1 = 0;
    item->duration1 = (low_us) / 10 * RMT_TICK_10_US;
}

/*
 * @brief Generate NEC header value: active 9ms + negative 4.5ms
 */
static void nec_fill_item_header(rmt_item32_t* item)
{
    nec_fill_item_level(item, NEC_HEADER_HIGH_US, NEC_HEADER_LOW_US);
}

/*
 * @brief Generate NEC data bit 1: positive 0.56ms + negative 1.69ms
 */
static void nec_fill_item_bit_one(rmt_item32_t* item)
{
    nec_fill_item_level(item, NEC_BIT_ONE_HIGH_US, NEC_BIT_ONE_LOW_US);
}

/*
 * @brief Generate NEC data bit 0: positive 0.56ms + negative 0.56ms
 */
static void nec_fill_item_bit_zero(rmt_item32_t* item)
{
    nec_fill_item_level(item, NEC_BIT_ZERO_HIGH_US, NEC_BIT_ZERO_LOW_US);
}

/*
 * @brief Generate NEC end signal: positive 0.56ms
 */
static void nec_fill_item_end(rmt_item32_t* item)
{
    nec_fill_item_level(item, NEC_BIT_END, 0x7fff);
}

/*
 * @brief Check whether duration is around target_us
 */
inline bool nec_check_in_range(int duration_ticks, int target_us, int margin_us)
{
    if(( NEC_ITEM_DURATION(duration_ticks) < (target_us + margin_us))
        && ( NEC_ITEM_DURATION(duration_ticks) > (target_us - margin_us))) {
        return true;
    } else {
        return false;
    }
}

/*
 * @brief Check whether this value represents an NEC header
 */
static bool nec_header_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, NEC_HEADER_HIGH_US, NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, NEC_HEADER_LOW_US, NEC_BIT_MARGIN)) {
        return true;
    }
    return false;
}

/*
 * @brief Check whether this value represents an NEC data bit 1
 */
static bool nec_bit_one_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, NEC_BIT_ONE_HIGH_US, NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, NEC_BIT_ONE_LOW_US, NEC_BIT_MARGIN)) {
        return true;
    }
    return false;
}

/*
 * @brief Check whether this value represents an NEC data bit 0
 */
static bool nec_bit_zero_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, NEC_BIT_ZERO_HIGH_US, NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, NEC_BIT_ZERO_LOW_US, NEC_BIT_MARGIN)) {
        return true;
    }
    return false;
}


/*
 * @brief Parse NEC 32 bit waveform to address and command.
 */
static int nec_parse_items(rmt_item32_t* item, int item_num, uint16_t* addr, uint16_t* data)
{
    int w_len = item_num;
    if(w_len < NEC_DATA_ITEM_NUM) {
        return -1;
    }
    int i = 0, j = 0;
    if(!nec_header_if(item++)) {
        return -1;
    }
    uint16_t addr_t = 0;
    for(j = 0; j < 16; j++) {
        if(nec_bit_one_if(item)) {
            addr_t |= (1 << j);
        } else if(nec_bit_zero_if(item)) {
            addr_t |= (0 << j);
        } else {
            return -1;
        }
        item++;
        i++;
    }
    uint16_t data_t = 0;
    for(j = 0; j < 16; j++) {
        if(nec_bit_one_if(item)) {
            data_t |= (1 << j);
        } else if(nec_bit_zero_if(item)) {
            data_t |= (0 << j);
        } else {
            return -1;
        }
        item++;
        i++;
    }
    *addr = addr_t;
    *data = data_t;
    return i;
}

/*
 * @brief Build NEC 32bit waveform.
 */
static int nec_build_items(int channel, rmt_item32_t* item, int item_num, uint16_t addr, uint16_t cmd_data)
{
    int i = 0, j = 0;
    if(item_num < NEC_DATA_ITEM_NUM) {
        return -1;
    }
    nec_fill_item_header(item++);
    i++;
    for(j = 0; j < 16; j++) {
        if(addr & 0x1) {
            nec_fill_item_bit_one(item);
        } else {
            nec_fill_item_bit_zero(item);
        }
        item++;
        i++;
        addr >>= 1;
    }
    for(j = 0; j < 16; j++) {
        if(cmd_data & 0x1) {
            nec_fill_item_bit_one(item);
        } else {
            nec_fill_item_bit_zero(item);
        }
        item++;
        i++;
        cmd_data >>= 1;
    }
    nec_fill_item_end(item);
    i++;
    return i;
}

/*
 * @brief RMT transmitter initialization
 */
static void nec_tx_init()
{
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = RMT_TX_CARRIER_EN;
    rmt_tx.tx_config.idle_level = 0;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

/*
 * @brief RMT receiver initialization
 */
static void nec_rx_init()
{
    rmt_config_t rmt_rx;
    rmt_rx.channel = RMT_RX_CHANNEL;
    rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = true;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
    rmt_config(&rmt_rx);
    rmt_driver_install(rmt_rx.channel, 1000, 0);
}
//ultrasound
//***********************************************************************************
//***********************************************************************************


void optic_ir_motor_ul_main()
{
    timer_queue = xQueueCreate(10, sizeof(timer_event_t));
    example_tg0_timer_init(TIMER_0, TEST_WITHOUT_RELOAD, TIMER_INTERVAL0_SEC, 0); // seconds interrupt
    xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);
    //Check if Two Point or Vref are burned into eFuse
    check_efuse_optic();
    check_efuse_ir();
    mcpwm_example_gpio_initialize();
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    int counter = 0;
    uint32_t past_v_op = 1000;
    uint32_t voltage_op;
    //Configure ADC
    if (unit_optic == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel_optic, atten_optic);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel_optic, atten_optic);
    }

    if (unit_ir == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel_ir, atten_ir);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel_ir, atten_ir);
    }

    //Characterize ADC
    adc_chars_optic = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type_op = esp_adc_cal_characterize(unit_optic, atten_optic, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars_optic);
    print_char_val_type(val_type_op);

    adc_chars_ir = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type_ir = esp_adc_cal_characterize(unit_ir, atten_ir, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars_ir);
    print_char_val_type_ir(val_type_ir);
    //Continuously sample ADC1

    uart_config_t uart_config_lidar = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
        };
    uart_param_config(UART_NUM_0, &uart_config_lidar);
    uart_set_pin(UART_NUM_0, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);
    uint8_t *data_lidar = (uint8_t *) malloc(BUF_SIZE);
    uint8_t frame[2];

    // Ultrasound
    int channel_rx = RMT_RX_CHANNEL;
    nec_rx_init();
    RingbufHandle_t rb = NULL;

    //get RMT RX ringbuffer
    rmt_get_ringbuf_handle(channel_rx, &rb);
    rmt_rx_start(channel_rx, 1);

    //vTaskDelay(10);
    nec_tx_init();
    esp_log_level_set(NEC_TAG, ESP_LOG_INFO);
    int channel_tx = RMT_TX_CHANNEL;
    uint16_t cmd = 0x0;
    uint16_t addr = 0x11;
    int nec_tx_num = RMT_TX_DATA_NUM;


    while (1) {

      //int counter = 0;
        int len = uart_read_bytes(UART_NUM_0, data_lidar, BUF_SIZE, 20 / portTICK_RATE_MS);
          for (int i = 0; i < 24; i++){
            if (data_lidar[i]== 0x59 && data_lidar[i+1] == 0x59){
              frame[1] = data_lidar[i+2];
              frame[0] = data_lidar[i+3];
            }
          }
        //uint16_t result;
        result = (frame[0] << 8)+ frame[1];

        uint32_t adc_reading_op = 0;
        uint32_t adc_reading_ir = 0;
        //Multisampling
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 100);
        vTaskDelay(2000 / portTICK_RATE_MS);
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 100);
        vTaskDelay(2000 / portTICK_RATE_MS);
        brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
        //vTaskDelay(2000 / portTICK_RATE_MS);
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit_optic == ADC_UNIT_1) {
                adc_reading_op += adc1_get_raw((adc1_channel_t)channel_optic);
            } else {
                int raw_op;
                adc2_get_raw((adc2_channel_t)channel_optic, ADC_WIDTH_BIT_12, &raw_op);
                adc_reading_op += raw_op;
            }
        }

        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit_ir == ADC_UNIT_1) {
                adc_reading_ir += adc1_get_raw((adc1_channel_t)channel_ir);
            } else {
                int raw_ir;
                adc2_get_raw((adc2_channel_t)channel_ir, ADC_WIDTH_BIT_12, &raw_ir);
                adc_reading_ir += raw_ir;
            }
        }
        adc_reading_op /= NO_OF_SAMPLES;
        adc_reading_ir /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage_op = esp_adc_cal_raw_to_voltage(adc_reading_op, adc_chars_optic);
        // printf("Raw: %d\tVoltage: %dmV past_v: %d\n", adc_reading_op, voltage_op,past_v_op);
        if (voltage_op > past_v_op && voltage_op > 900 && past_v_op < 900){
            counter++;
            // printf("rise_edge \n");
        }
        if (voltage_op < past_v_op && past_v_op > 900 ){
             counter++;
             // printf("fall_edge \n");
        }
        past_v_op = voltage_op;
        // speed = counter * 2 * 3.14 * 3 / time;
        speed = counter * 2 * 3.14 * 3 / time;

        uint32_t voltage_ir = esp_adc_cal_raw_to_voltage(adc_reading_ir, adc_chars_ir);
        uint32_t v_ir = 150 - 0.058 * voltage_ir;
        //printf("Raw: %d\tVoltage: %dcm\n", adc_reading_ir, v_ir);


        // Ultrasound
        //**********************************************************************************************************************************
        if(rb) {
            size_t rx_size = 0;
            //try to receive data from ringbuffer.
            //RMT driver will push all the data it receives to its ringbuffer.
            //We just need to parse the value and return the spaces of ringbuffer.
            rmt_item32_t* item_rx = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 1000);
            // printf("Printing Size %d \n", rx_size);//rx_size/4;
            if(item_rx) {
                // printf("Right after item_rx\n");
                uint16_t rmt_addr;
                uint16_t rmt_cmd;
                int offset_rx = 0;
                // while(1) { // while bracket
                    //parse data value from ringbuffer.
                    num = item_rx[0].duration0;
                    d = 340.9 * num / 2000;

                    int res = nec_parse_items(item_rx + offset_rx, rx_size / 4 - offset_rx, &rmt_addr, &rmt_cmd);
                    if(res > 0) {
                        offset_rx += res + 1;
                        ESP_LOGI(NEC_TAG, "RMT RCV --- addr: 0x%04x cmd: 0x%04x", rmt_addr, rmt_cmd);
                    } else {
                        //break;
                    }
                //} // while bracket
                //after parsing the data, return spaces to ringbuffer.
                vRingbufferReturnItem(rb, (void*) item_rx);
            } else {
                //break;
            }
        }
        //vTaskDelete(NULL);

        //*******************************************************************************
            // printf("Right before RMT TX\n");
            ESP_LOGI(NEC_TAG, "RMT TX DATA");
            size_t size = (sizeof(rmt_item32_t) * NEC_DATA_ITEM_NUM * nec_tx_num);
            //each item represent a cycle of waveform.
            rmt_item32_t* item_tx = (rmt_item32_t*) malloc(size);
            int item_num = NEC_DATA_ITEM_NUM * nec_tx_num;
            memset((void*) item_tx, 0, size);
            int k, offset_tx = 0;
            //while(1) {
                //To build a series of waveforms.
                k = nec_build_items(channel_tx, item_tx + offset_tx, item_num - offset_tx, ((~addr) << 8) | addr, ((~cmd) << 8) |  cmd);
                if(k < 0) {
                    break;
                }
                cmd++;
                addr++;
                offset_tx += k;
            //}
            //To send data according to the waveform items.
            rmt_write_items(channel_tx, item_tx, item_num, true);
            //Wait until sending is done.
            rmt_wait_tx_done(channel_tx, portMAX_DELAY);
            //before we free the data, make sure sending is already done.
            free(item_tx);
            vTaskDelay(2000 / portTICK_PERIOD_MS);

      } // main while loop bracket
     vTaskDelete(NULL);

}


void app_main()
{
  optic_ir_motor_ul_main();
}
