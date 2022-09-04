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
#ifndef _WIFI_STACK_H_
#define _WIFI_STACK_H_


#include <stdio.h>
#include "esp_err.h"

// #define INFINITE_RETRY_TRUE  1
// #define INFINITE_RETRY_FALSE 0

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup event handlers and nvs etc
 * Should call this before calling any other function, 
 * Set's up NVS, EventHandlers and storage
 * 
 * @return ESP_OK on success
 */
esp_err_t initWifi (void);

/**
 * @brief Start the Wifi in Station mode 
 * Starts the Wifi in station mode,  pass the parameters for SSID and Password if want to use within the code or
 * Pass NULL in SSID and Password if you want to use the config Setup from the menuConfig
 *
 * @return ESP_OK on success
 */
esp_err_t startWifiStation(const char* _ssid, const char* _password);

/**
 * @brief Start the Access Point 
 * Starts the Access Point, Pass SSID and Password if want to use the custom within the code or
 * Pass NULL in SSID and Passowrd if you want to use the parameters from menuConfig
 * Make sure password should be minimum 8 characters long
 *
 * @return ESP_OK on success
 */
esp_err_t startWifiAccessPoint (const char* _ssid, const char* _password);

/**
 * @brief Wait For Station to get connected
 * xEventWaitBit is used. would halt the execution till the station is connected to the AccessPoint
 *
 * @return ESP_OK on success
 */
esp_err_t waitForStaAPConnected(void);

/**
 * @brief Wait till you get the IP Address
 *  xEventWaitBits is used. would halt the execution till we get the IP Address
 *
 * @return ESP_OK on success
 */
esp_err_t waitForGotIpAddress(void);

/**
 * @brief Finish work with library
 * Stops all the wifi functions and releases the memory allocated
 *
 * @return --
 */
void stopWifi (void);



/**
 * @brief Register Callback Once Station Is Connected
 * use registerStationConnectedCB() , 
 * to pass in the function you want to execute as a callback.
 * 
 * @return --
 */
#if defined(CONFIG_STA_CONNECTED_CB)
static void (*connectedStationCB)();
static void registerStationConnectCB(void (*f)(void));
#endif

/**
 * @brief Register Callback Once Station Is Disconnected
 * use registerStationDisconnectedCB() , 
 * to pass in the function you want to execute as a callback.
 * 
 * @return --
 */
#if defined(CONFIG_STA_DISCONNECTED_CB)
static void (*disconnectedStationCB)();
static void registerStationDisconnectedCB(void (*f)(void));
#endif


/**
 * @brief Register Callback Once Station Is Reconnecting
 * use registerStationReconnectingCB() , 
 * to pass in the function you want to execute as a callback.
 * 
 * @return --
 */
#if defined(CONFIG_STA_RECONNECTING_CB)
static void (*reconnectingStationCB)();
static void registerStationReconnectingCB(void (*f)(void));
#endif


/**
 * @brief Register Callback Once Station got ip address
 * use registerStationGotIPCB() , 
 * to pass in the function you want to execute as a callback.
 * 
 * @return --
 */
#if defined(CONFIG_GOT_IP_CB)
static void (*gotipStationCB)();
static void registerStationGotIPCB(void (*f)(void));
#endif

/**
 * @brief Register Callback Once AccessPoint is connected
 * use registerAccessPointConnectCB() , 
 * to pass in the function you want to execute as a callback.
 * 
 * @return --
 */
#if defined(CONFIG_AP_CONNECTED_CB)
static void (*connectedAccessPointCB)();
static void registerAccessPointConnectCB(void (*f)(void));
#endif

/**
 * @brief Register Callback Once Station Is Reconnecting
 * use registerAccessPointDisconnectedCB() , 
 * to pass in the function you want to execute as a callback.
 * 
 * @return --
 */
#if defined(CONFIG_AP_DISCONNECTED_CB)
static void (*disconnectedAccessPointCB)();
static void registerAccessPointDisconnectedCB(void (*f)(void));
#endif


#ifdef __cplusplus
}
#endif

#endif
