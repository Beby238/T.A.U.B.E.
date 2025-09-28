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
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "************";
const char *password = "*************";

//Für das Anzeigen wird nur der Buffer und die Länge gebraucht
struct Image {
  uint8_t* buf = nullptr;
  size_t len = 0;
  String caption = "";
};

Image images[5];
int currentIndex = 0;

extern uint8_t* fb_ptr;
extern size_t photosize;

extern AsyncWebServer server;


/**
* Einstellung vom AsyncWebserver auf bestimmte Aufrufe. Der Webserver kontrolliert den Buffer nach einer eingestellten Zeit.
*/
void handleWorkload() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<h2>ESP32-CAM Galerie</h2>";
    for(int i=0;i<5;i++){
      html += "<img id='img"+String(i)+"' src='/bild?id="+String(i)+"' width='500' height='360'><br>"; //Bild kann hier verzerrt aussehen
       html += "<p id='caption"+String(i)+"'>"+images[i].caption+"</p><br>";
    }
   html += "<script>"
            "setInterval(()=>{"
            "for(let i=0;i<5;i++){"
            "document.getElementById('img'+i).src='/bild?id='+i+'&t='+new Date().getTime();"
            "document.getElementById('caption'+i).innerText = 'Bild ' + (i + 1);"
            "}"
            "}, 5000);" 
            "</script>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/bild", HTTP_GET, [](AsyncWebServerRequest *request){
  if (!request->hasParam("id")) {
    request->send(400, "text/plain", "Missing id");
    return;
  }
  int id = request->getParam("id")->value().toInt();
  if (id < 0 || id >= 5) {
    request->send(404, "text/plain", "No image");
    return;
  }
  AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", images[id].buf, images[id].len);
  request->send(response);
  });
}

/**
* In dieser Funktion wird der externe Bildbuffer fb_ptr in einem größeren Buffer kopiert. Dabei werden ältere Bilder überschrieben.
*/
void loadbuffer(){
  //das alte Bild wird gelöscht
  if(images[currentIndex].buf){
    free(images[currentIndex].buf);
    images[currentIndex].buf = nullptr;
  }

  images[currentIndex].buf = (uint8_t*)ps_malloc(photosize);
  if(images[currentIndex].buf){
    memcpy(images[currentIndex].buf, fb_ptr, photosize);
    images[currentIndex].len = photosize;
    images[currentIndex].caption = "Bild " + String(currentIndex + 1);
    currentIndex = (currentIndex + 1) % 5;
  }
}
 

/**
* Die Einstellungen sowie der Start vom AsyncWebServer.
*/
void startingserver(){
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Verbunden mit WLAN");
  Serial.print("IP-Adresse: http://");
  Serial.println(WiFi.localIP());


  handleWorkload();

  server.begin();
  Serial.println("Server gestartet");
}


