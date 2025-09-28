/*
 * Copyright (c) 2025 Dein Name
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
 
/**
* Diese Datei wird auf einem ESP32 geladen und nimmt ein Bild auf, verarbeitet es und sendet es über ESP-NOW an einen anderen ESP32.
* @author Daniel Babkin
*/
#include <esp_now.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Größe der Datenpakete. Muss gleich vom Sender sein.
#define maxpackage 1000

AsyncWebServer server(80);

uint8_t *fb_ptr;
int position = 0;
int photosize;

//Für die KI zur Benennung
char caption = 0;

byte bildvorhanden = 0;

// Struct für Bilderdaten
typedef struct photo_information {
  uint8_t phase;
  int width = fb->width;
  int height = fb->height;
  int jpegsize;
  int position;
  int gesamtpakete;
  uint8_t data[maxpackage];
} photo_information;

photo_information photo_info;

// Funktionen von syncwebserver.cpp, um auf diese zuzugreifen
void startingserver();
void loadbuffer();

/**
* Einstellung für ESP32
*/
void setup() {
  Serial.begin(115200);

  // WIFI_AP_STA, falls man NOW und WIFI verwenden will --> nur bei AsncWebserver möglich
  WiFi.mode(WIFI_AP_STA);
  
  //Webserver starten und deren Einstellungen
  startingserver();

  server.begin();
  Serial.println("Server gestartet");

  esp_now_init();
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRec));
  Serial.println("psramFound() = " + String(psramFound()));

}

/**
* Schleife für Esp32
*/
void loop() {

  // Wenn ein Bild vorhanden ist, wird es im Webserver angezeigt und temporär gespeichert
  if (bildvorhanden){
    Serial.println("Bild in Webserver");
    loadbuffer();
    bildvorhanden = 0;
    if (fb_ptr){
      free(fb_ptr);
      fb_ptr = nullptr;
    }
    position = 0;
    photo_info.gesamtpakete = 0;
    photo_info.phase = 0;
    photo_info.jpegsize = 0;
  }
}

/**
* Eine Callback Methode für ESP-NOW, um die Datenpakete zu empfangen und zu speichern.
*/
void OnDataRec(const uint8_t * mac, const uint8_t *incomingdata, int len) {
  if (len != sizeof(photo_info)) {
    Serial.printf("Empfangen: falsche Größe: %d\n", len);
    return;
  }
  memcpy(&photo_info, incomingdata, sizeof(photo_info));
  switch (photo_info.phase){
  
  //Um Bildgröße und Gesamtpakete zu bekommen. 
    case 0x01:
      Serial.println("Bildgröße: " + String(photo_info.jpegsize));
      Serial.println("gesamt Pakete: " + String(photo_info.gesamtpakete));
      fb_ptr = (uint8_t*) ps_malloc(photo_info.jpegsize+50);
      memset(fb_ptr, 0, photo_info.jpegsize);
      photosize = photo_info.jpegsize;
      break;

  //Empfangen von Datenpakete und in einem Bildbuffer reinschreiben.
    case 0x02:
      int dataSize = maxpackage;
      if (position == photo_info.gesamtpakete){
        Serial.println("\n************************");

        dataSize = photo_info.jpegsize - ((photo_info.position-1) * maxpackage);
        Serial.println("Packetgröße: " + String(dataSize));
        bildvorhanden = 1;
      }
      for (int i = 0; i < dataSize; i++){
        fb_ptr[position * maxpackage + i] = photo_info.data[i];
      }
      position++;
      memset(photo_info.data, 0, sizeof(photo_info.data));
      Serial.println("Paket da");
      break;
  }
}
