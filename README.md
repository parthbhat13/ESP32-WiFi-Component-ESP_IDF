# ESP32 WiFi Component 

## What is the library all about?
This is a very simple library with some necessary functions packed into it. 
1. The Library has callbacks implemented for the following
    - Access Point Connected 
    - Access Point Disconnected 

    - Station Connected 
    - Station Disconnected 
    - Station Reconnecting 
    - Got Ip Address 

2. The Library has Retries and infinite retry implemented.
3. user can easily call function to setup either wifi Station or AccessPoint.
4. easy functionality of shutting down wifi as well.
5. The library has the bits waiting functionality, to wait till the Station or Access-point is connected or have got the Ip Address.


---


## Using the library 
With the basic setup / defaults all you have to do is pull the project, and use the WifiStack in  your components folder and add the header. 



### WiFi Station
- Using WiFi Station
  There are two ways to use wifi Station
  
  when you want to manually pass the ssid and password without using the menuConfig you can do the following 
  ```
  ESP_ERROR_CHECK(initWifi());
  ESP_ERROR_CHECK(startWifiStation("Myssid", "myPassowrd"));
  ```
  when you want to use the data passed into the menuconfig then you can use the following 
  ```
  ESP_ERROR_CHECK(initWifi());
  ESP_ERROR_CHECK(startWifiStation(NULL, NULL));
  ```

  For addon, we can even wait until we have the station connected by calling the below functions 
  Station Connection 
  ```
  esp_err_t err =  waitForStaAPConnected();  // would return ESP_OK if success
  if(err == ESP_OK)
  {
    // do the job you like
  }

  ```
 

- Using Callbacks for Wifi Station 
  Before you can use the callback, you have to enable the type of callback you want to use inside of the menuconfig
  the WiFi Station has the following callbacks
  Below are some examples 
  
  Connecting To Station Callback
  ```
  void stationConnectedCB(void)
  {
    printf("Station Got Connected, CB Hit\r\n");
  }

  void app_main()
  {
    // First we have to register the callback we want to use 
    registerStationConnectCB(&stationConnectedCB);

    // init the wifi 
    ESP_ERROR_CHECK(initWifi());

    // start the wifi in station mode 
    ESP_ERROR_CHECK(startWifiStation(NULL,NULL));

  }
  ```

  Disconnected From Station Callback
  ```
  void stationDisconnectedCB(void)
  {
    printf("Station Got Disconnected, CB Hit\r\n");
  }

  void app_main()
  {
    // First we have to register the callback we want to use 
    registerStationDisconnectedCB(&stationDisconnectedCB);

    // init the wifi 
    ESP_ERROR_CHECK(initWifi());

    // start the wifi in station mode 
    ESP_ERROR_CHECK(startWifiStation(NULL,NULL));

  }
  ```

  Reconnecting To Station Callback
  ```
  void stationReconnectingCB(void)
  {
    printf("Station Is Reconnecting, CB Hit\r\n");
  }

  void app_main()
  {
    // First we have to register the callback we want to use 
    registerStationReconnectingCB(&stationReconnectingCB);

    // init the wifi 
    ESP_ERROR_CHECK(initWifi());

    // start the wifi in station mode 
    ESP_ERROR_CHECK(startWifiStation(NULL,NULL));

  }
  ```

---

### WiFi Access Point
- Using Wifi Access Point
  There are two ways to use wifi Station
  
  when you want to manually pass the ssid and password without using the menuConfig you can do the following 
  ```
  ESP_ERROR_CHECK(initWifi());
  ESP_ERROR_CHECK(startWifiStation("Myssid", "myPassowrd"));
  ```

  when you want to use the data passed into the menuconfig then you can use the following 
  ```
  ESP_ERROR_CHECK(initWifi());
  ESP_ERROR_CHECK(startWifiStation(NULL, NULL));
  ```

  Additionally, you can even use the bits to check if the wifi accesspoint is connected or not,
  ```
  esp_err_t err =  waitForStaAPConnected();  // would return ESP_OK if success
  if(err == ESP_OK)
  {
    // do the job you like
  }

  ```

- Using Callbacks for Wifi Access Point 
  Before you can use the callback, you have to enable the type of callback you want to use inside of the menuconfig
  the WiFi Access Point has the following callbacks
  Below are some examples 
  
  Connected To Accessing Point Callback (Basically whenever any device gets connected to esp32)
  ```
  void accessPointConnectedCB(void)
  {
    printf("AccessPoint Got Connected, CB Hit\r\n");
  }

  void app_main()
  {
    // First we have to register the callback we want to use 
    registerAccessPointConnectCB(&accessPointConnectedCB);

    // init the wifi 
    ESP_ERROR_CHECK(initWifi());

    // start the wifi in station mode 
    ESP_ERROR_CHECK(startWifiAccessPoint(NULL,NULL));

  }
  ```

  Disconnected Access point Callback (Basically whenever any device gets disconnected from esp32)
  ```
  void accessPointDisconnectedCB(void)
  {
    printf("AccessPoint Got Disconnected, CB Hit\r\n");
  }

  void app_main()
  {
    // First we have to register the callback we want to use 
    registerAccessPointDisconnectedCB(&accessPointDisconnectedCB);

    // init the wifi 
    ESP_ERROR_CHECK(initWifi());

    // start the wifi in station mode 
    ESP_ERROR_CHECK(startWifiAccessPoint(NULL,NULL));

  }
  ```

---

### Wifi Ip Address Events In Station Mode
- We even have the option on what to do once we get the Ip Address, which mostly as off now works whenever we have the device in station mode 
  Using the below function for waiting till we have the IP Address 

  ```
  esp_err_t err =  waitForGotIpAddress();  // would return ESP_OK if success
  if(err == ESP_OK)
  {
    // do the job you like
  }

  ```

- Using the callback for once we have got the IP Address, 
  Like always, you have to first enable to use the callback in the makeMenuConfig

  ```
  void gotIpAddrCB(void)
  {
    printf("Got IP, CB Hit\r\n");
  }

  void app_main()
  {
    // First we have to register the callback we want to use 
    registerStationGotIPCB(&gotIpAddrCB);

    // init the wifi 
    ESP_ERROR_CHECK(initWifi());

    // start the wifi in station mode 
    ESP_ERROR_CHECK(startWifiStation(NULL,NULL));

  }
  ```

---
### Additional Functions 

- There is an option to either set ```STA_MAX_RETRY_COUNTS``` in the menu config, which is set to 10times by default
  After trying for 10 times esp32 will not to anything 

- There is an option to set the ```STA_INFINIT_RETRY``` in the menu config, which is set to false/disabled by default
  If you want to keep on trying with reconnecting to the station, you can enable this option



---

### Bug Fixes?
There might as well be some or maybe major bug fixes in the code, I have tried my level best to make it as simple as possible as I keep learning about esp32 and esp-idf
This version of code is tested on the ESP_IDF -- 4.4.1. 
In terms of bugs all I can think of the following for now.

1. Better ways to implement callbacks 
2. Make the callbacks non-Blocking 

Cheers! if you guys like this repo, do let me know, and feel free to push new changes.
**Parth Yatin Temkar :)**