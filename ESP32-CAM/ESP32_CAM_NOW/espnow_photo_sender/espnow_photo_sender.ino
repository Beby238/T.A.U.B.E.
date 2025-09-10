#include <esp_now.h>
#include "esp_camera.h"
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

#define maxPackageSize 240.0

// Esp32S3, kann auch automatisiert verlaufen mit SSID
uint8_t broadcastAddress[] = {0xFC, 0x01, 0x2C, 0xD1, 0xF6, 0xD4};

camera_fb_t* fb = NULL;

//Daten von Peer speichern (für die Verbindung wichtig)
esp_now_peer_info_t peerInfo;

uint8_t tookPhotoFlag = 0; //0 = Kein Foto aufnehmen
uint8_t issending = 0;
int currentTransmitPosition = 0;
int totalTransmitPackages = 0;
uint8_t sendnextPackageFlag = 0;

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
  config.jpeg_quality = 60;
  config.fb_count = 2;

  Serial.println("psramFound() = " + String(psramFound()));

  if (psramFound()) {
    config.frame_size = FRAMESIZE_96X96; //FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA //FRAMESIZE_QVGA
    config.jpeg_quality = 60;
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
  tookPhotoFlag = 0;
  fb = esp_camera_fb_get();
  if (!fb){
    Serial.println("Camera capture failed");
  }
  Serial.println("Buffer Länge: " + (String)fb->len);
  //Serial.println("Foto funktioniert");

  transmitting();
}

// Wird später implementiert
void cuttingPhoto(){

}

void transmitting(){
  Serial.println("In transmitting");
  int photosize = fb->len;
  Serial.println("Größe des Fotos: " + (String)photosize);
  //Serial.println("Nach der Größe des Fotos");
  currentTransmitPosition = 0;
  totalTransmitPackages = ceil(photosize / maxPackageSize);
  Serial.println("Packetanzahl: " +  (String)totalTransmitPackages);
  uint8_t message[] = {0x01, totalTransmitPackages >> 8, (byte) totalTransmitPackages}; // Wird auf 3 Bytes geteilt,
  // beim Empfänger wird totalTransmitPackages wieder aufgebaut.


  //Empfänger sagen, dass eine Datei gesendet wird
  sendData(message, sizeof(message));
}

// Reaktion vom Empfänger
void OnDataSent(const uint8_t * mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println("Derzeitiges Paket: " + String(currentTransmitPosition));
  if (totalTransmitPackages){ //Falls es eine Datei gibt zum versenden
    sendnextPackageFlag = 1;
    
    // wenn Error, sende letztes Packet nochmal
    if (status != ESP_NOW_SEND_SUCCESS){
      currentTransmitPosition--;
    }
  }
}

/// XXX: Die Paketgesamtmenge reicht vielleicht nicht aus
void sendNextPackage(){
  //Serial.println("Nächstes Paket senden , Z 137");
  sendnextPackageFlag = 0;

  //if is after the last package
  if (currentTransmitPosition == totalTransmitPackages){
    Serial.println("Datei komplett gesendet.");
    Serial.println("Byte ausgabe, potentiell hier error, Z 145");
    currentTransmitPosition = 0;
    totalTransmitPackages = 0;
    esp_camera_fb_return(fb);
    return;
  }

  //Array Packet senden:
  int dataSize = maxPackageSize;
  //Die FUnktion mach mit dem letzten Packet Probleme muss angepasst werde
  if (currentTransmitPosition == totalTransmitPackages - 1){
    Serial.println("\n************************");
    Serial.println("Letztes PAKET! 171");
    Serial.println("Größe des letztes Paket: " + totalTransmitPackages - 1);
    //dataSize =  - ((totalTransmitPackages - 1) - maxPackageSize);

    Serial.println(" last dataSize Package: " + (String)dataSize);
  }

  //datamessagearray
  uint8_t messageArray[dataSize + 3]; // die ersten 3 Einträge ist die Übermittlung von Operator und derzeitige Position vom Paket (16 Bit aufgeteilt auf 2 Byte)
  messageArray[0] = 0x02;

  //hier wäre file.seek und currentTransmitPosition die um eins erhöht wird
  currentTransmitPosition++;

  messageArray[1] = currentTransmitPosition >> 8;
  messageArray[2] = (byte) currentTransmitPosition;

  uint16_t pos = currentTransmitPosition * maxPackageSize; //AChtung falls die Bilder größer oder mehrere gesendet wird

  for (int i = 0; i < dataSize; i++){
      messageArray[3+i] = fb->buf[pos + i];
      if ((i+1)%40 == 0){
        Serial.println("");
        Serial.println("");
      }else{
        //Serial.print(String(fb->buf[currentTransmitPosition * maxPackageSize + 1]));
        Serial.print(String(fb->buf[pos + i+3]));
        Serial.print(" ");
      }
  }
  sendData(messageArray, sizeof(messageArray));
  //Serial.println("Paket wurde gesendet, Z.187");
}

//Nur für das senden zuständig
void sendData(uint8_t* dataArray, uint8_t arrayLength){
  //const uint8_t *peer_addr = slave.peer_addr; //Benötigt?
  //Serial.println("Daten senden, Z. 195");
  esp_err_t result = esp_now_send(broadcastAddress, dataArray, arrayLength);

  if (result == ESP_OK) {
    //Serial.println("Success");
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
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
  //möglichweise wird delay verwendet für ESPNOW
  //delay(1000);
}


void setup(){
  Serial.begin(115200);

  initCamera();


  //ESP zu Wi-Fi Station setzen, wichtig für NOW 
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
  esp_camera_fb_return(fb);
}

void loop(){
  //Serial.println("Im Loop, Z 249");


  //foto Schießen, falls weder gesendet oder noch kein Foto geschossen wurde
  if (!sendnextPackageFlag && !tookPhotoFlag && Serial.available() > 0 && Serial.read() == 'f'){
    tookPhotoFlag = 1;
    Serial.println("Foto aufnehmen, Z 253");
    takePhoto();
  }


  //Falls nächstes Packet gesendet wird
  if(sendnextPackageFlag){
    sendNextPackage();
  }
}

