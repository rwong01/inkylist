#include "Arduino.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// To get timeZone from main file
extern int timeZone;

// Wifi ssid and password
extern char ssid[];
extern char pass[];
extern char api_key[];
extern char inbox_id[];


struct tasks
{
    char deadline[50];
    char content[500];
};


#ifndef NETWORK_H
#define NETWORK_H

// All functions defined in Network.cpp

class Network
{
  public:
    // Functions we can access in main file
    void begin();
    void getTime(char *timeStr);
    struct tasks* getData();

  private:
    // Functions called from within our class
    void setTime();
};

#endif