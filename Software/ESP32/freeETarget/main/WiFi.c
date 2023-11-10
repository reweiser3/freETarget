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
 * See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/network/esp_wifi.html
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
#include "serial_io.h"
#include "WiFi.h"

#define PORT                        CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT

static const char *TAG = "tcp_server";
wifi_init_config_t WiFi_config;
wifi_ap_config_t   WiFi_access_point;
wifi_sta_config_t  WiFi_station;


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
static void WiFi_init(void)
{

   esp_wifi_init(&WiFi_config);           // Initialize the configuration

   if ( json_wifi_ssid == 0 )             // The SSID is undefined
   {
/*
 * The target is an access point and broadcasts an SSID
 */
      strcpy(WiFi_access_point.ssid, names[json_name_id]);
      WiFi_access_point.password[0]     = 0;
      WiFi_access_point.ssid_len        = strlen(names[json_name_id]);
      WiFi_access_point.channel         = json_wifi_channel;
      WiFi_access_point.auth_mode       = WIFI_AUTH_OPEN;
      WiFi_access_point.ssid_hidden     = 0;          // Broadcast SSID
      WiFi_access_point.max_connection  = 4;          // Max number of stations allowed to connect in, max 10
      WiFi_access_point.beacon_interval = 1000;       // Beacon interval which should be multiples of 100
      WiFi_access_point.pairwise_cipher = 0;
      WiFi_access_point.ftm_responder   = 0;          // Enable FTM Responder mode
      WiFi_access_point.capable         = 1;
      WiFi_access_point.pmf_cfg         = 0;          // Configuration for Protected Management Frame
      esp_wifi_set_mode(WIFI_MODE_AP)
      esp_wifi_set_config(WIFI_MODE_AP, WiFi_access_point);
   }
/*
 * The target is a station and connects to an existing SSID
 */
   else
   {
      strcpy(WiFi_station.ssid, json_wifi_ssid);
      strcpy(WiFi_station.password, json_wifi_password);
      WiFi_station.scan_method = WIFI_FAST_SCAN;
      WiFi_station.bssid_set   = 0;
      WiFi_station.bssid[0]    = 0;                // MAC address of target AP Not Used
      WiFi_station.channel     = json_wifi_channel;// Channel of target AP. Set to 1~13 to scan starting from the specified channel before connecting to AP. If the channel of AP is unknown, set it to 0.*/
      WiFi_station.listen_interval = 3;            // Listen interval for ESP32 station to receive beacon when WIFI_PS_MAX_MODEM is set. Units: AP beacon intervals. Defaults to 3 if set to 0. */
      WiFi_station.sort_method =  WIFI_CONNECT_AP_BY_SIGNAL;    /**< sort the connect AP in the list by rssi or security mode */
      WiFi_station.threshold.rssi = 0;
      WiFi_station.threshold.authmode = OPEN;
      esp_wifi_set_mode(WIFI_MODE_STA)
      esp_wifi_set_config(WIFI_MODE_STA, WiFi_station);
      esp_wifi_start();                      // Start the WiFi
      esp_wifi_connect();
   }
/*
 *  All done
 */

   return;
}

/*****************************************************************************
 *
 * @function: WiFi_test()
 *
 * @brief:    Test the WiFI interface
 * 
 * @return:   None
 *
 ******************************************************************************
 *
 * 
 *******************************************************************************/
static void WiFi_test(void)
{

   WiFI_init();

/*
 *  All done
 */
    return;
}