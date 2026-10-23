#pragma once
#include <ArduinoOTA.h>

class OtaUpload
{
    public:
        static bool isRunning;

        static void init();
        static void handle();

    private:
        static void loop();
        static void end();
        static void error(const char* errorMsg);
};
