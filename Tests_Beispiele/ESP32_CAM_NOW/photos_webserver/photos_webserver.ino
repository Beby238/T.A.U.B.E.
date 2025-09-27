#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "esp_camera.h"


// ==== Webserver ====
AsyncWebServer server(80);

// ==== Bildspeicher ====
#define MAX_IMAGES 10

//KAnn unter umständen nicht klappen -> einfach struct mit 5 bis zu 10 Buffern für die Bilder
uint8_t* imageData[MAX_IMAGES] = {nullptr};
size_t   imageSize[MAX_IMAGES] = {0};
int imageCount = 0;
int nextIndex  = 0;

// ==== Kamera-Pins (AI Thinker ESP32-S3-CAM, ggf. anpassen) ====
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// 
void storeImage(uint8_t* data, size_t len) {
  if (imageData[nextIndex]) {
    free(imageData[nextIndex]);
    imageData[nextIndex] = nullptr;
  }
  uint8_t* buf = (uint8_t*)ps_malloc(len);
  if (buf) {
    memcpy(buf, data, len);
    imageData[nextIndex] = buf;
    imageSize[nextIndex] = len;
    //Hier muss sich imageCount reseten
    if (imageCount < MAX_IMAGES) imageCount++;
    nextIndex = (nextIndex + 1) % MAX_IMAGES;
  }
}

// ==== Route: Galerie-Seite ====
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 Galerie</title>
  <style>
    body { font-family: sans-serif; }
    .item { margin-bottom: 20px; }
    img { max-width: 300px; display: block; }
  </style>
</head>
<body>
  <h1>Meine Galerie</h1>
  %IMAGES%
  <script>
    setTimeout(() => location.reload(), 5000); // alle 5s neu laden
  </script>
</body>
</html>
)rawliteral";

String buildGallery() {
  String page(htmlPage);
  String imgs;
  for (int i = 0; i < imageCount; i++) {
    int idx = (nextIndex - imageCount + i + MAX_IMAGES) % MAX_IMAGES;
    imgs += "<div class='item'><img src='/image/";
    imgs += idx;
    imgs += "?t=";
    imgs += millis();
    imgs += "'><p>Bild ";
    imgs += i + 1;
    imgs += "</p></div>\n";
  }
  page.replace("%IMAGES%", imgs); //%IMAGES% dient als platzhate
  return page;
}

// ==== Setup ====
void setup() {
  Serial.begin(115200);

  

  // WLAN
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  // Kamera konfigurieren
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_QVGA;  // 320x240
  config.jpeg_quality = 30;
  config.fb_count     = 2;
  config.fb_location  = CAMERA_FB_IN_PSRAM;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Kamera-Init fehlgeschlagen!");
    return;
  }

  // Webserver Routen
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", buildGallery());
  });

  server.on("^\\/image\\/(\\d+)$", HTTP_GET, [](AsyncWebServerRequest *request){
    int idx = request->pathArg(0).toInt();
    if (idx < 0 || idx >= MAX_IMAGES || !imageData[idx]) {
      request->send(404, "text/plain", "Bild nicht gefunden");
      return;
    }
    AsyncWebServerResponse *response =
      request->beginResponse_P(200, "image/jpeg", imageData[idx], imageSize[idx]);
    request->send(response);
    idx++;
    if (idx >= MAX_IMAGES){idx = 0;}
  });

  server.begin();
}

// ==== Loop: regelmäßig Fotos aufnehmen ====
void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Fehler: Kein Kamerabild!");
    delay(2000);
    return;
  }
  Serial.println("Bild Größe: " + String(fb->len));
  storeImage(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  delay(10000); // alle 10 Sekunden ein Foto
}
