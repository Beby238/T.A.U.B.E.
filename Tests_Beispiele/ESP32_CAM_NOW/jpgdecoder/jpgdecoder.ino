#include <WiFi.h>
#include <esp_camera.h>
#include <JPEGDecoder.h>
#include <TJpg_Encoder.h>
#include <WebServer.h>


// WLAN-Zugangsdaten
const char* ssid = "DEIN_SSID";
const char* password = "DEIN_PASSWORT";

WiFiServer server(80);

camera_config_t config = {
  .pin_pwdn = -1,
  .pin_reset = -1,
  .pin_xclk = 21,
  .pin_sccb_sda = 26,
  .pin_sccb_scl = 27,
  .pin_d7 = 35,
  .pin_d6 = 34,
  .pin_d5 = 39,
  .pin_d4 = 36,
  .pin_d3 = 19,
  .pin_d2 = 18,
  .pin_d1 = 5,
  .pin_d0 = 4,
  .pin_vsync = 25,
  .pin_href = 23,
  .pin_pclk = 22,
  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,
  .pixel_format = PIXFORMAT_JPEG,
  .frame_size = FRAMESIZE_VGA,
  .jpeg_quality = 10,
  .fb_count = 1
};

// Cropping Einstellungen
int cropX = 50;
int cropY = 50;
int cropW = 160;
int cropH = 120;

// PSRAM-Puffer
uint16_t *croppedPixels = nullptr;
uint8_t *jpegBuffer = nullptr;
size_t jpegBufferSize = 0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WLAN");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WLAN verbunden!");
  Serial.println(WiFi.localIP());

  server.begin();

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Kamera konnte nicht initialisiert werden!");
    return;
  }

  // PSRAM-Puffer reservieren
  croppedPixels = (uint16_t*)ps_malloc(cropW * cropH * sizeof(uint16_t));
  jpegBufferSize = cropW * cropH * 2 * 2; // grob reserviert
  jpegBuffer = (uint8_t*)ps_malloc(jpegBufferSize);
}

void loop() {
  WiFiClient client = server.available();

  // Wenn 'f' über Serial gesendet wird
  if (Serial.available() && Serial.read() == 'f') {
    camera_fb_t *fb = esp_camera_fb_get();
    if(!fb) {
      Serial.println("Kamerabild konnte nicht aufgenommen werden!");
      return;
    }

    // JPEG direkt dekodieren
    JpegDec.decodeArray(fb->buf, fb->len);

    // Croppen in PSRAM
    for(int y=0; y<cropH; y++){
      for(int x=0; x<cropW; x++){
        croppedPixels[y*cropW + x] = JpegDec.getPixel(cropX + x, cropY + y);
      }
    }

    // RGB565 → JPEG im PSRAM
    TJpg_Encoder encoder;
    encoder.setOutputBuffer(jpegBuffer, jpegBufferSize);
    encoder.encode(croppedPixels, cropW, cropH, 2);

    esp_camera_fb_return(fb);
    Serial.println("Cropped JPEG im PSRAM erstellt!");
  }

  // Webserver liefert zuletzt erzeugtes Bild aus
  if(client){
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type: image/jpeg");
    client.println("Connection: close");
    client.println();

    client.write(jpegBuffer, jpegBufferSize);
    client.stop();
  }
}
