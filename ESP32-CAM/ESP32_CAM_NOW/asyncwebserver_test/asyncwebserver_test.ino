#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "esp_camera.h"

// ==== WLAN ====
const char *ssid = "Definitiv0909";
const char *password = "6aD4bXjAEHNQD99m";

//Für das Anzeigen wird nur der Buffer und die Länge gebraucht
struct Image {
  uint8_t* buf = nullptr;
  size_t len = 0;
  String caption = "";
};

Image images[5];
int currentIndex = 0;

// *********** Helper: Framebuffer kopieren ***********
uint8_t* copyBuffer(camera_fb_t* fb) { // muss ausgetauscht werden mit dem erhaltenen Buffer
  uint8_t* buf = (uint8_t*)malloc(fb->len);
  if (buf) memcpy(buf, fb->buf, fb->len);
  return buf;
}
// ==== Webserver ====
AsyncWebServer server(80);

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

void startCameraServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<h2>ESP32-CAM Galerie</h2>";
    html += "<button onclick='takePhoto()'>Foto aufnehmen</button><br><br>";
    for(int i=0;i<5;i++){
      html += "<img id='img"+String(i)+"' src='/bild?id="+String(i)+"' width='160' height='120'><br>";
       html += "<p id='caption"+String(i)+"'>"+images[i].caption+"</p><br>";
    }
   html += "<script>"
            "function takePhoto(){"
            "fetch('/takephoto').then(()=>{"
            "for(let i=0;i<5;i++){"
            "document.getElementById('img'+i).src='/bild?id='+i+'&t='+new Date().getTime();"
            "document.getElementById('caption'+i).innerText= 'Bild ' + (i+1);"
            "}});"
            "}"
            "function updateCaption(id){"
            "let text = document.getElementById('caption'+id).value;"
            "fetch('/setcaption?id='+id+'&text='+encodeURIComponent(text));"
            "}"
            "</script>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/bild", HTTP_GET, [](AsyncWebServerRequest *request){
  if (!request->hasParam("id")) {
    request->send(400, "text/plain", "Missing id");
    return;
  }
  int id = request->getParam("id")->value().toInt();
  if (id < 0 || id >= 5) {
    request->send(404, "text/plain", "No image");
    return;
  }
  AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", images[id].buf, images[id].len);
  request->send(response);
  });

  server.on("/takephoto", HTTP_GET, [](AsyncWebServerRequest *request){
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      request->send(500, "text/plain", "Kamera Fehler");
      return;
    }

    // Alten Speicher freigeben
    if(images[currentIndex].buf){
      free(images[currentIndex].buf);
      images[currentIndex].buf = nullptr;
      images[currentIndex].len = 0;
      images[currentIndex].caption = "Bild " + String(currentIndex + 1);
    }

    // Neues Bild kopieren
    images[currentIndex].buf = copyBuffer(fb);
    images[currentIndex].len = fb->len;

    images[currentIndex].caption = "Bild " + String(currentIndex + 1);

    currentIndex = (currentIndex + 1) % 5; // Ringpuffer

    esp_camera_fb_return(fb);
    request->send(200, "text/plain", "Foto gemacht");
  });

  server.begin();
  Serial.println("Server gestartet");
  /*
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    camera_fb_t *fb = esp_camera_fb_get();
    delay(100);
    if (!fb){
      request->send(500, "text/plain", "Kamerafehler");
      return;
    }
    fb = images[position]
    position++;

    //Bild senden
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", fb->buf, fb->len);
    request->send(response);
    esp_camera_fb_return(fb);
  });

  server.begin();
  Serial.println("HTTP Server gestartet");
  */
}

void setup() {
  Serial.begin(115200);
  // WLAN
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
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
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Kamera-Init fehlgeschlagen!");
    return;
  }
  startCameraServer();
}

void loop(){

}