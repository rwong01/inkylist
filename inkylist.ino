// Include Inkplate library to the sketch
#include "Inkplate.h"

// Our networking functions, declared in Network.cpp
#include "Network.h"
#include "Inter16pt7b.h"

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error "Wrong board selection for this example, please select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

// Adjust your time zone, 2 means UTC+2
int timeZone = -8;

const char* server = "https://api.todoist.com/rest/v2/tasks?";

// create object with all networking functions
Network network;

// create display object
Inkplate display(INKPLATE_1BIT);

// Delay between API calls in miliseconds (first 60 represents minutes so you can change to your need)
#define DELAY_MS (uint32_t) 5 * 60 * 1000 //5 minutes?

// Display vertically; Set to 3 to flip the screen 180 degrees
#define ROTATION 1


// Variable for counting partial refreshes
RTC_DATA_ATTR unsigned refreshes = 0;

// Constant to determine when to full update
const int fullRefresh = 20;

void setup() {
    Serial.println("Boot Up");
    Serial.begin(115200);
    // Initial display settings
    display.begin();
    display.setRotation(ROTATION);
    display.setTextWrap(true);    
    network.begin();
}

void loop() {
    struct tasks *entities;
    entities = network.getData();
    drawAll(entities);

    // Refresh full screen every fullRefresh times, defined above
    Serial.println(refreshes);
    if (refreshes % fullRefresh == 0)
        display.display();
    else
        display.partialUpdate();

    ++refreshes;
    // Go to sleep before checking again
    esp_sleep_enable_timer_wakeup(1000 * DELAY_MS);
    (void)esp_deep_sleep_start();
}

void drawAll(struct tasks *entities) {
    uint16_t y = 30;
    display.setCursor(0, y);
    display.setFont(&Inter16pt7b);

    int i = 0;
    Serial.println("Ok enter drawAll");
    while (strcmp(entities[i].content, "\0") != 0) {  // Sentinel: checking for null content
        // Print the task content
        display.println(entities[i].content);

        if(strcmp(entities[i].deadline, "\r\n") != 0) {
          display.print("due: ");
          // Print the deadline
          display.println(entities[i].deadline);
        }

        // Move cursor to next line
        display.println();
        i++;
    }

}