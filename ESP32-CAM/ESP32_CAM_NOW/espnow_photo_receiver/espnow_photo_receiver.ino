#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#define CHANNEL 1
#define maxpacksize 240.0

//die JPEGCLass wird vielleicht gebraucht um es anzuzeigen
//const char *ssid = "xxxx";
//const char *password = "xxxx";

int currentTransmitPosition = 0;
int totalTransmitPackages = 0;

WebServer server(80);

byte bildvorhanden = 0;
byte servergestartet = 0;

void* fb_buffer = 0;
uint8_t* fb_ptr = 0;
size_t jpegSize = 0;

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
    startingserver();
    Serial.println("****************** sollte nur einmal da sein");
    servergestartet = 1;
    bildvorhanden = 0;
  }
  if (servergestartet){
      server.handleClient();
  }
}

//initiert psram;
void reservepsram(int size){
  //Serial.println("PSRAM wird festgelegt 38");
  //reserviert z.B. 1 Kb undfb_bufferzeigt auf die niedrigste Adresse
    //ps_malloc und malloc geben einen void* zurück, muss umgewandelt werden (durch Void flexibel)
  fb_buffer = ps_malloc(size); //Nur Beispiel

  fb_ptr = (uint8_t*)fb_buffer; //Typumwandlung für bytewqeise Zugriffe

  //Setze alle Bytes auf 0
  memset(fb_ptr, 0, size);

  //heap_caps_get_largest_free_block(), um auf Fragmentierung zu achten
  //fb_ptr[42] = 0;
  //Serial.println("RAM erfolgreich festgelegt 50");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *indata, int len) {
  //memcpy(..., incomingData, sizeof(myData)); //memcpy: destintation, source, size
    //potentiell gut
  //Serial.println("Daten kommen rein, 54");
  switch(*indata++){

    case 0x01:
      //Serial.println("Größe kommt an 59");
      //Serial.println("Start new Photo transfer.");
      currentTransmitPosition = 0;
      totalTransmitPackages = (*indata++) << 8 | *indata;
        //Zuerst wird Pointer initialisiert, dann um eins inkrementiert und dann wird vorherhiger Wert gelesen
      Serial.println("totalTransmitPackages = " + String(totalTransmitPackages));

      //psram initieren und schreiben
      /// XXX: Kaunn sein dass die RAM Größe nicht ausreicht
      reservepsram(ceil(totalTransmitPackages * maxpacksize));
      jpegSize = ceil(totalTransmitPackages * maxpacksize);

      break;

    case 0x02:
      //Nach Positionangabe, soll es zum Datenbyte gehen
        //Der Sender bestimmt die Position
      currentTransmitPosition = (*indata++) << 8 | *indata++;
      Serial.println("");
      Serial.println("Packet Nummer = " + String(currentTransmitPosition));
      Serial.println("");
      //Serial.println("Daten kommen 78");
      /// XXX: !!!!!!ACHTUNG KRITISCHE STELLE

      for (int i = 0; i < len-3; i++) {// HIER ACHTEN WEGEN (len - 3)!!!!!!
        
        /// XXX: Die Zuweisung zur Adresse kann falsch sein!!!

        fb_ptr[currentTransmitPosition * totalTransmitPackages + i] = *indata++;
      }
      for (int i = 0; i < len-3; i++){
        if ((i + 1)% 35 == 0){
          Serial.println("");
        }else{
        Serial.print(String(fb_ptr[currentTransmitPosition * totalTransmitPackages + i]));
        Serial.print(" ");
        }
      }
      if (currentTransmitPosition == totalTransmitPackages){
        //Code um in Webbasiert anzuzeigen, keine Lust auf SD-Karte:
        Serial.println("Datentransfer fertig");
        //Bytes anzeigen
        bildvorhanden = 1;
        //fb_ptr = 0;
        //free(fb_buffer);
        Serial.println("Sendung fertig");
      }
      //Serial.println("Daten abgerufen.");
      //delay(10);
  }
}
