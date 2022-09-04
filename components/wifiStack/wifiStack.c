/*
    MIT License

    Copyright (c) [2022] [Parth Yatin Temkar]

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Author: Parth Yatin Temkar 
*/

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_err.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "nvs_flash.h"


#define STA_SSID            CONFIG_STA_SSID
#define STA_PASS            CONFIG_STA_PASSWORD
#define MAX_RETRY_ATTEMPTS  CONFIG_STA_MAX_RETRY_COUNTS 
#define INFINITE_RETRY      CONFIG_STA_INFINIT_RETRY


#define AP_SSID             CONFIG_AP_SSID
#define AP_PASS             CONFIG_AP_PASSWORD
#define MAX_AP_CONNECTIONS  CONFIG_AP_MAX_CONNECTIONS



static const char *TAG = "WiFi_Stack";

static EventGroupHandle_t wifiEventGroup;

#define  CONNECTED_BIT      BIT0
#define  DISCONNECT_BIT     BIT1
#define  GOT_IP_BIT         BIT2



static esp_netif_t *apNetif;
wifi_config_t wifiConfigStation;
wifi_config_t wifiConfigAccessPoint;

static int retryCounts = 0;
uint8_t infiniteRetry = 0;
uint8_t configuredStation = 0;

// Callbacks 
#if defined(CONFIG_STA_CONNECTED_CB)
void (*connectedStationCB)();
#endif

#if defined(CONFIG_STA_DISCONNECTED_CB)
void (*disconnectedStationCB)();
#endif

#if defined(CONFIG_STA_RECONNECTING_CB)
void (*reconnectingStationCB)();
#endif

#if defined(CONFIG_GOT_IP_CB)
void (*gotipStationCB)();
#endif

#if defined(CONFIG_AP_CONNECTED_CB)
void (*connectedAccessPointCB)();
#endif

#if defined(CONFIG_AP_DISCONNECTED_CB)
void (*disconnectedAccessPointCB)();
#endif

// Event Handler 
static void wifiEventHandle(void *args, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    switch(eventId)
    {
        case WIFI_EVENT_STA_START:
        {
            ESP_LOGI(TAG, "WiFi Station Starting..");
            ESP_ERROR_CHECK(esp_wifi_connect());
        }
        break;

        case WIFI_EVENT_STA_CONNECTED:
        {
            ESP_LOGI(TAG, "WiFi Station Connected..");
            xEventGroupSetBits(wifiEventGroup, CONNECTED_BIT);
            retryCounts = 0;
            #if defined(CONFIG_STA_CONNECTED_CB)
            if(connectedStationCB != NULL)
            {
                connectedStationCB();
            }
            #endif
        }   
           
        break;

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            #if defined(CONFIG_STA_RECONNECTING_CB)
            if(reconnectingStationCB != NULL)
            {
                reconnectingStationCB(); 
            }
            #endif

            if((retryCounts < MAX_RETRY_ATTEMPTS) && (!infiniteRetry))
            {
                ESP_LOGW(TAG, "reconnecting %d", retryCounts);
                esp_wifi_connect();
                retryCounts++;
            }
            else if(infiniteRetry)
            {
                ESP_LOGW(TAG, "Infinite reconnecting...");
                esp_wifi_connect();
                
            }
            else 
            {
                xEventGroupSetBits(wifiEventGroup, DISCONNECT_BIT);   
                
                #if defined(CONFIG_STA_DISCONNECTED_CB)            
                disconnectedStationCB();
                #endif

                ESP_LOGE(TAG, "WiFi Station Disconnected..");
            }
        }

        break;

        case IP_EVENT_STA_GOT_IP:
        {
            #if defined(CONFIG_GOT_IP_CB)
            if(gotipStationCB != NULL)
            {
                gotipStationCB();
            }
            #endif

            ip_event_got_ip_t *event = (ip_event_got_ip_t*) eventData;
            ESP_LOGI(TAG, "WiFi Station Got IP: " IPSTR,IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(wifiEventGroup, GOT_IP_BIT);
        }
        break;

       // Access Point 
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "WiFi AccessPoint Starting..");
        break;

        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG, "WiFi AccessPoint Stopping..");
            esp_netif_destroy(apNetif);
        break;

        case WIFI_EVENT_AP_STACONNECTED:
        {
            ESP_LOGI(TAG, "WiFi AccessPoint Connected..");
            wifi_event_ap_staconnected_t *apConnectedEvent = (wifi_event_ap_staconnected_t *)eventData;
            ESP_LOGI(TAG, "WiFi AccessPoint station " MACSTR " joined, AID=%d", MAC2STR(apConnectedEvent->mac), apConnectedEvent->aid);
            xEventGroupSetBits(wifiEventGroup, CONNECTED_BIT);
            
            #if defined(CONFIG_AP_CONNECTED_CB)
            connectedAccessPointCB();
            #endif
        }
        break;

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            ESP_LOGI(TAG, "WiFi AccessPoint Disconnected..");
            wifi_event_ap_stadisconnected_t *apDisconnectedEvent = (wifi_event_ap_stadisconnected_t *)eventData;
            ESP_LOGI(TAG, "WiFi AccessPoint station " MACSTR " leave, AID=%d", MAC2STR(apDisconnectedEvent->mac), apDisconnectedEvent->aid); 
            xEventGroupSetBits(wifiEventGroup, DISCONNECT_BIT);

            #if defined(CONFIG_AP_DISCONNECTED_CB)
            disconnectedAccessPointCB();
            #endif
        }  
        break;

        default: break;

    }
}

