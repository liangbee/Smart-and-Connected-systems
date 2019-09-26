#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include <esp_wifi.h>
#include "app_wifi.h"
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <http_server.h>
#include "esp_http_client.h"

#define EXAMPLE_WIFI_SSID Group_9
#define EXAMPLE_WIFI_PASS smart-systems
static const char *buttonTAG= "HTTP_CLIENT";
static const char *ledTAG="APP";
#define MAX_HTTP_RECV_BUFFER 512
extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]   asm("_binary_howsmyssl_com_root_cert_pem_end");

// Hardware interrupt definitions
#define GPIO_INPUT_IO_1       36 // A4 for button press
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL    1ULL<<GPIO_INPUT_IO_1

#define GREENPIN  26 // A0

//#define WEB_SERVER "192.168.1.107" // node
#define WEB_SERVER "192.168.1.111" // node
#define WEB_PORT  3000// 3000
#define WEB_URL "192.168.1.111" // node

char led_status = '0'; // 0 is off, 1 is on
char button_status = '0'; // 0 is unpressed , 1 is pressed
char button_buffer[] = "1";
// Semaphores (for signaling), Mutex (for resources), and Queues (for button)
SemaphoreHandle_t xSemaphore = NULL;
SemaphoreHandle_t mux = NULL;
static xQueueHandle gpio_evt_queue = NULL;
static httpd_handle_t server = NULL;


static const char *buttonREQUEST = "POST" WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

// above are button post code 
    esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(buttonTAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(buttonTAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(buttonTAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(buttonTAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(buttonTAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(buttonTAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(buttonTAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

esp_err_t ctrl_put_handler(httpd_req_t *req)
{
    char buf[10];
    int ret;

    if ((ret = httpd_req_recv(req, buf, 10)) < 0) {
        return ESP_FAIL;
    }

    char led_stat = buf[4];
    if(led_stat == '1') {
      led_status = '1';
    } else {
      led_status = '0';
    }

    printf("%c\n", led_stat);

    /* Respond with empty body */
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static void http_post(){
  esp_http_client_config_t config = {
        .url = "http://192.168.1.111", // pi's address
        .port = 3000,
        .method = HTTP_METHOD_POST,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    const char *post_data = button_buffer; // FOB_ID:CODE:HUB_ID
    printf("Posting data: %s\n", post_data);
    esp_http_client_set_url(client, "http://192.168.1.107:3000"); // node address
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(buttonTAG, "HTTP POST Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
        printf("http request successfully sent.\n");
    } else {
        ESP_LOGE(buttonTAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
}


// Button interrupt handler -- add to queue
static void IRAM_ATTR gpio_isr_handler(void* arg){
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}


// GPIO init for LEDs
static void led_init() {
    gpio_pad_select_gpio(GREENPIN);  
    gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);
}

// Button interrupt init
static void hw_int_init() {
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    gpio_intr_enable(GPIO_INPUT_IO_1 );
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
}

// Button task -- initiate send ID and code task.
void button_task(){
  uint32_t io_num;
  while(1) {
     if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      xSemaphoreTake(mux, portMAX_DELAY);
      button_status = '1';
      xSemaphoreGive(mux);
      printf("Button pressed.\n");
      http_post();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

}

// LED task to light LED based on status
void led_task(){
  while(1) {
    switch(led_status){
      case '1' : // on, green
        gpio_set_level(GREENPIN, 1);
        //printf("Current state: %c \n",led_status);
        break;
      case'0':
        gpio_set_level(GREENPIN, 0);
        //printf("Current state: %c \n", led_status);
        break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


// Button interrupt handler -- add to queue

httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t led_server = NULL;
    httpd_config_t led_config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(ledTAG, "Starting server on port: '%d'", led_config.server_port);
    if (httpd_start(&led_server, &led_config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(ledTAG, "Registering URI handlers");
        httpd_register_uri_handler(led_server, &ctrl);
        return led_server;
    }

    ESP_LOGI(ledTAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}


void app_main() {

    // Create the semaphore to signal
    vSemaphoreCreateBinary( xSemaphore );
    // Mutex for current values when sending and during election
    mux = xSemaphoreCreateMutex();
 esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    app_wifi_initialise();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    led_init();
    hw_int_init();
    xTaskCreate(led_task, "set_gpio_task", 1024*2, NULL, configMAX_PRIORITIES-2, NULL);
    xTaskCreate(button_task, "button_task", 1024*2, NULL, configMAX_PRIORITIES-3, NULL);

}
