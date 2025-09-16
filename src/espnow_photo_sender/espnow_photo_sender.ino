#include "esp_camera.h"
#include <esp_now.h>
#include <WiFi.h>

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

//Die max. Größe vom Paket liegt zwischen 1000 und 1500 Bytes
#define maxpackage 1000


uint8_t broadcastAddress[] = {0xFC, 0x01, 0x2C, 0xD1, 0xF6, 0xD4};
esp_now_peer_info_t peerInfo;

byte tookPhotoFlag = 0;
byte sendnextPackageFlag = 0;
byte letztespaket = 0;

typedef struct photo_information {
  uint8_t phase;
  int jpegsize;
  int position;
  int gesamtpakete;
  uint8_t data[maxpackage];
} photo_information;

photo_information photo_info;

camera_fb_t* fb = 0;

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
  //config.grab_mode = CAMERA_GRAB_WHEN_EMPTY; // For Streaming
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST; //Um das letzte Bild zu nehmen
  config.jpeg_quality = 12;
  config.fb_count = 2;

// FRAMESIZE_QQVGA=160x120, QCIF=176x144, HQVGA=240x176, QVGA=320x240, CIF=352x288, VGA=640x480
// FRAMESIZE_SVGA=800x600, XGA=1024x768, SXGA=1280x1024, UXGA=1600x1200
// Hinweis: Höhere Auflösungen benötigen PSRAM und senken die Framerate.

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SXGA;
    config.jpeg_quality = 20;
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
  tookPhotoFlag = 0;
  Serial.println("Foto aufnehmen");

  //Je länger die LED an ist, desto heller ist es --> Kamera Einstellungen machen auch was aus
  digitalWrite(4, HIGH);
  delay(300);
  fb = esp_camera_fb_get();
  if (!fb){
    Serial.println("Camera capture failed");
  }
  digitalWrite(4, LOW);
  Serial.println("");
  transmitting();
}

void transmitting(){
  photo_info.jpegsize = fb->len;
  Serial.println("Größe des Fotos: " + (String)photo_info.jpegsize);
  photo_info.position = 0;
  photo_info.gesamtpakete = ceil(photo_info.jpegsize / maxpackage);
  Serial.println("Packetanzahl: " +  (String)photo_info.gesamtpakete);

  photo_info.phase = 0x01;
  sendData();
}

void sendnextPaket(){
  sendnextPackageFlag = 0;
  if (letztespaket && photo_info.position > photo_info.gesamtpakete){
    Serial.println("Ende. 123");
    Serial.println("Größe vom Struct: " + String(sizeof(photo_info)));
    photo_info.position = 0;
    photo_info.gesamtpakete = 0;
    esp_camera_fb_return(fb);
    letztespaket = 0;
    return;
  }

  int dataSize = maxpackage;
  if (photo_info.position == photo_info.gesamtpakete){
      Serial.println("\n************************");
      dataSize = photo_info.jpegsize - ((photo_info.position) * maxpackage);
      Serial.println("Packetgröße: " + String(dataSize));
      letztespaket = 1;
  }
  photo_info.phase = 0x02;
  for (int i = 0; i < dataSize; i++){
    photo_info.data[i] = fb->buf[photo_info.position * maxpackage + i];
    /*
    if ((i+1) % 40 == 0){
      Serial.println("");
    }
    Serial.print(photo_info.data[i], HEX);
    Serial.print(" ");
    */
  }
  photo_info.position++;
  sendData();
}

void OnDataSent(const uint8_t * mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  //Serial.println("Derzeitiges Paket: " + String(currentTransmitPosition));
  if (photo_info.gesamtpakete){ //Falls es eine Datei gibt zum versenden
    sendnextPackageFlag = 1;
    
    if (status != ESP_NOW_SEND_SUCCESS){
      photo_info.position--;
    }
  }

}

//Nur für das senden zuständig
void sendData(){
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &photo_info, sizeof(photo_info));
  if (result == ESP_OK) {
    //Serial.println("Success");
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    Serial.println("ESPNOW not Init.");
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Internal Error");
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
 }
 //neuere Methode für callback hinzufügen
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent)); // esp_now_register_send_cb(OnDataSent);

    // Register peer, kann durch automatische Suche ersetzt werden
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);

    /// XXX: Channel Problem bei WIFI --> nachsehen
  peerInfo.channel = 0; 
  peerInfo.encrypt = false;

      // Add peer, kann durch automatische Suche ersetzt werden        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); // LED aus

  Serial.println("psramFound() = " + String(psramFound()));

  initCamera();
  Serial.println("Größe vom Struct: " + String(sizeof(photo_info)));
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!sendnextPackageFlag && !tookPhotoFlag && Serial.available() > 0 && Serial.read() == 'f'){
    tookPhotoFlag = 1;
    takePhoto();
  }
    if(sendnextPackageFlag){
    sendnextPaket();
  }
}
