#pragma once
#include <FastLED.h>
#include <Preferences.h>
#include "settings.h"
#include "colorSettings.h"

class NeoPixels
{
    public:
        CRGB leds[COUNT_NEOPIXEL_TOTAL];

        bool runPoliceAnimation = false;
        bool wasPoliceAnimationRunning = false;

        void init();
        void fadeInAllLeds();
        void setColor();
        void setAllColor(uint32_t color, bool show = false);
        void setBrightness(uint8_t brightness);
        void AllLedsFullBrightness();
        
        void animation_fadeAll(const bool fadeIn, uint32_t delay_uS = 100);
        void animation_police();

    private:
        static uint8_t _currentBrightness;
};  
