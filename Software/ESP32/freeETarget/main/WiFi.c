/******************************************************************************
 * 
 * WiFi.c 
 * 
 * WiFi Driver for FreeETarget
 * 
 ******************************************************************************
 *
 * The WiFi driver supposts two modes of operation:
 *       1 - Access Point (AP) where the target provides the SSID
 *       2 - Station Mode (STA) where the target talks to a router with an SSID
 * 
 * The two modes are differenciated when the SSID stored in the configuration 
 * is defined (STA) or empty (AP).
 *
 * See: 
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/network/esp_wifi.html
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_netif.html
 * https://medium.com/@fatehsali517/how-to-connect-esp32-to-wifi-using-esp-idf-iot-development-framework-d798dc89f0d6
 * 
 *****************************************************************************/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "freETarget.h"
#include "json.h"
#include "diag_tools.h"
#include "WiFi.h"

#define PORT                        CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT

/*
 * Macros
 */
#define WIFI_CONNECTED_BIT BIT0           // we are connected to the AP with an IP
#define WIFI_FAIL_BIT      BIT1           // we failed to connect after the maximum amount of retries */
#define WIFI_MAX_RETRY     3              // Try 3x
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN

/*
 * Variables
 */
static wifi_config_t        WiFi_config;
static EventGroupHandle_t s_wifi_event_group;
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;
static int s_retry_num = 0;

/*
 * Private Functions
 */
void WiFi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

/*****************************************************************************
 *
 * @function: WiFi_init()
 *
 * @brief:    Initialize the WiFi Interface
 * 
 * @return:   None
 *
 ******************************************************************************
 *
 * The initialization determines if the target is a station
 * or an access point (AP) that provides the SSID to connect to.
 * 
 * Once that is done the appropriate configuration is made and the target enabled.
 * 
 *******************************************************************************/
void WiFi_init(void)
{
   ESP_ERROR_CHECK(esp_netif_init());
   ESP_ERROR_CHECK(esp_event_loop_create_default());
   
   if ( json_wifi_ssid[0] == 0 )             // The SSID is undefined
   {
      WiFi_AP_init();
   }
   else
   {
      WiFi_station_init();
   }

/*
 *  All done
 */
   return;
}

/*****************************************************************************
 *
 * @function: WiFi_AP_init()
 *
 * @brief:    Initialize the WiFi Interface as an access point
 * 
 * @return:   None
 *
 ******************************************************************************
 *
 * This function initializes the WiFi to act as an Access Point.
 * 
 * The target broadcasts an SSID and lets clients connect to it.
 * 
 *******************************************************************************/
void WiFi_AP_init(void)
{

   DLT(DLT_CRITICAL);
   printf("WiFi_AP_init\r\n");
    
   ESP_ERROR_CHECK(esp_netif_init());
   ESP_ERROR_CHECK(esp_event_loop_create_default());
   esp_netif_create_default_wifi_ap();

   wifi_init_config_t WiFi_init_config = WIFI_INIT_CONFIG_DEFAULT();
   ESP_ERROR_CHECK(esp_wifi_init(&WiFi_init_config));

   ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &WiFi_event_handler,
                                                        NULL,
                                                        NULL));

   strcpy((char*)&WiFi_config.ap.ssid, "FET-");
   strcat((char*)&WiFi_config.ap.ssid, names[json_name_id]);
   WiFi_config.ap.ssid_len = strlen(json_wifi_ssid);
   WiFi_config.ap.channel  = json_wifi_channel;
   strcpy((char*)&WiFi_config.ap.password, json_wifi_pwd);
   WiFi_config.ap.max_connection = 4;
   if ( json_wifi_pwd[0] == 0 )
   {
      WiFi_config.ap.authmode = WIFI_AUTH_OPEN;
   }
   else
   {
      WiFi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
   }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &WiFi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("wifi_init_softap finished. SSID:FET-%s password:%s channel:%d\r\n",
             names[json_name_id], json_wifi_pwd, json_wifi_channel);

/*
 * Ready to go
 */
   return;
}


/*****************************************************************************
 *
 * @function: WiFi_station_init()
 *
 * @brief:    Initialize the WiFi Interface as a station
 * 
 * @return:   None
 *
 ******************************************************************************
 *
 * This function initializes the WiFi to act as an station
 * 
 * The target connects to an SSID and lets clients connect to it.
 * 
 * See: https://github.com/espressif/esp-idf/blob/v4.3/examples/wifi/getting_started/station/main/station_example_main.c
 * 
 *******************************************************************************/
void WiFi_station_init(void)
{
   DLT(DLT_CRITICAL);
   printf("WiFi_station_init\r\n");

   s_wifi_event_group = xEventGroupCreate();
   ESP_ERROR_CHECK(esp_netif_init());

   ESP_ERROR_CHECK(esp_event_loop_create_default());
   esp_netif_create_default_wifi_sta();

   wifi_init_config_t   WiFi_init_config = WIFI_INIT_CONFIG_DEFAULT();
   ESP_ERROR_CHECK(esp_wifi_init(&WiFi_init_config));           // Initialize the configuration

   ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFi_event_handler, NULL, &instance_any_id));
   ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFi_event_handler, NULL, &instance_got_ip));

   strcpy((char*)&WiFi_config.sta.ssid, "TargetRange");
   strcpy((char*)&WiFi_config.sta.password, json_wifi_pwd);
   WiFi_config.sta.password[0] = 0;
   WiFi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
   WiFi_config.sta.pmf_cfg.capable = true;
   WiFi_config.sta.pmf_cfg.required = false;
   esp_wifi_set_mode(WIFI_MODE_STA);
   esp_wifi_set_config(WIFI_IF_STA, &WiFi_config);
   esp_wifi_start();                      // Start the WiFi

/*
 * Wait here for an event to occur
 */
   EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

   /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        printf( "connected to ap SSID:%s password:%s",
                 json_wifi_ssid, json_wifi_pwd);
    } else if (bits & WIFI_FAIL_BIT) {
        printf("Failed to connect to SSID:%s, password:%s",
                 json_wifi_ssid, json_wifi_pwd);
    } else {
        printf("UNEXPECTED EVENT");
    }
/*
 *  All done
 */
   return;
}


/*****************************************************************************
 *
 * @function:WiFi_event_handler
 *
 * @brief:   Manage events coming from the FreeRTOS event handler
 * 
 * @return:   None
 *
 ******************************************************************************
 *
 * The initialization determines if the target is a station
 * or an access point (AP) that provides the SSID to connect to.
 * 
 * Once that is done the appropriate configuration is made and the target enabled.
 * 
 *******************************************************************************/
void WiFi_event_handler
(
   void* arg, 
   esp_event_base_t event_base,
   int32_t event_id, 
   void* event_data
)
{
/*
 * I am a station
 */
   if ( event_base == WIFI_EVENT )
   {
      if ( event_id == WIFI_EVENT_STA_START)
      {
         esp_wifi_connect();
      }

      if ( event_id == WIFI_EVENT_STA_DISCONNECTED )
      {
         if (s_retry_num < WIFI_MAX_RETRY)
         {
            esp_wifi_connect();
            s_retry_num++;
         }
         else
         {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
         }
      }
   }   
   
   if ( event_base == IP_EVENT )
   {
      if ( event_id == IP_EVENT_STA_GOT_IP )
      {
         ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
         s_retry_num = 0;
         xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
      }
   }

/*
 * I am an access point
 */
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
      wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
    } 
   
   if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
   {
      wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
   }

/*
 * All done, return
 */
   return;
}