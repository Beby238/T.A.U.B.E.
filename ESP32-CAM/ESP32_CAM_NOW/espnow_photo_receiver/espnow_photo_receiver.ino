#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#define CHANNEL 0

#define maxpackage 200

WebServer server(80);

uint8_t *fb_ptr;
byte bildvorhanden = 0;
byte servergestartet = 0;
int position = 0;
int photosize;

typedef struct photo_information {
  uint8_t phase;
  int jpegsize;
  int position;
  int gesamtpakete;
  uint8_t data[maxpackage];
} photo_information;

photo_information photo_info;

void startingserver();
void handleImage();

void setup() {
  Serial.begin(115200);

  // WIFI_AP_STA, falls man NOW und WIFI verwenden will --> nur bei AsncWebserver möglich
  WiFi.mode(WIFI_AP_STA);

  esp_now_init();
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  Serial.println("psramFound() = " + String(psramFound()));
}

void loop() {
  if (bildvorhanden && !servergestartet){
    Serial.println("Webserver wird gestartet");
    //startingserver();
    Serial.println("****************** sollte nur einmal da sein");
    servergestartet = 1;
    bildvorhanden = 0;
    Serial.println("Daten anzeigen:");
    for (int i = 0; i < photo_info.jpegsize; i++){
      if ((i +1)% 40 == 0){
        Serial.println("");
      }
      int zahl = fb_ptr[i];
      Serial.print(zahl, HEX);
      Serial.print(" ");
    }
    Serial.println("");
    Serial.println("Daten angezeigt.");
    startingserver();
  }
  
  if (servergestartet){
      server.handleClient();
  }
}


void OnDataRecv(const uint8_t * mac, const uint8_t *incomingdata, int len) {
  //In die Memory reinkopieren
  if (len != sizeof(photo_info)) {
    Serial.printf("Empfangen: falsche Größe: %d\n", len);
    return;
  }
  memcpy(&photo_info, incomingdata, sizeof(photo_info));
  switch (photo_info.phase){
    case 0x01:
      Serial.println("Bildgröße: " + String(photo_info.jpegsize));
      Serial.println("gesamt Pakete: " + String(photo_info.gesamtpakete));
      fb_ptr = (uint8_t*) ps_malloc(photo_info.jpegsize+50);
      memset(fb_ptr, 0, photo_info.jpegsize);
      photosize = photo_info.jpegsize;
      break;
    
    case 0x02:
      //photo_info.data muss vielleicht allokiert werden
      Serial.println("Stück Position: " + String(photo_info.position));
      int dataSize = maxpackage;

      //Sollte potentiell als letztes gehen
      if (position == photo_info.gesamtpakete){
        //Serial.println("\n************************");
        dataSize = photo_info.jpegsize - ((photo_info.position-1) * maxpackage);
        Serial.println("Packetgröße: " + String(dataSize));
        bildvorhanden = 1;
      }
      for (int i = 0; i < dataSize; i++){
        fb_ptr[position * maxpackage + i] = photo_info.data[i];
      }
      position++;
      memset(photo_info.data, 0, sizeof(photo_info.data));

      break;
  }
}