// WiFi Functions 
esp_err_t initWifi(void)
{
    esp_err_t err;
    err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
   
    if(err != ESP_OK)
        return err;

    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&wifiInitConfig);
    if(err != ESP_OK)
        return err;

    
    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandle, NULL);
    if(err != ESP_OK)
        return err;

    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandle, NULL);
    if(err != ESP_OK)
        return err;

    
    err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if(err != ESP_OK)
        return err;

    return ESP_OK;

}

// esp_err_t startWifiStation(const char* _ssid, const char* _password)

esp_err_t startWifiStation(const char* _ssid, const char* _password)
{
    esp_err_t err;
   if((_ssid == NULL) && (_password == NULL) && (STA_SSID != NULL) && (STA_PASS != NULL))
    {
        _ssid = STA_SSID;
        _password = STA_PASS;
    }

    
    infiniteRetry = INFINITE_RETRY;
    wifiEventGroup = xEventGroupCreate();

    //wifi_config_t wifiConfigStation;
    memset(&wifiConfigStation, 0, sizeof(wifi_config_t));
    strncpy((char *) wifiConfigStation.sta.ssid, _ssid, sizeof(wifiConfigStation.sta.ssid) - 1);
    strncpy((char *) wifiConfigStation.sta.password, _password, sizeof(wifiConfigStation.sta.password) - 1);
    
    //wifiConfigStation.sta.ssid = _ssid;
   // wifiConfigStation.sta.password = _password;

    configuredStation = 1;
   
    esp_netif_create_default_wifi_sta();
   

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if(err != ESP_OK)
        return err;

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfigStation);
    if(err != ESP_OK)
        return err;

    err = esp_wifi_start();
    if(err != ESP_OK)
        return err;

    
    return ESP_OK;
}


esp_err_t startWifiAccessPoint  (const char* _ssid, const char* _password)
{
    if(_ssid == NULL && _password == NULL)
    {
        _ssid = AP_SSID;
        _password = AP_PASS;
    }

    esp_err_t err;
    wifiEventGroup = xEventGroupCreate();
    memset(&wifiConfigAccessPoint, 0, sizeof(wifi_config_t));
    strncpy((char*) wifiConfigAccessPoint.ap.ssid, _ssid, sizeof(wifiConfigAccessPoint.ap.ssid) - 1);
    strncpy((char*) wifiConfigAccessPoint.ap.password, _password, sizeof(wifiConfigAccessPoint.ap.password) - 1);

    wifiConfigAccessPoint.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifiConfigAccessPoint.ap.max_connection = MAX_AP_CONNECTIONS;
    wifiConfigAccessPoint.ap.channel = 1;
    wifiConfigAccessPoint.ap.beacon_interval = 100;

    apNetif = esp_netif_create_default_wifi_ap();
    assert(apNetif);

    err = esp_wifi_set_mode(WIFI_MODE_AP);
    if(err != ESP_OK)
        return err;


    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifiConfigAccessPoint);
    if(err != ESP_OK)
        return err;

    err = esp_wifi_start();
    if(err != ESP_OK)
        return err;

    return ESP_OK;
}

esp_err_t waitForStaAPConnected(void)
{
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, CONNECTED_BIT | DISCONNECT_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    
    if(bits & CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "WiFi Connected BIT SET");
        return ESP_OK;
    }
    else if (bits & DISCONNECT_BIT)
    {
        ESP_LOGW(TAG, "WiFi Disconnected BIT SET");
        return -1;
    }
    else 
    {
        ESP_LOGE(TAG, "Unexpected Error In WiFi BITS");
        return -1;
    }

    return -1;
}


esp_err_t waitForGotIpAddress(void)
{
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, GOT_IP_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

    if(bits & GOT_IP_BIT)
    {
        ESP_LOGI(TAG, "Got IP BIT SET");
        return ESP_OK;
    }
    else 
    {
        ESP_LOGW(TAG, "GOT IP BIT RESET");
        return -1;
    }
    return -1;
}


void stopWifi(void)
{
   ESP_LOGI(TAG, "WiFi ShuttingDown..");
   esp_wifi_disconnect(); 
   esp_wifi_stop();
   vEventGroupDelete(wifiEventGroup);
  
   if(configuredStation)
   {
    configuredStation = 0;
    memset(wifiConfigStation.sta.ssid, 0, 32);
    memset(wifiConfigStation.sta.password, 0, 64);
   }

   ESP_LOGI(TAG, "WiFi Destroyed..");
}

#if defined(CONFIG_STA_CONNECTED_CB)
void registerStationConnectCB(void (*f)(void))
{
    connectedStationCB = f;
}
#endif

#if defined(CONFIG_STA_DISCONNECTED_CB)
void registerStationDisconnectedCB(void (*f)(void))
{
    disconnectedStationCB = f;
}
#endif

#if defined(CONFIG_STA_RECONNECTING_CB)
void registerStationReconnectingCB(void (*f)(void))
{
    reconnectingStationCB = f;
}
#endif

#if defined(CONFIG_GOT_IP_CB)
void registerStationGotIPCB(void (*f)(void))
{
    gotipStationCB = f;
}
#endif

#if defined(CONFIG_AP_CONNECTED_CB)
void registerAccessPointConnectCB(void (*f)(void))
{
    connectedAccessPointCB = f;
}
#endif

#if defined(CONFIG_AP_DISCONNECTED_CB)
void registerAccessPointDisconnectedCB(void (*f)(void))
{
    disconnectedAccessPointCB = f;
}
#endif


