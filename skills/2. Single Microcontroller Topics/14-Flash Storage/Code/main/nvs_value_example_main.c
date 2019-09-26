/* Non-Volatile Storage (NVS) Read and Write a Value - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "nvs.h"

static char* echo_task()
{
    uint8_t *data = (uint8_t *) malloc(1024);
    static char arr[11];
    int i = 0;
    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(UART_NUM_0, data, 1024, 20 / portTICK_RATE_MS); // data has to be unsigned
          if (len > 0){
           printf("Input: %c \n", *data);
           //char temp = (const char*) data;
           char temp = *data;
           printf("input char is %c \n", temp);
           arr[i] = temp;
           i++;
           if (i == 12){
            
             return arr; // when the declared arr is called it auto matically set to a pointer
           }
      }

    }
}

void read(char index){
  nvs_handle my_handle;
  esp_err_t err;
  nvs_open("storage", NVS_READWRITE, &my_handle);
        int8_t chara; // value will default to 0, if not set yet in NVS
        char string[11];
        size_t size = 11;
        //esp_err_tnvs_set_u8(nvs_handlehandle, const char *key, uint8_t value)
        switch(index){
          case('0'):
          err = nvs_get_i8(my_handle, "0", &chara);
          string[0] = chara;
          err = nvs_get_i8(my_handle, "1", &chara);
          string[1] = chara;
          err = nvs_get_i8(my_handle, "2", &chara);
          string[2] = chara;
          err = nvs_get_i8(my_handle, "3", &chara);
          string[3] = chara;
          err = nvs_get_i8(my_handle, "4", &chara);
          string[4] = chara;
          err = nvs_get_i8(my_handle, "5", &chara);
          string[5] = chara;
          err = nvs_get_i8(my_handle, "6", &chara);
          string[6] = chara;
          err = nvs_get_i8(my_handle, "7", &chara);
          string[7] = chara;
          err = nvs_get_i8(my_handle, "8", &chara);
          string[8] = chara;
          err = nvs_get_i8(my_handle, "9", &chara);
          string[9] = chara;
          string[10] = '\0';
          printf(" The string stored in the entered index is: %s \n", string);
          break;
          case('1'):
          err = nvs_get_i8(my_handle, "10", &chara);
          string[0] = chara;
          err = nvs_get_i8(my_handle, "11", &chara);
          string[1] = chara;
          err = nvs_get_i8(my_handle, "12", &chara);
          string[2] = chara;
          err = nvs_get_i8(my_handle, "13", &chara);
          string[3] = chara;
          err = nvs_get_i8(my_handle, "14", &chara);
          string[4] = chara;
          err = nvs_get_i8(my_handle, "15", &chara);
          string[5] = chara;
          err = nvs_get_i8(my_handle, "16", &chara);
          string[6] = chara;
          err = nvs_get_i8(my_handle, "17", &chara);
          string[7] = chara;
          err = nvs_get_i8(my_handle, "18", &chara);
          string[8] = chara;
          err = nvs_get_i8(my_handle, "19", &chara);
          string[9] = chara;
          string[10] = '\0';
          printf(" The string stored in the entered index is: %s \n", string);
          break;
          case('2'):
          err = nvs_get_i8(my_handle, "20", &chara);
          string[0] = chara;
          err = nvs_get_i8(my_handle, "21", &chara);
          string[1] = chara;
          err = nvs_get_i8(my_handle, "22", &chara);
          string[2] = chara;
          err = nvs_get_i8(my_handle, "23", &chara);
          string[3] = chara;
          err = nvs_get_i8(my_handle, "24", &chara);
          string[4] = chara;
          err = nvs_get_i8(my_handle, "25", &chara);
          string[5] = chara;
          err = nvs_get_i8(my_handle, "26", &chara);
          string[6] = chara;
          err = nvs_get_i8(my_handle, "27", &chara);
          string[7] = chara;
          err = nvs_get_i8(my_handle, "28", &chara);
          string[8] = chara;
          err = nvs_get_i8(my_handle, "29", &chara);
          string[9] = chara;
          string[10] = '\0';
          printf("The string stored in the entered index is: %s \n", string);
          break;
          case('3'):
          err = nvs_get_i8(my_handle, "30", &chara);
          string[0] = chara;
          err = nvs_get_i8(my_handle, "31", &chara);
          string[1] = chara;
          err = nvs_get_i8(my_handle, "32", &chara);
          string[2] = chara;
          err = nvs_get_i8(my_handle, "33", &chara);
          string[3] = chara;
          err = nvs_get_i8(my_handle, "34", &chara);
          string[4] = chara;
          err = nvs_get_i8(my_handle, "35", &chara);
          string[5] = chara;
          err = nvs_get_i8(my_handle, "36", &chara);
          string[6] = chara;
          err = nvs_get_i8(my_handle, "37", &chara);
          string[7] = chara;
          err = nvs_get_i8(my_handle, "38", &chara);
          string[8] = chara;
          err = nvs_get_i8(my_handle, "39", &chara);
          string[9] = chara;
          string[10] = '\0';
          printf(" The string stored in the entered index is: %s \n", string);
          break;
          case('4'):
          err = nvs_get_i8(my_handle, "40", &chara);
          string[0] = chara;
          err = nvs_get_i8(my_handle, "41", &chara);
          string[1] = chara;
          err = nvs_get_i8(my_handle, "42", &chara);
          string[2] = chara;
          err = nvs_get_i8(my_handle, "43", &chara);
          string[3] = chara;
          err = nvs_get_i8(my_handle, "44", &chara);
          string[4] = chara;
          err = nvs_get_i8(my_handle, "45", &chara);
          string[5] = chara;
          err = nvs_get_i8(my_handle, "46", &chara);
          string[6] = chara;
          err = nvs_get_i8(my_handle, "47", &chara);
          string[7] = chara;
          err = nvs_get_i8(my_handle, "48", &chara);
          string[8] = chara;
          err = nvs_get_i8(my_handle, "49", &chara);
          string[9] = chara;
          string[10] = '\0';
          printf(" The string stored in the entered index is: %s \n", string);
          break;
          case('5'):
          err = nvs_get_i8(my_handle, "50", &chara);
          string[0] = chara;
          err = nvs_get_i8(my_handle, "51", &chara);
          string[1] = chara;
          err = nvs_get_i8(my_handle, "52", &chara);
          string[2] = chara;
          err = nvs_get_i8(my_handle, "53", &chara);
          string[3] = chara;
          err = nvs_get_i8(my_handle, "54", &chara);
          string[4] = chara;
          err = nvs_get_i8(my_handle, "55", &chara);
          string[5] = chara;
          err = nvs_get_i8(my_handle, "56", &chara);
          string[6] = chara;
          err = nvs_get_i8(my_handle, "57", &chara);
          string[7] = chara;
          err = nvs_get_i8(my_handle, "58", &chara);
          string[8] = chara;
          err = nvs_get_i8(my_handle, "59", &chara);
          string[9] = chara;
          string[10] = '\0';
          printf(" The string stored in the entered index is: %s \n", string);
          break;
           case('6'):
          err = nvs_get_i8(my_handle, "60", &chara);
          string[0] = chara;
          err = nvs_get_i8(my_handle, "61", &chara);
          string[1] = chara;
          err = nvs_get_i8(my_handle, "62", &chara);
          string[2] = chara;
          err = nvs_get_i8(my_handle, "63", &chara);
          string[3] = chara;
          err = nvs_get_i8(my_handle, "64", &chara);
          string[4] = chara;
          err = nvs_get_i8(my_handle, "65", &chara);
          string[5] = chara;
          err = nvs_get_i8(my_handle, "66", &chara);
          string[6] = chara;
          err = nvs_get_i8(my_handle, "67", &chara);
          string[7] = chara;
          err = nvs_get_i8(my_handle, "68", &chara);
          string[8] = chara;
          err = nvs_get_i8(my_handle, "69", &chara);
          string[9] = chara;
          string[10] = '\0';
          printf(" The string stored in the entered index is: %s \n", string);
          break;
          case('7'):
          err = nvs_get_i8(my_handle, "70", &chara);
          string[0] = chara;
          err = nvs_get_i8(my_handle, "71", &chara);
          string[1] = chara;
          err = nvs_get_i8(my_handle, "72", &chara);
          string[2] = chara;
          err = nvs_get_i8(my_handle, "73", &chara);
          string[3] = chara;
          err = nvs_get_i8(my_handle, "74", &chara);
          string[4] = chara;
          err = nvs_get_i8(my_handle, "75", &chara);
          string[5] = chara;
          err = nvs_get_i8(my_handle, "76", &chara);
          string[6] = chara;
          err = nvs_get_i8(my_handle, "77", &chara);
          string[7] = chara;
          err = nvs_get_i8(my_handle, "78", &chara);
          string[8] = chara;
          err = nvs_get_i8(my_handle, "79", &chara);
          string[9] = chara;
          string[10] = '\0';
          printf(" The string stored in the entered index is: %s \n", string);
          break;
        }
                printf("completed \n");
}


void uart_config(){
    printf("Please enter 10 characters. \n");
    uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_0, &uart_config);
  uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);
}


void app_main()
{
  
  uart_config();
  char *tt = echo_task();
  char index = tt[0];
  char b = tt[1];
  char c = tt[2];
  char d = tt[3];
  char e = tt[4];
  char f = tt[5];
  char g = tt[6];
  char h = tt[7];
  char i = tt[8];
  char j = tt[9];
  char k = tt[10];
  char l = tt[11];
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");
        

        // Write
        int8_t chara = b;
        //char string[] = {'a','b','a'};
        //const char string[] = {b,c,d,e,f,g,h,i,j,k};
        printf("Updating restart counter in NVS ... ");
        //restart_counter++;
        switch(index){
        case('0'):
        err = nvs_set_i8(my_handle, "0", chara);
        chara = c;
        err = nvs_set_i8(my_handle, "1", chara);
        chara = d;
        err = nvs_set_i8(my_handle, "2", chara);
        chara = e;
        err = nvs_set_i8(my_handle, "3", chara);
        chara = f;
        err = nvs_set_i8(my_handle, "4", chara);
        chara = g;
        err = nvs_set_i8(my_handle, "5", chara);
        chara = h;
        err = nvs_set_i8(my_handle, "6", chara);
        chara = i;
        err = nvs_set_i8(my_handle, "7", chara);
        chara = j;
        err = nvs_set_i8(my_handle, "8", chara);
        chara = k;
        err = nvs_set_i8(my_handle, "9", chara);
        break;
        case('1'):
        err = nvs_set_i8(my_handle, "10", chara);
        chara = c;
        err = nvs_set_i8(my_handle, "11", chara);
        chara = d;
        err = nvs_set_i8(my_handle, "12", chara);
        chara = e;
        err = nvs_set_i8(my_handle, "13", chara);
        chara = f;
        err = nvs_set_i8(my_handle, "14", chara);
        chara = g;
        err = nvs_set_i8(my_handle, "15", chara);
        chara = h;
        err = nvs_set_i8(my_handle, "16", chara);
        chara = i;
        err = nvs_set_i8(my_handle, "17", chara);
        chara = j;
        err = nvs_set_i8(my_handle, "18", chara);
        chara = k;
        err = nvs_set_i8(my_handle, "19", chara);
        break;
        case('2'):
        err = nvs_set_i8(my_handle, "20", chara);
        chara = c;
        err = nvs_set_i8(my_handle, "21", chara);
        chara = d;
        err = nvs_set_i8(my_handle, "22", chara);
        chara = e;
        err = nvs_set_i8(my_handle, "23", chara);
        chara = f;
        err = nvs_set_i8(my_handle, "24", chara);
        chara = g;
        err = nvs_set_i8(my_handle, "25", chara);
        chara = h;
        err = nvs_set_i8(my_handle, "26", chara);
        chara = i;
        err = nvs_set_i8(my_handle, "27", chara);
        chara = j;
        err = nvs_set_i8(my_handle, "28", chara);
        chara = k;
        err = nvs_set_i8(my_handle, "29", chara);
        break;
        case('3'):
        err = nvs_set_i8(my_handle, "30", chara);
        chara = c;
        err = nvs_set_i8(my_handle, "31", chara);
        chara = d;
        err = nvs_set_i8(my_handle, "32", chara);
        chara = e;
        err = nvs_set_i8(my_handle, "33", chara);
        chara = f;
        err = nvs_set_i8(my_handle, "34", chara);
        chara = g;
        err = nvs_set_i8(my_handle, "35", chara);
        chara = h;
        err = nvs_set_i8(my_handle, "36", chara);
        chara = i;
        err = nvs_set_i8(my_handle, "37", chara);
        chara = j;
        err = nvs_set_i8(my_handle, "38", chara);
        chara = k;
        err = nvs_set_i8(my_handle, "39", chara);
        break;
        case('4'):
        err = nvs_set_i8(my_handle, "40", chara);
        chara = c;
        err = nvs_set_i8(my_handle, "41", chara);
        chara = d;
        err = nvs_set_i8(my_handle, "42", chara);
        chara = e;
        err = nvs_set_i8(my_handle, "43", chara);
        chara = f;
        err = nvs_set_i8(my_handle, "44", chara);
        chara = g;
        err = nvs_set_i8(my_handle, "45", chara);
        chara = h;
        err = nvs_set_i8(my_handle, "46", chara);
        chara = i;
        err = nvs_set_i8(my_handle, "47", chara);
        chara = j;
        err = nvs_set_i8(my_handle, "48", chara);
        chara = k;
        err = nvs_set_i8(my_handle, "49", chara);
        break;
        case('5'):
        err = nvs_set_i8(my_handle, "50", chara);
        chara = c;
        err = nvs_set_i8(my_handle, "51", chara);
        chara = d;
        err = nvs_set_i8(my_handle, "52", chara);
        chara = e;
        err = nvs_set_i8(my_handle, "53", chara);
        chara = f;
        err = nvs_set_i8(my_handle, "54", chara);
        chara = g;
        err = nvs_set_i8(my_handle, "55", chara);
        chara = h;
        err = nvs_set_i8(my_handle, "56", chara);
        chara = i;
        err = nvs_set_i8(my_handle, "57", chara);
        chara = j;
        err = nvs_set_i8(my_handle, "58", chara);
        chara = k;
        err = nvs_set_i8(my_handle, "59", chara);
        break;
        case('6'):
        err = nvs_set_i8(my_handle, "60", chara);
        chara = c;
        err = nvs_set_i8(my_handle, "61", chara);
        chara = d;
        err = nvs_set_i8(my_handle, "62", chara);
        chara = e;
        err = nvs_set_i8(my_handle, "63", chara);
        chara = f;
        err = nvs_set_i8(my_handle, "64", chara);
        chara = g;
        err = nvs_set_i8(my_handle, "65", chara);
        chara = h;
        err = nvs_set_i8(my_handle, "66", chara);
        chara = i;
        err = nvs_set_i8(my_handle, "67", chara);
        chara = j;
        err = nvs_set_i8(my_handle, "68", chara);
        chara = k;
        err = nvs_set_i8(my_handle, "69", chara);
        break;
        case('7'):
        err = nvs_set_i8(my_handle, "70", chara);
        chara = c;
        err = nvs_set_i8(my_handle, "71", chara);
        chara = d;
        err = nvs_set_i8(my_handle, "72", chara);
        chara = e;
        err = nvs_set_i8(my_handle, "73", chara);
        chara = f;
        err = nvs_set_i8(my_handle, "74", chara);
        chara = g;
        err = nvs_set_i8(my_handle, "75", chara);
        chara = h;
        err = nvs_set_i8(my_handle, "76", chara);
        chara = i;
        err = nvs_set_i8(my_handle, "77", chara);
        chara = j;
        err = nvs_set_i8(my_handle, "78", chara);
        chara = k;
        err = nvs_set_i8(my_handle, "79", chara);
        break;
        }
       // err = nvs_set_str(my_handle,"two",string);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        read(l);
}
        // Close
        nvs_close(my_handle);
    
    printf("\n");
    esp_restart();
}
