#pragma once
#include <WebServer.h>

class API
{
    public:
        void init();
        void handle();

        bool isRunning = false;
        
    private:
        WebServer webServer{80};

        void callbackHome();
        void callbackPing();
        void callbackRestart();
        void callback404();
        void callbackColors();
        void callbackGetColors();
        void callbackDefaultColors();
        void callbackSaveColors(); 
        void respondToClient(const bool response);
};
