#pragma once
#include "api.h"
#include "settings.h"
#include "neopixels.h"

class Hardware 
{
    public:
        static volatile bool buttonFlex;
        static volatile bool buttonWeb;
        static bool isWiFiConnected;

        void init(API& api, NeoPixels& neoPixels);
        static void startupLED(); 
        static void errorLED(bool error);
        static void blinkLED(uint8_t led, uint8_t times = 4); 
        static void reboot();
        static void buttonFlexPressed();
        static void buttonWebPressed();
        static bool initWiFi();
        static void endWiFi();
         
    private:
        static API* _api;
        static NeoPixels* _neoPixels;

        static bool IRAM_ATTR isButtonDebounced();
        static void IRAM_ATTR isr_buttonFlex();
        static void IRAM_ATTR isr_buttonWeb();
};
