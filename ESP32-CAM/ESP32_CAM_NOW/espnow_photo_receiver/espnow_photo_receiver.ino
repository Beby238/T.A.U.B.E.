#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#define CHANNEL 1
#define maxpacksize 240.0

//die JPEGCLass wird vielleicht gebraucht um es anzuzeigen
const char *ssid = "GalaxyA56F23";
const char *password = "dxch1196";

int currentTransmitPosition = 0;
int totalTransmitPackages = 0;

byte bildvorhanden = 0;

void* fb_buffer = 0;
uint8_t* fb_ptr = 0;

WebServer server(80);

void handleImage() {
  //server.send_P(200, "image/jpeg", (const char*)fb_ptr, totalTransmitPackages * maxpacksize);
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(totalTransmitPackages * maxpacksize));
  server.send(200, "image/jpeg", (const char*)fb_ptr);
}

// Handler für die Startseite
void handleRoot() {
  server.send(200, "text/html", "<html><body><h1>ESP32-S3 Webserver</h1><img src='/img' /></body></html>");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //pinMode(4, INPUT); //oder wenn esp32s3 LED_BUILTIN

    // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_MODE_APSTA);

    // Init ESP-NOW
  esp_now_init();
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  Serial.println("psramFound() = " + String(psramFound()));

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Verbunden mit WLAN");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // Routen definieren
  server.on("/", HTTP_GET, handleRoot);
  server.on("/img", HTTP_GET, handleImage);

  // Server starten
  server.begin();
}

void loop() {
  server.handleClient();
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
        fb_ptr = 0;
        free(fb_buffer);
        Serial.println("Sendung fertig");
      }
      //Serial.println("Daten abgerufen.");
      //delay(10);
  }
}