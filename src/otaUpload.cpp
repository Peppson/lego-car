#include "otaUpload.h"
#include "settings.h"
#include "hardware.h"
#include "neopixels.h"
#include "secrets.h"

bool OtaUpload::isRunning = false;

void OtaUpload::init()
{   
    if (isRunning) return;
    
    log("----------- Starting OTA -----------\n");
    isRunning = true;

    Hardware::blinkLED(PIN_LED_BLUE);     
    digitalWrite(PIN_LED_BLUE, HIGH);       

    if (!Hardware::initWiFi()) 
        error("Wifi failed");

    ArduinoOTA.setHostname("ota-hostname-lego-car");
    ArduinoOTA.setPassword(SECRET_OTA_PASSWORD);    

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        log("OTA Progress: %u%%\n", (progress * 100) / total);
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
        log("OTA ERROR: %u\n", error);

        switch (error)
        {
            case OTA_AUTH_ERROR: log("OTA_AUTH_ERROR\n"); break;
            case OTA_BEGIN_ERROR: log("OTA_BEGIN_ERROR\n"); break;
            case OTA_CONNECT_ERROR: log("OTA_CONNECT_ERROR\n"); break;
            case OTA_RECEIVE_ERROR: log("OTA_RECEIVE_ERROR\n"); break;
            case OTA_END_ERROR: log("OTA_END_ERROR\n"); break;
            default: log("OTA_UNKNOWN_ERROR\n"); break;
        }
    });

    ArduinoOTA.begin();

    #if !DEBUG_OTA_ENABLED
        loop();
    #endif
}

void OtaUpload::handle()
{   
    ArduinoOTA.handle();
}

void OtaUpload::loop()
{   
    // Reboots after successful upload
    while (true) 
    {
        ArduinoOTA.handle();

        if (digitalRead(PIN_BTN_WEB) == HIGH) 
            end();
    }
}

void OtaUpload::end()
{   
    isRunning = false;
    ArduinoOTA.end();
    Hardware::endWiFi();
    Hardware::reboot();
}

void OtaUpload::error(const char* errorMsg)
{   
    log("Error OTA: %s\n", errorMsg);

    isRunning = false;
    Hardware::errorLED(true);
    ArduinoOTA.end();
    Hardware::endWiFi();

    STOP
}
