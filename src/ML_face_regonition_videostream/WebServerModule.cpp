#include "WebServerModule.h"
#include "esp_camera.h"

WebServerModule::WebServerModule(int &x, int &y, int &w, int &h, String &label, float &confidence)
    : server(80), faceX(x), faceY(y), faceW(w), faceH(h), faceLabel(label), faceConfidence(confidence) {}

void WebServerModule::begin(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to WiFi");
        return;
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>ESP32 Face Detection</title>
            <style>
                body { text-align: center; }
                canvas { max-width: 90%; border:1px solid #ccc; }
            </style>
        </head>
        <body>
            <h1>ESP32 Kamera Stream</h1>
            <canvas id="canvas"></canvas>
            <script>
                const canvas = document.getElementById('canvas');
                const ctx = canvas.getContext('2d');
                const img = new Image();

                img.onload = function() {
                    canvas.width = img.width;
                    canvas.height = img.height;
                    ctx.drawImage(img, 0, 0);
                    fetch('/face')
                        .then(r => r.json())
                        .then(data => {
                            
                            ctx.strokeStyle = "red";
                            ctx.lineWidth = 3;
                            ctx.strokeRect(data.x, data.y, data.w, data.h);
                            ctx.fillStyle = "white";
                            ctx.font = "16px Arial";
                            ctx.fillText(`${data.label} (${data.confidence})`, data.x, data.y - 5);
                        });
                };

                setInterval(() => {
                    img.src = '/stream?' + new Date().getTime();
                }, 200);
            </script>
        </body>
        </html>
        )rawliteral";
        request->send(200, "text/html", html);
    });


    // JSON-Daten vom Face Detection
    server.on("/face", HTTP_GET, [&](AsyncWebServerRequest *request) {
        String json;
        if (faceX >= 0) {
            json = "{\"x\":" + String(faceX) +
                   ",\"y\":" + String(faceY) +
                   ",\"w\":" + String(faceW) +
                   ",\"h\":" + String(faceH) +
                   ",\"label\":\"" + faceLabel + "\"" +
                   ",\"confidence\":" + String(faceConfidence, 2) + "}";
        } else {
            json = "{}";
        }
        request->send(200, "application/json", json);
    });


    // Stream-Endpunkt
    server.on("/stream", HTTP_GET, [&](AsyncWebServerRequest *request) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            request->send(500, "text/plain", "camera capture failed");
            return;
        }



        AsyncWebServerResponse *response = request->beginResponse_P(
            200, "image/jpeg", fb->buf, fb->len
        );
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Content-Type", "image/jpeg");
        response->addHeader("Cache-Control", "no-cache");
        request->send(response);
        esp_camera_fb_return(fb);
    });

    server.begin();
    Serial.println("WebServer gestartet!");
}

void WebServerModule::handleClient() {
}
