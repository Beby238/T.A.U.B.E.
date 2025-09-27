//Weitere Datei für F.R. ML
const char *ssid = "GalaxyA56F23";
const char *password = "d10doklol1092";


#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


//Für das Anzeigen wird nur der Buffer und die Länge gebraucht
struct Image {
  uint8_t* buf = nullptr;
  size_t len = 0;
  String caption = "";
};

Image images[5]; // Bild anzeige anpassen
int currentIndex = 0;

extern uint8_t* fb_ptr;
extern size_t photosize;

extern AsyncWebServer server;


/// XXX: die funktion wird nciht aufgerufen
void handleWorkload() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<h2>ESP32-CAM Galerie</h2>";
    for(int i=0;i<5;i++){
      html += "<img id='img"+String(i)+"' src='/bild?id="+String(i)+"' width='640' height='480'><br>"; //Bild kann hier verzerrt aussehen
       html += "<p id='caption"+String(i)+"'>"+images[i].caption+"</p><br>";
    }
   html += "<script>"
            "setInterval(()=>{"
            "for(let i=0;i<5;i++){"
            "document.getElementById('img'+i).src='/bild?id='+i+'&t='+new Date().getTime();"
            "document.getElementById('caption'+i).innerText = 'Bild ' + (i + 1);"
            "}"
            "}, 5000);"  // alle 2 Sekunden
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
 

//Oder ins void setup() einfügen
void startingserver(){
  WiFi.begin(ssid, password);  
  WiFi.printDiag(Serial);
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


