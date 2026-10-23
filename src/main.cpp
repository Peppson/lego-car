#include "colorSettings.h"
#include "settings.h"
#include "otaUpload.h"
#include "hardware.h"
#include "neopixels.h"
#include "api.h"

extern ColorSettings colorSettings;
static Hardware hardware;
static OtaUpload otaUpload;
static API api;
NeoPixels neoPixels;

void setup()
{   
    hardware.init(api, neoPixels);
    neoPixels.init();

    #if DEBUG_STARTUP_LED_ENABLED
        hardware.startupLED();
    #endif

    #if DEBUG_TEST_FULL_BRIGHTNESS
        neoPixels.AllLedsFullBrightness();
    #endif

    #if DEBUG_OTA_ENABLED
        otaUpload.init();
    #endif

    neoPixels.fadeInAllLeds();
}

void loop()
{   
    if (api.isRunning)
        api.handle();

    if (otaUpload.isRunning)
        otaUpload.handle();

    if (hardware.buttonFlex)
        hardware.buttonFlexPressed();

    if (hardware.buttonWeb)
        hardware.buttonWebPressed();

    if (colorSettings.hasChanged)
    {   
        neoPixels.setColor();
        colorSettings.hasChanged = false;
    }

    if (neoPixels.runPoliceAnimation)
        neoPixels.animation_police();

    if (neoPixels.wasPoliceAnimationRunning) //...
    {
        neoPixels.wasPoliceAnimationRunning = false;
        neoPixels.fadeInAllLeds();
    }
}
