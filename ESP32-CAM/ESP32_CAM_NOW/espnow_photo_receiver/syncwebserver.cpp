#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "GalaxyA56F23";
const char *password = "dxch1196";

//mittels extern, wird die gleiche Adresse auf pointer gezeigt
extern void* fb_buffer;
extern uint8_t* fb_ptr;
extern size_t jpegSize;

extern WebServer server;

void handleImage() {
  Serial.println("Sind im handleImage");
  if (fb_ptr != nullptr && jpegSize > 0){
    Serial.println("Bild wird abgerufen!");
    server.send_P(200, "image/jpeg", (const char*)fb_ptr, jpegSize);
  }else{
    server.send(404, "text/plain", "Kein Bild verfügbar");
  }
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
  Serial.println("JPEG Größe: " + String(jpegSize));

  //Dafür benötige http routen layout
  //server.on("/img", HTTP_GET, handleImage);

  //Bild wird direkt angezeigt
  server.on("/image.jpg", handleImage);

  server.begin();
  Serial.println("Webserver gestartet");
}