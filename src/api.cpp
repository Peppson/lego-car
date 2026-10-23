#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "api.h"
#include "hardware.h"
#include "generatedWebpage.h"
#include "colorSettings.h"


void API::init() 
{
    log("Init webserver\n");
    digitalWrite(PIN_LED_BLUE, HIGH);

    if (!Hardware::initWiFi())
        Hardware::errorLED(true);

    if (!MDNS.begin("lego-car"))
    {
        log("Error starting mDNS\n");
        return;
    }

    webServer.on("/hotspot-detect.html", HTTP_GET,   [this]() { this->callbackHome();          }); // Ios
    webServer.on("/generate204",         HTTP_GET,   [this]() { this->callbackHome();          }); // Android
    webServer.on("/static/hotspot.txt",  HTTP_GET,   [this]() { this->callbackHome();          }); // PC
    webServer.on("/canonical.html",      HTTP_GET,   [this]() { this->callbackHome();          });
    webServer.on("/",                    HTTP_GET,   [this]() { this->callbackHome();          });
    webServer.on("/ping",                HTTP_GET,   [this]() { this->callbackPing();          });
    webServer.on("/restart",             HTTP_GET,   [this]() { this->callbackRestart();       });
    webServer.on("/colors",              HTTP_GET,   [this]() { this->callbackGetColors();     });
    webServer.on("/colors",              HTTP_POST,  [this]() { this->callbackColors();        });
    webServer.on("/colors-save",         HTTP_POST,  [this]() { this->callbackSaveColors();    });
    webServer.on("/colors-default",      HTTP_GET,   [this]() { this->callbackDefaultColors(); });
    
    webServer.onNotFound(                            [this]() { this->callback404();         });
    webServer.begin();

    isRunning = true;
    log("Webserver online: http://lego-car.local\n");
}

void API::handle() 
{
    webServer.handleClient();
}

void API::callbackHome() 
{
    log("/home \n");
    webServer.send_P(200, "text/html", EMBEDDED_WEBPAGE);
}

void API::callback404() 
{
    log("/404 \"%s\"\n", (webServer.uri().c_str()));
    webServer.send(404, "text/plain", "Not found");
}

void API::callbackPing() 
{
    log("/ping \n");
    respondToClient(true);
}

void API::callbackRestart()
 {
    log("/restart \n");
    respondToClient(true);
    Hardware::reboot();
}

void API::callbackGetColors()
{
    log("/colors \n");
    StaticJsonDocument<256> json;

    json["frontHeadLight"] = colorSettings.frontHeadLight;
    json["frontTurnSignal"] = colorSettings.frontTurnSignal;
    json["cabinLight"] = colorSettings.cabinLight;
    json["backBreakLight"] = colorSettings.backBreakLight;

    String response;
    serializeJson(json, response);
    webServer.send(200, "application/json", response);
}

void API::callbackDefaultColors()
{   
    log("/colors-default \n");
    StaticJsonDocument<256> json;

    colorSettings.hasChanged = true;
    colorSettings.frontHeadLight = DEFAULT_HEADLIGHT;
    colorSettings.frontTurnSignal = DEFAULT_TURNSIGNAL;
    colorSettings.cabinLight = DEFAULT_CABIN;
    colorSettings.backBreakLight = DEFAULT_BREAKLIGHT;

    json["frontHeadLight"] = colorSettings.frontHeadLight;
    json["frontTurnSignal"] = colorSettings.frontTurnSignal;
    json["cabinLight"] = colorSettings.cabinLight;
    json["backBreakLight"] = colorSettings.backBreakLight;

    String response;
    serializeJson(json, response);
    webServer.send(200, "application/json", response);
}

void API::callbackColors()
{
    StaticJsonDocument<256> json;

    DeserializationError error = deserializeJson(json, webServer.arg("plain"));

    if (error)
    {
        webServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    bool changed = false;

    if (json["frontHeadLight"].is<uint32_t>())
    {
        colorSettings.frontHeadLight = json["frontHeadLight"];
        changed = true;
    }

    if (json["frontTurnSignal"].is<uint32_t>())
    {
        colorSettings.frontTurnSignal = json["frontTurnSignal"];
        changed = true;
    }

    if (json["cabinLight"].is<uint32_t>())
    {
        colorSettings.cabinLight = json["cabinLight"];
        changed = true;
    }

    if (json["backBreakLight"].is<uint32_t>())
    {
        colorSettings.backBreakLight = json["backBreakLight"];
        changed = true;
    }

    if (changed)
        colorSettings.hasChanged = true;

    #if DEBUG_LOG_CLIENT_POST
        log("frontHeadLight: #%06lX\n", (unsigned long)colorSettings.frontHeadLight);
        log("frontTurnSignal: #%06lX\n", (unsigned long)colorSettings.frontTurnSignal);
        log("cabinLight: #%06lX\n", (unsigned long)colorSettings.cabinLight);
        log("backBreakLight: #%06lX\n\n", (unsigned long)colorSettings.backBreakLight);
    #endif

    respondToClient(true);
}

void API::callbackSaveColors() 
{
    log("/colors-save \n"); //todo add memory?
    respondToClient(true);
}

void API::respondToClient(const bool response) 
{
    webServer.send(response ? 200 : 400, "text/plain", ""); 
}
