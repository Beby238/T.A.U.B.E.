#include "esp_camera.h"
#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>

#include "img_converters.h" 

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

// Webserver auf Port 80
WebServer server(80);


uint8_t *rgb_buf = NULL;
uint8_t *rgb_cut = NULL;
uint8_t * jpg_buf = NULL;
size_t jpg_len = 0;

size_t rgbwidth = 0;
size_t rgbheight = 0;
size_t rgblen = 0;

bool photo = 0;

camera_fb_t* fb = 0;

//camera_fb_t* converted = (camera_fb_t*)ps_malloc(sizeof(camera_fb_t));
camera_config_t config;

void initCamera(){
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
  //config.grab_mode = CAMERA_GRAB_WHEN_EMPTY; // For Streaming
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST; //Um das letzte Bild zu nehmen
  config.jpeg_quality = 12;
  config.fb_count = 2;

// FRAMESIZE_QQVGA=160x120, QCIF=176x144, HQVGA=240x176, QVGA=320x240, CIF=352x288, VGA=640x480
// FRAMESIZE_SVGA=800x600, XGA=1024x768, SXGA=1280x1024, UXGA=1600x1200
// Hinweis: Höhere Auflösungen benötigen PSRAM und senken die Framerate.

  if (psramFound()) {
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 62;
    config.fb_count = 1;
  }

    // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("psramFound() = " + String(psramFound()));
}


void takePhoto(){
  //tookPhotoFlag = 0;
  Serial.println("Foto aufnehmrgb_cuten");

  //Je länger die LED an ist, desto heller ist es --> Kamera Einstellungen machen auch was aus
  digitalWrite(4, HIGH);
  delay(300);
  fb = esp_camera_fb_get();
  if (!fb){esp_camera_fb_return(fb);
    Serial.println("Camera capture failed");
  }
  digitalWrite(4, LOW);
  delay(100);
  //Serial.println("Vor Konvertierung Jpg Size: " + String(fb->len));
  rgb_buf = (uint8_t*) ps_malloc(fb->width * fb->height *2);
  jpg2rgb565(fb->buf, fb->len, (uint8_t*)rgb_buf, JPG_SCALE_NONE); //Das kann geändert werden: JPG_SCALE_(NONE/2X/4X/8X)

  //Farben tauschen, weil Endian Reihenfolge anders ist
  for (size_t i = 0; i < fb->width * fb->height *2; i += 2) { //sizeof(rgb_buf)
    uint8_t tmp = rgb_buf[i];
    rgb_buf[i] = rgb_buf[i + 1];
    rgb_buf[i + 1] = tmp;
  }
  photocutting(150, 300, 300, 150);
  jpg_buf = (uint8_t*) ps_malloc(fb->width * fb->height *2);
  //fmt2jpg((uint8_t*)rgb_buf, fb->width * fb->height*2, fb->width, fb->height, PIXFORMAT_RGB565, 40, &jpg_buf, &jpg_len);
  fmt2jpg((uint8_t*)rgb_cut, rgblen, rgbwidth, rgbheight, PIXFORMAT_RGB565, 90, &jpg_buf, &jpg_len);
  free(rgb_buf);
  //free(jpg_buf);
  esp_camera_fb_return(fb);
}

/// XXX: Die Koordinaten können nicht stimmen
void photocutting(unsigned int cropLeft, unsigned int cropRight,
  unsigned int cropTop, unsigned int cropBottom){

  rgbwidth = fb->width;
  rgbheight = fb->height;
  rgblen = rgbwidth * rgbheight * 2;
  //fb->width und fb->height können sich ändern je nach SCale
  unsigned int maxTopIndex = cropTop * rgbwidth * 2;
  unsigned int minBottomIndex = ((rgbwidth*rgbheight) - (cropBottom * rgbwidth)) * 2;
  unsigned short maxX = rgbwidth - cropRight; //In Pixels
  unsigned short newWidth = rgbwidth - cropLeft - cropRight;
  unsigned short newHeight = rgbheight - cropTop - cropBottom;
  size_t newRgblen = newWidth * newHeight * 2;
  

  rgb_cut = (uint8_t*) ps_malloc(newWidth * newHeight * 2);
  unsigned int writeIndex = 0;
  //Loop over all bytes
  for (int i = 0; i < rgblen; i+=2){
    int x = (i/2) % rgbwidth;

    if (i < maxTopIndex){continue;}
    if (i > minBottomIndex){continue;}
    if (x <=  cropLeft){continue;}
    if (x > maxX){continue;}

    rgb_cut[writeIndex++] = rgb_buf[i];
    rgb_cut[writeIndex++] = rgb_buf[i+1];
  }
  free(rgb_buf);
  rgbwidth = newWidth;
  rgbheight = newHeight; 
  rgblen = newRgblen;
  Serial.println("cutted RGB größe: " + String(rgblen));
}


void handle_jpg() {
  server.sendHeader("Content-Type", "image/jpeg");
  server.send_P(200, "image/jpeg", (const char *)jpg_buf, jpg_len);
  //server.send_P(200, "image/jpeg", (const char *)rgb_buf, rgblen);
  //esp_camera_fb_return(fb);
}

// Handler für HTML-Seite mit eingebettetem Bild
void handle_root() {
  String html = "<html><body><h1>ESP32-CAM</h1><img src='/jpg' /></body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); // LED aus

  Serial.println("psramFound() = " + String(psramFound()));

  initCamera();

    // Routen festlegen
  server.on("/", handle_jpg);
  //server.on("/jpg", handle_jpg);

  //TJpgDec.setCallback(tjpgDrawPixel);
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  if(!photo && Serial.available() && Serial.read() == 'f'){
    photo = 1;
    takePhoto();
    Serial.println("JPEG Größe: " + String(jpg_len));
    Serial.println("");
    /*
    for (int i = 0; i < jpg_len; i++){
      if ((i+1) % 40 == 0){
        Serial.println("");
      }
        Serial.print(jpg_buf[i], HEX);
        Serial.print(" ");
        
    }*/
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Verbunden mit WLAN");
    Serial.print("IP-Adresse: http://");
    Serial.println(WiFi.localIP());
    server.begin();
  }
  if (photo){
    server.handleClient();
  }
}
