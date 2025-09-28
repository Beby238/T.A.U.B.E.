#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include <Arduino.h>

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FLASH_LED         4   // LED-Pin für Blitz

camera_fb_t *fb = NULL;

// Funktion: nächsten Dateizähler ermitteln
int getNextPhotoNumber() {
  int number = 0;
  File f = SD_MMC.open("/counter.txt", FILE_READ);
  if(f){
    number = f.parseInt();
    f.close();
  }
  f = SD_MMC.open("/counter.txt", FILE_WRITE);
  if(f){
    f.println(number + 1);
    f.close();
  }
  return number;
}

void setup() {
  Serial.begin(115200);

  // Kamera konfigurieren
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 5;
  config.fb_count = 2; // nutzt PSRAM, 4 bis 6

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Kamera-Initialisierung fehlgeschlagen!");
    return;
  }

  /*
  // Sensor-Einstellungen
  sensor_t *s = esp_camera_sensor_get();
  if(s){
    s->set_brightness(s, 2);
    s->set_contrast(s, 2);
    s->set_saturation(s, -2);
    s->set_whitebal(s, 1);       // Auto Weißabgleich
    s->set_exposure_ctrl(s, 1);  // Auto Belichtung
    s->set_gain_ctrl(s, 1);      // Automatischer Gain
    s->set_aec2(s, 1);           // AEC2 aktivieren (WebServer ähnlich)
    s->set_agc_gain(s, 0);       // Startwert für Gain
  }*/
  delay(100);
  // SD-Karte mounten
  if(!SD_MMC.begin()){
    Serial.println("Fehler beim Mounten der SD-Karte");
    return;
  }
  Serial.println("SD-Karte bereit!");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'f') {
        takePhoto();
    }
  }
}


// Funktion: ein Foto aufnehmen
void takePhoto() {
  //delay(500);
  //digitalWrite(FLASH_LED, HIGH);

  camera_fb_t *fb = esp_camera_fb_get();
  if (fb) {
      esp_camera_fb_return(fb);
  }

  if(!fb){
    Serial.println("Fehler beim Aufnehmen des Fotos");
    digitalWrite(FLASH_LED, LOW);
    return;
  }

  String path = "/photo_" + String(getNextPhotoNumber()) + ".jpg";
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  if(file){
    file.write(fb->buf, fb->len);
    file.close();
    Serial.printf("Foto gespeichert: %s (%u bytes)\n", path.c_str(), fb->len);
  } else {
    Serial.println("Fehler beim Öffnen der Datei auf SD-Karte");
  }

  esp_camera_fb_return(fb);
  digitalWrite(FLASH_LED, LOW);
}

