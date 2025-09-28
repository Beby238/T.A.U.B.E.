#pragma once
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

class WebServerModule {
public:
    WebServerModule(int &x, int &y, int &w, int &h, String &label, float &confidence);
    void begin(const char* ssid, const char* password);
    void handleClient();

private:
    AsyncWebServer server;
    int &faceX;
    int &faceY;
    int &faceW;
    int &faceH;
    String &faceLabel;
    float &faceConfidence;
};
