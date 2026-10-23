#pragma once
#include <FastLED.h>
#include <Arduino.h>
#include <stdio.h>
#include "secrets.h"

// Dev
#define DEBUG_LOG_ENABLED 0
#define DEBUG_LOG_CLIENT_POST 0
#define DEBUG_OTA_ENABLED 0
#define DEBUG_STARTUP_LED_ENABLED 0
#define DEBUG_TEST_FULL_BRIGHTNESS 0

// Defaults
constexpr uint DEFAULT_HEADLIGHT = 0x140b04;
constexpr uint DEFAULT_TURNSIGNAL = 0x080301;
constexpr uint DEFAULT_CABIN = 0x2c1907;
constexpr uint DEFAULT_BREAKLIGHT = 0x770000;
constexpr uint8_t DEFAULT_BRIGHTNESS = 100;

// I/O
constexpr int PIN_BTN_FLEX = 4;
constexpr int PIN_BTN_WEB = 5;
constexpr int PIN_LED_GREEN = 2;
constexpr int PIN_LED_RED = 16;
constexpr int PIN_LED_BLUE = 19;
constexpr int PIN_NEOPIXEL_FRONT = 23;
constexpr int PIN_NEOPIXEL_BACK = 22;
constexpr uint SERIAL_BAUD_RATE = 115200;
constexpr uint WIRE_BAUD_RATE = 400000;
constexpr uint BUTTON_DEBOUNCE_INTERVAL = 250;

// Neopixels
constexpr int COUNT_NEOPIXEL_FRONT = 5;
constexpr int COUNT_NEOPIXEL_BACK = 2;
constexpr uint COUNT_NEOPIXEL_TOTAL = (COUNT_NEOPIXEL_BACK + COUNT_NEOPIXEL_FRONT);
constexpr uint NEOPIXEL_MIN_BRIGHTNESS = 1;

// Wifi
constexpr const char* SSID = SECRET_SSID;
constexpr const char* PASSWORD = SECRET_PASSWORD; 

// Colors
enum Color : uint32_t
{
    OFF = 0x000000,
    RED = 0xFF0000,
    GREEN = 0x008000,               
    ORANGE = 0xFF4500, 
    BLUE = 0x0000FF,
    YELLOW = 0xFFFF00,
    WHITE = 0x82591f
};

// Log
#if DEBUG_LOG_ENABLED
    #define log(...) printf(__VA_ARGS__)
    #define STOP log("\n#####  Stop right there criminal scum!  ##### \n"); while (1) { delay(1); }
#else
    #define log(...)
    #define STOP while (1) { delay(1); }
#endif
