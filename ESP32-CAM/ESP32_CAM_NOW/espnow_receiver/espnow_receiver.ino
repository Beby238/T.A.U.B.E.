#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#define CHANNEL 0
#define maxPackageSize 240.0

//die JPEGCLass wird vielleicht gebraucht um es anzuzeigen
//const char *ssid = "GalaxyA56F23";
//const char *password = "dxch1196";

int currentTransmitPosition = 0;
int totalTransmitPackages = 0;
int jpegSize = 0;
//int offset = 0;


WebServer server(80);

byte bildvorhanden = 0;
byte servergestartet = 0;

//void* fb_buffer = ;
uint8_t* fb_ptr = nullptr;

void startingserver();
void handleImage();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //pinMode(4, INPUT); //oder wenn esp32s3 LED_BUILTIN

    // WIFI_AP_STA, falls man NOW und WIFI verwenden will --> nur bei AsncWebserver möglich
  WiFi.mode(WIFI_AP_STA);

    // Init ESP-NOW
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
    for (int i = 0; i < jpegSize; i++){
      Serial.print(fb_ptr[i], HEX);
      Serial.print(" ");
      /*
      if ((i + 1)% 240 == 0){
            Serial.println("");
            Serial.println("");
      }
      if ((i + 1)% 40 == 0){
        Serial.println("");
      }
      else{
        Serial.print(String(fb_ptr[i]));
        Serial.print(" ");
      }
      */
    }
  }
  /*
  if (servergestartet){
      server.handleClient();
  }
  */
}

//initiert psram;
void reservepsram(int size){
  //Serial.println("PSRAM wird festgelegt 38");
  //reserviert z.B. 1 Kb undfb_bufferzeigt auf die niedrigste Adresse
    //ps_malloc und malloc geben einen void* zurück, muss umgewandelt werden (durch Void flexibel)
  //fb_buffer = ps_malloc(size); //Nur Beispiel
  fb_ptr =(uint8_t*) ps_malloc(size);

  //fb_ptr = (uint8_t*)fb_buffer; //Typumwandlung für bytewqeise Zugriffe

  //Setze alle Bytes auf 0
  //memset(fb_ptr, 0, size);
  Serial.println("");
  //Serial.println("Größe vom void Pointer: " + String(sizeof(fb_ptr)));
  //Serial.println("Größe vom uint8_t Pointer: " + String(sizeof(fb_ptr)));
}

void OnDataRecv(const uint8_t * mac, const uint8_t *indata, int len) {
  //memcpy(..., incomingData, sizeof(myData)); //memcpy: destintation, source, size
    //potentiell gut
  //Serial.println("Daten kommen rein, 54");
  Serial.println("");
  Serial.println("message0: " + String(*indata));
  switch(*indata++){

    case 0x01:

      currentTransmitPosition = 0;
      jpegSize = (*indata++) << 8 | *indata;

        //Zuerst wird Pointer initialisiert, dann um eins inkrementiert und dann wird vorherhiger Wert gelesen
      Serial.println("Bildgröße = " + String(jpegSize));

      totalTransmitPackages = ceil(jpegSize / maxPackageSize);
      Serial.println("totalTransmitPackages = " + String(totalTransmitPackages));

      //psram initieren und schreiben
      /// XXX: Kaunn sein dass die RAM Größe nicht ausreicht
      //reservepsram(ceil(totalTransmitPackages * maxpacksize));
      reservepsram(jpegSize);

      break;

    case 0x02:
      Serial.println("message1: " + String(*indata));
      //Nach Positionangabe, soll es zum Datenbyte gehen

      //currentTransmitPosition = (*indata++) << 8 | *indata++;
      
      currentTransmitPosition = *indata << 8;
      *indata++;
      Serial.println("message2: " + String(*indata));
      currentTransmitPosition |= (byte) *indata;
      *indata++;
      Serial.println("Indata: " + String(*indata));
      Serial.println("Packet Nummer = " + String(currentTransmitPosition));
      currentTransmitPosition--;

      /// XXX: HIER IST FEHLER!!!!

      //len-3 weil 243 gesendet wird und die ersten 3 keine Daten vom jpeg sind
  
      for (int i = 0; i < 243; i++) {// HIER ACHTEN WEGEN (len - 3)!!!!!!
        Serial.println("Indata: " + String(*indata));
        fb_ptr[currentTransmitPosition * 240 + i] = *indata++;
       }
      
      if (currentTransmitPosition == totalTransmitPackages){
        //Code um in Webbasiert anzuzeigen, keine Lust auf SD-Karte:
        Serial.println("Datentransfer fertig");
        //Bytes anzeigen
        bildvorhanden = 1;
        //free(fb_ptr) = 0;
        //free(fb_buffer);
        Serial.println("Sendung fertig");
      }
      break;
      //Serial.println("Daten abgerufen.");
      //delay(10);
  }
}
