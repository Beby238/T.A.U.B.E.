
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

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

camera_fb_t* fb = NULL;
size_t jpegSize = 0;

byte tookPhotoFlag = 0;
byte webserverFLAG = 0;

const char *ssid = "GalaxyA56F23";
const char *password = "d10doklol1092";


WebServer server(80);

//Hier könnte noch HTML sein, für bessere Bilder

void initCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST; //Um das letzte Bild zu nehmen
  config.jpeg_quality = 12;
  config.fb_count = 2;

  Serial.println("psramFound() = " + String(psramFound()));

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA; //FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA //FRAMESIZE_QVGA
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 62;
    config.fb_count = 1;
  }

    // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}


void takePhoto(){
  fb = esp_camera_fb_get();
  if (!fb){
    Serial.println("Camera capture failed");
  }
  Serial.println("Buffer Länge: " + (String)fb->len);
  //Serial.println("Foto funktioniert");
  jpegSize = fb->len;
  tookPhotoFlag = 1;
}

void handleImage() {
  Serial.println("Sind im handleImage");
  if (fb->buf != nullptr && jpegSize > 0){
    Serial.println("Bild wird abgerufen!");

    server.sendHeader("Content-Type", "image/jpeg");
    server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
    //esp_camera_fb_return(fb);
  }else{
    server.send(404, "text/plain", "Kein Bild verfügbar");
  }
}

void handlingroute(){
  //server.send(200, "text/html", INDEX_HTML);
  server.send(200, "text/plain", "DAS IST ROUT!");
}

//Oder ins void setup() einfügen
void startingserver(){
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Verbunden mit WLAN");
  Serial.print("IP-Adresse: http://");
  Serial.println(WiFi.localIP());
  Serial.println("JPEG Größe: " + String(jpegSize));

  //Dafür benötige http routen layout
  //server.on("/img", HTTP_GET, handleImage);

  //Bild wird direkt angezeigt
  server.on("/", handlingroute);
  server.on("/image", handleImage);

  server.begin();
  Serial.println("Webserver gestartet");
}

void setup() {
  Serial.begin(115200);

  initCamera();
  WiFi.mode(WIFI_STA);
}

void loop() {
  if (!tookPhotoFlag && Serial.available() > 0 && Serial.read() == 'f'){
    Serial.println("Foto aufnehmen, Z 253");
    takePhoto();
  }

  if (tookPhotoFlag && !webserverFLAG){
    webserverFLAG = 1;
    startingserver();
  }

  if (webserverFLAG){
    server.handleClient();
  }
}
