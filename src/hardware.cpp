#include <otaUpload.h>
#include "hardware.h"

volatile bool Hardware::buttonWeb = false;
volatile bool Hardware::buttonFlex = false;
bool Hardware::isWiFiConnected = false;
API* Hardware::_api = nullptr;
NeoPixels* Hardware::_neoPixels = nullptr;

void Hardware::init(API& api, NeoPixels& neoPixels) 
{
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 3*1000) { }
    delay(50);
    log("\n\n\n\n\n----------- STARTED -----------\n");

    // Buttons with internal + external 5K1 pulldown R
    pinMode(PIN_BTN_FLEX, INPUT_PULLDOWN);
    pinMode(PIN_BTN_WEB, INPUT_PULLDOWN);

    // Leds
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_BLUE, LOW);

    // Set ptr for later
    _api = &api;
    _neoPixels = &neoPixels;

    // Interupts
    attachInterrupt(PIN_BTN_FLEX, isr_buttonFlex, HIGH);
    attachInterrupt(PIN_BTN_WEB, isr_buttonWeb, HIGH);
}

void Hardware::startupLED() 
{
    const uint8_t times = 2;
    blinkLED(PIN_LED_BLUE, times);
    blinkLED(PIN_LED_RED, times);
    blinkLED(PIN_LED_GREEN, times);
}

void Hardware::errorLED(bool error) 
{   
    if (error)
    {
        log("----------- Error -----------\n"); // Great logging :)
        digitalWrite(PIN_LED_RED, HIGH);
        digitalWrite(PIN_LED_BLUE, LOW);
        digitalWrite(PIN_LED_GREEN, LOW);
    }
    else 
    {
        digitalWrite(PIN_LED_RED, LOW);
    }
}

void Hardware::blinkLED(uint8_t led, uint8_t times) 
{   
    const int interval = 75;

    for (size_t i = 0; i < times; i++)
    {
        digitalWrite(led, HIGH);
        delay(interval);
        digitalWrite(led, LOW);
        delay(interval);
    }
}

void Hardware::reboot() 
{   
    log("Reboot\n");
    _neoPixels->animation_fadeAll(false);
    delay(500);
    ESP.restart();
}


// ISR
bool IRAM_ATTR Hardware::isButtonDebounced() 
{
    static uint nextTime = 0;
    const uint curTime = millis();

    // Debounce
    if (curTime > nextTime) 
    {
        nextTime = curTime + BUTTON_DEBOUNCE_INTERVAL;
        return true;
    }

    return false;
}

void IRAM_ATTR Hardware::isr_buttonFlex() 
{
    if (!isButtonDebounced()) return;
    buttonFlex = true;
}

void IRAM_ATTR Hardware::isr_buttonWeb() 
{
    if (!isButtonDebounced()) return;
    buttonWeb = true;
}

void Hardware::buttonFlexPressed() 
{   
    detachInterrupt(PIN_BTN_FLEX);
    buttonFlex = false;
    
    bool ledOn = false;
    uint now = millis();
    uint flexThreshold = now + 3 * 1000;
    uint ledThreshold = now + 500;

    // Hold for 4s for restart
    while (digitalRead(PIN_BTN_FLEX))
    {
        now = millis();

        if (now > ledThreshold && !ledOn)
        {
            digitalWrite(PIN_LED_GREEN, HIGH);
            ledOn = true;
        }

        if (now > flexThreshold) 
        {
            Hardware::blinkLED(PIN_LED_GREEN);
            Hardware::reboot();
        }
    }

    // Or push for flex
    if (!ledOn)
    {
        _neoPixels->runPoliceAnimation = !_neoPixels->runPoliceAnimation;
        if (_neoPixels->runPoliceAnimation == false)
            _neoPixels->wasPoliceAnimationRunning = true;
    }

    digitalWrite(PIN_LED_GREEN, LOW);
    attachInterrupt(PIN_BTN_FLEX, isr_buttonFlex, HIGH);
}

void Hardware::buttonWebPressed() 
{   
    detachInterrupt(PIN_BTN_WEB);
    buttonWeb = false;
    
    bool ledOn = false;
    uint now = millis();
    uint webThreshold = now + 3 * 1000;
    uint ledThreshold = now + 500;

    // Hold for 4s for OTA
    while (digitalRead(PIN_BTN_WEB))
    {
        now = millis();

        if (now > ledThreshold && !ledOn)
        {
            digitalWrite(PIN_LED_BLUE, HIGH);
            ledOn = true;
        }

        if (now > webThreshold) 
        {
            OtaUpload::init();
        }
    }

    // Or push for webserver
    if (!ledOn)
    {   
        _api->init();
    }

    if (!_api->isRunning)
        digitalWrite(PIN_LED_BLUE, LOW);
        
    attachInterrupt(PIN_BTN_WEB, isr_buttonWeb, HIGH);
}

bool Hardware::initWiFi() 
{   
    if (isWiFiConnected) return true;
    
    log("Init WiFi\nConnecting to \"%s\"\n", SECRET_SSID);

    bool success = true; 
    uint32_t endTime = millis() + 20*1000;

    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
    WiFi.setSleep(false);
    
    while (WiFi.status() != WL_CONNECTED) 
    {
        if (millis() > endTime) 
        { 
            success = false;
            break;
        }
        #if DEBUG_LOG_ENABLED
            Serial.print(". ");
        #endif
        delay(250);
    }

    log("\nWifi: %s \n", success ? "Connected" : "Failed");
    if (success) isWiFiConnected = true;

    return success;
}

void Hardware::endWiFi() 
{
    log("End WiFi\n");
    isWiFiConnected = false;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}
