//ES wird nur das Bild angezeigt
#include <WiFi.h>
#include <WebServer.h>

//mittels extern, wird die gleiche Adresse auf pointer gezeigt
//extern void* fb_buffer;
extern uint8_t* fb_ptr;
extern size_t photosize;

extern WebServer server;


/// XXX: die funktion wird nciht aufgerufen
void handleImage() {
  //Serial.println("Sind im handleImage");
  if (fb_ptr != nullptr && photosize > 0){
    Serial.println("Bild wird abgerufen!");

    server.sendHeader("Content-Type", "image/jpeg");
    server.send_P(200, "image/jpeg", (const char*)fb_ptr, photosize);
  }else{
    server.send(404, "text/plain", "Kein Bild verfügbar");
  }
  free(fb_ptr);
}
 

//Oder ins void setup() einfügen
void startingserver(){
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Verbunden mit WLAN");
  Serial.print("IP-Adresse: http://");
  Serial.println(WiFi.localIP());
  //Serial.println("JPEG Größe: " + String(photosize));

  //Dafür benötige http routen layout
  //server.on("/img", HTTP_GET, handleImage);

  //Bild wird direkt angezeigt
  server.on("/", handleImage);

  server.begin();
  //Serial.println("Webserver gestartet");
}