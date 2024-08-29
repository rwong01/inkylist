#include "Network.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Define SSID, PASS, TODOIST_API_KEY in external library
#include "secrets.h"

#include "Inkplate.h"

// Must be installed for this example to work
#include <ArduinoJson.h>

// external parameters from our main file
extern char* server;

// Get our Inkplate object from main file to draw debug info on
extern Inkplate display;

// Static Json from ArduinoJson library
JsonDocument doc;

void Network::begin() {
  // Initiating wifi, like in BasicHttpClient example
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);

  int cnt = 0;
  Serial.print(F("Waiting for WiFi to connect..."));
  while ((WiFi.status() != WL_CONNECTED)) {
    Serial.print(F("."));
    delay(1000);
    ++cnt;

    if (cnt == 20) {
      Serial.println("Can't connect to WIFI, restarting");
      delay(100);
      ESP.restart();
    }
  }
  Serial.println(F(" connected"));

  // Find internet time
  setTime();
}

// Function for initial time setting ovet the ntp server
void Network::setTime() {
  // Used for setting correct time
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();

  // Used to store time info
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  timeinfo.tm_hour = (timeinfo.tm_hour + timeZone + 24) % 24;
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

struct tasks* Network::getData() {
  struct tasks* ent;

  // If not connected to wifi reconnect wifi
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();

    delay(5000);

    int cnt = 0;
    Serial.println(F("Waiting for WiFi to reconnect..."));
    while ((WiFi.status() != WL_CONNECTED)) {
      // Prints a dot every second that wifi isn't connected
      Serial.print(F("."));
      delay(1000);
      ++cnt;

      if (cnt == 7) {
        Serial.println("Can't connect to WIFI, restart initiated.");
        delay(100);
        ESP.restart();
      }
    }
  } else {
    HTTPClient http;
    http.getStream().setTimeout(10);
    http.getStream().flush();
    // Initiate http
    char temp[128];
    sprintf(temp, server);

    http.useHTTP10(true);
    http.begin(temp);
    String header = "Bearer " + String(TODOIST_API_KEY);  // Adding "Bearer " before token
    http.addHeader("Authorization", header);      // Adding Bearer token as HTTP header

    int httpCode = http.GET();

    if (httpCode == 200) {
      Serial.println("SUCCESS");
      // while (http.getStream().available() && http.getStream().peek() != '{')
      //   (void)http.getStream().read();
      JsonDocument doc;
      // Try parsing JSON object
      DeserializationError error = deserializeJson(doc, http.getStream());

      switch (error.code()) {
        case DeserializationError::Ok:
          {
            Serial.println(F("Deserialization succeeded"));
            JsonArray docArray = doc.as<JsonArray>();
            // Loop over the JsonArray using an index
            int n = docArray.size();
            ent = (struct tasks*)ps_malloc((n + 1) * sizeof(struct tasks));
            for (size_t i = 0; i < n; i++) {
              // Access individual elements using the index
              JsonObject obj = docArray[i];
              const char* deadline = obj["due"]["string"];
              const char* content = obj["content"];

              if (deadline != NULL)
                strcpy(ent[i].deadline, deadline);
              else
                strcpy(ent[i].deadline, "\r\n");
              if (content != NULL)
                strcpy(ent[i].content, content);
              else
                strcpy(ent[i].content, "\r\n");
            }
            ent[n].content[0] = '\0';
            docArray.clear();
            break;
          }
        case DeserializationError::InvalidInput:
          Serial.println(F("Invalid input!"));
          break;
        case DeserializationError::NoMemory:
          Serial.println(F("Not enough memory"));
          break;
        default:
          Serial.println(F("Deserialization failed"));
          break;
      }
      doc.clear();
    } else {
      Serial.println("HTTP ERROR");
      display.clearDisplay();
      display.setCursor(50, 50);
      display.setTextSize(3);
      display.print(F("Error"));
      display.display();
    }
    http.end();
  }
  // Return to initial state
  WiFi.setSleep(sleep);
  return ent;
}