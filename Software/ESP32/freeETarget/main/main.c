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
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "freETarget.h"
#include "json.h"
#include "timer.h"
#include "serial_io.h"

void app_main(void)
{

/*
 *  Start FreeETarget
 */
    freeETarget_init();

/*
 * Everything is ready, start the threads
 */
//   xTaskCreate(tcp_server_task,  "tcp_server", 4096, (void*)AF_INET, 5, NULL);
   xTaskCreate(freeETarget_json,        "json_task",                4096, NULL, 5, NULL);
   xTaskCreate(freeETarget_synchronous, "freeETarget_synchronous",  4096, NULL, 4, NULL);
   freeETarget_timer_init();
}
