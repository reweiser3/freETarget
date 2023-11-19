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
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "freETarget.h"
#include "json.h"
#include "diag_tools.h"
#include "WiFi.h"

#define PORT                        CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT

wifi_config_t        WiFi_config;
wifi_init_config_t   WiFi_init_config;

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
   
   if ( json_wifi_ssid == 0 )             // The SSID is undefined
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
   esp_netif_create_default_wifi_ap();
   
   esp_wifi_init(&WiFi_init_config);           // Initialize the configuration
   strcpy((char*)&WiFi_config.ap.ssid, names[json_name_id]);
   WiFi_config.ap.password[0]      = 0;
   WiFi_config.ap.ssid_len         = strlen(names[json_name_id]);
   WiFi_config.ap.channel          = json_wifi_channel;
   WiFi_config.ap.authmode         = WIFI_AUTH_OPEN;
   WiFi_config.ap.ssid_hidden      = 0;          // Broadcast SSID
   WiFi_config.ap.max_connection   = 4;          // Max number of stations allowed to connect in, max 10
   WiFi_config.ap.beacon_interval  = 1000;       // Beacon interval which should be multiples of 100
   WiFi_config.ap.pairwise_cipher  = 0;
   WiFi_config.ap.ftm_responder    = 0;          // Enable FTM Responder mode
   WiFi_config.ap.pmf_cfg.capable  = 0;          // Configuration for Protected Management Frame
   WiFi_config.ap.pmf_cfg.required = 0;          // Configuration for Protected Management Frame
   esp_wifi_set_mode(WIFI_MODE_AP);
   esp_wifi_set_config(WIFI_MODE_AP, &WiFi_config);
   
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
 *******************************************************************************/
void WiFi_station_init(void)
{
   DLT(DLT_CRITICAL);
   printf("WiFi_station_init\r\n");

   ESP_ERROR_CHECK(esp_netif_init());
   esp_netif_create_default_wifi_sta();

   esp_wifi_init(&WiFi_init_config);           // Initialize the configuration
   strcpy((char*)&WiFi_config.sta.ssid, json_wifi_ssid);
   strcpy((char*)&WiFi_config.sta.password, json_wifi_pwd);
   WiFi_config.sta.scan_method = WIFI_FAST_SCAN;
   WiFi_config.sta.bssid_set   = 0;
   WiFi_config.sta.bssid[0]    = 0;                // MAC address of target AP Not Used
   WiFi_config.sta.channel     = json_wifi_channel;// Channel of target AP. Set to 1~13 to scan starting from the specified channel before connecting to AP. If the channel of AP is unknown, set it to 0.*/
   WiFi_config.sta.listen_interval = 3;            // Listen interval for ESP32 station to receive beacon when WIFI_PS_MAX_MODEM is set. Units: AP beacon intervals. Defaults to 3 if set to 0. */
   WiFi_config.sta.sort_method =  WIFI_CONNECT_AP_BY_SIGNAL;    /**< sort the connect AP in the list by rssi or security mode */
   WiFi_config.sta.threshold.rssi = 0;
   WiFi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
   esp_wifi_set_mode(WIFI_MODE_STA);
   esp_wifi_set_config(WIFI_MODE_STA, &WiFi_config);
   esp_wifi_start();                      // Start the WiFi
   esp_wifi_connect();
/*
 *  All done
 */

   return;
}

