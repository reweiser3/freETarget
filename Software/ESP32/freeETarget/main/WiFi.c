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
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "freETarget.h"
#include "json.h"
#include "diag_tools.h"
#include "WiFi.h"

#define PORT                        1090
#define KEEPALIVE_IDLE              true
#define KEEPALIVE_INTERVAL          100
#define KEEPALIVE_COUNT             50
#define MAX_SOCKETS                 4     // Allow for four sockets

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
static int socket_list[MAX_SOCKETS];                // Space to remember four sockets

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
   
/* 
 * Initialize the WiFI
 */
   if ( json_wifi_ssid[0] == 0 )             // The SSID is undefined
   {
      WiFi_AP_init();
   }
   else
   {
      WiFi_station_init();
   }

/*
 * Initialize the server
 */


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
    if ( DLT(DLT_INFO) )
    {
        if (bits & WIFI_CONNECTED_BIT)
        {
            printf( "Connected to ap SSID:%s password:%s",
                     json_wifi_ssid, json_wifi_pwd);
        } 
        else if (bits & WIFI_FAIL_BIT) 
            {
                printf("Failed to connect to SSID:%s, password:%s",
                     json_wifi_ssid, json_wifi_pwd);
            }
            else
            {
                printf("UNEXPECTED EVENT");
            }
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


/*****************************************************************************
 *
 * @function: tcp_server_io()
 *
 * @brief: Transmit data in and out of the target
 * 
 * @return: None
 *
 ******************************************************************************
 *
 * Synchronous task called from freeRTOS to interrogate the TCPIP stack and 
 * accept calls from clients
 * 
 *******************************************************************************/
static void tcp_server_io
(
    const int sock              // Socket identifier
)
{
    int len;
    char rx_buffer[128];
    int  to_write;

    do 
    {
/*
 *  In from TCPIP
 */
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len >= 0)
        {
 //           len -= tcpip_to_serial(rx_buffer, len);
        }
    }
    while ( len > 0 );

/*
 * Out to TCPIP
 */      
    while (1)
    {
//        to_write = serial_to_tcpip(rx_buffer,  sizeof(rx_buffer));      
        if ( to_write == 0 )
        {
            break;
        }
        while (to_write > 0)
        {
            to_write -= send(sock, rx_buffer + (len - to_write), to_write, 0);
        }
    }
/*
 *  All done
 */
    return;
}

/*****************************************************************************
 *
 * @function: tcp_server_task()
 *
 * @brief: Synchorous task to manage the TCPIP Stack
 * 
 * @return: Never
 *
 ******************************************************************************
 *
 * Synchronous task called from freeRTOS to interrogate the TCPIP stack and 
 * accept calls from clients
 * 
 *******************************************************************************/
void WiFi_tcp_server_task(void *pvParameters)
{
   char addr_str[128];
   int ip_protocol = 0;
   int keepAlive = 1;
   int keepIdle = KEEPALIVE_IDLE;
   int keepInterval = KEEPALIVE_INTERVAL;
   int keepCount = KEEPALIVE_COUNT;
   struct sockaddr_storage dest_addr;
   int listen_sock;
   int option = 1;
   struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
   socklen_t addr_len = sizeof(source_addr);
   int sock;
   int i;

   DLT(DLT_CRITICAL);
   printf("WiFi_tcp_server_task()\r\n");

/*
 * Start the server
 */
   for (i=0; i != MAX_SOCKETS; i++)
   {
      socket_list[i] = -1;
   }

   struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
   dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
   dest_addr_ip4->sin_family = AF_INET;
   dest_addr_ip4->sin_port = htons(PORT);
   ip_protocol = IPPROTO_IP;

   listen_sock = socket(AF_INET, SOCK_STREAM, ip_protocol);
   if (listen_sock < 0) 
   {
      DLT(DLT_CRITICAL);
      printf("Unable to create socket: errno %d\r\n", errno);
      vTaskDelete(NULL);
      return;
   }

   option = 1;
   setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
   bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
   listen(listen_sock, 1);
   
/*
 * Wait here for sockets to be connected
 */
   while (1)
   {
      sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
      if (sock > 0)
      {
        if ( DLT(DLT_CRITICAL) )
        {
            printf("New connection");
        }
        for (i= 0; i != MAX_SOCKETS; i++ )
        {
            if  (socket_list[i] == -1 )
            {
                socket_list[i] = sock;
                break;
            }
        }

/*
 * Set tcp keepalive option
 */
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        if ( DLT(DLT_CRITICAL) )
        {         
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
            printf("Socket accepted ip address: %s", addr_str);
        }
      }
      vTaskDelay(ONE_SECOND/4);
   }
}

#if(0)
/*===============================================*/
/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


#define PORT                        CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT

static const char *TAG = "example";

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            int to_write = len;
            while (to_write > 0) {
                int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                if (written < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                }
                to_write -= written;
            }
        }
    } while (len > 0);
}

void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }
#ifdef CONFIG_EXAMPLE_IPV6
    else if (addr_family == AF_INET6) {
        struct sockaddr_in6 *dest_addr_ip6 = (struct sockaddr_in6 *)&dest_addr;
        bzero(&dest_addr_ip6->sin6_addr.un, sizeof(dest_addr_ip6->sin6_addr.un));
        dest_addr_ip6->sin6_family = AF_INET6;
        dest_addr_ip6->sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }
#endif

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#ifdef CONFIG_EXAMPLE_IPV6
        else if (source_addr.ss_family == PF_INET6) {
            inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET6, 5, NULL);
#endif
}
#endif