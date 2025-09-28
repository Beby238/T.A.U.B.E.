# T.A.U.B.E. / N.
### Taktische Aufklärungs- und Beobachtung-Einheit / Netzwerk
- Repository erstellt am 08.08.2025
- Ertsellt von Daniel Babkin und Mikail Gül

## Einleitung:
Das ursprüngliche Ziel war es einen Taubenroboter zu bauen die eine Kamera mit Gesichtserkennung hat, einen Motor um z.B. den Kopf zu bewegen sowie einen Lautsprecher womit Taubengeräusche ausgegeben werden.
Durch anfängliche Kamera Problemen an einem Esp32 S3-Wroom und einem kauf von zusätzlichen ESP's mit eingebauten Kameras, wurde sich stattdessen entschieden ein intelligentes Überwachungssystem zu erstellen mit Gesichtsanalyse und darauffolgend die Taubenroboter zu erstellen.
Leider ist das Projekt noch nicht vollständig. Dieses Projekt wird aber dennoch fortgesetzt.

Ein intelligentes Überwachungssystem in Form von z.B. Taubenroboteren/drohnen, kann zur besseren Überwachung führen sowohl positiv als auch negativ. 
Auf der positiven Seite, können diese z.B. an öffentliche Plätze platziert werden, um in Fälle eine Straftat eine gute Beweislage zu sichern oder an kritischen Stellen von z.B. Infrastruktur platziert und bedient werden, um für eine bessere Sicherheit zu bieten.
Wichtig ist aber auch, dass diese Drohnen sehr schnell und leicht missbraucht werden können, von einem Staat, Organisation, Unternehmen oder einzelne Personen an Orten wo diese nicht gebraucht werden oder für illegale Aktivitäten. Das Herstellen von solchen intelligenten Überwachungsdrohnen ist schon mit wenig Kenntnissen relativ gut machbar umsetzbar und erfordert nur die benötigten Ressourcen die heutzutage sehr leicht zugänglich und günstig sind.
Es ist auch sehr wichtig zu nennen, dass solche Drohnen auch zu sozialen Unruhen führen kann, wie man es schon an Verschwörungstheorien über echte Tauben sehen kann.


## Technischer Hintergrund
Auf der Hardware Seite wurden drei Esp32-CAMs mit OV2640 Kameras verwenden und ein ESP32 S3 Wroom der etwas mehr Leistung hat.
Auf der Software Seite wurden die Bibliotheken von Espressif für die Kameraaufnahme, der Bilderverarbeitung und ESP-NOW für die Bildübertragung verwendet. Für die Gesichtsformerkennung und der Gesichtsanalyse wurde die SDK Bibliothek Edge Impulse und für das Anzeigen der Bilder AsyncWebServer verwendet.

## Anforderungen
Das intelligente Überwachungssystem soll in der Lage sein, automatisch Gesichterformen zu erkennen und nur den Gesichtsbereich zu senden an einen mehr leistungsfähigen Mikrokontroller der dann die Gesichtsanalyse durchführt.

Die Gesichtsformerkennung soll unter einer Sekunde stattfinden. Die Sendung des Gesichts sollte unter 500 Millisekunden dauern, da sie sonst andere Drohnen vor Sendungen blockiert. Falls andere Drohnen blockiert werden, sollen diese das Gesicht temporär speichern und anschließend erneut senden.

## Architektur
![Tauben Architektur](/images/Architektur.png)
Im Architektur Modell sind drei Esp32-CAMs enthalten mit den OV2640 Kameras. Diese haben die Aufgabe Gesichtsformen zu erkennen, auszuschneiden und anschließend an den Esp32 S3-Wroom zu senden. Der Esp32 S3-Wroom analysiert die ausgeschnitten Gesichter, ob diese z.B. feindlich oder freundlich. Anschließend werden die Bilder vom Esp32 S3-Wroom vom eigenen lokal gehosteten, asynchronischen Web Server angezeigt mit dem jeweiligen Label oder weiterversendet. 

Die Esp32-CAMs und der Esp32 S3-Wroom haben diese Aufgabenzuteilung, da diese Aufgaben zusammen einen ziemlich hohen Ressourcenverbrauch haben (Video + Bildverarbeitung + Gesichtsanalyse + asyncWebServer + weitersenden) und dadurch ein Esp32-CAM nicht die genügend Ressourcen hätte und der Esp32 S3-Wroom langsam wäre.

### Gründe für die Zuteilung
- Das Ausschneiden dient dazu, die ESP-NOW Übertragung zu verkürzen, und um die Gesichtanalyse im Esp32 S3-Wroom zu verkürzen.
- Da es ein Netzwerk ist, muss die Lauffähigkeit von ESP-NOW gewährleistet sein und die Aufgaben recht sinvoll zugeteilt werden.
- Da der Esp32 S3-Wroom deutlich mehr Ressourcen hat, kann dieser die Gesichter deutlich schneller verarbeiten sowie theoretisch für mehrere gleichzeitig. Dies hat keinen großen Effekt auf die Leistung wenn wenige Esp32-CAMs verbunden sind. Bei mehreren Esp32-CAMS kann aber dadurch die Leistung aufrechterhalten werden.
 - Der Esp32 S3-Wroom dient zusätzlich auch als Schnittstelle, da nur er über eine WebServer zugreifbar ist.
 

## Implementierung
Es wurde in C++ geschrieben.
Es wurden folgenden Bibliotheken verwendet:
- `"esp_camera.h"`
 - Kamera Einstellung sowie Video/Foto Aufnamen Funktionen
- `<esp_now.h>`
 - Übertragungsprotokoll, dass die WiFi Bibliothek verwendet für den Verbindungsaufbau.
- `<WiFi.h>`
 - Einrichtung des Esp32 S3 Wroom als Station und für ESP-NOW
- `"esp_wifi.h"`
 - Für die ESP-NOW Kanaleinstellung und Suche, da der Empfänger durch AsyncWebServer einen eigenartigen Kanal Einstellungen hat
- `"img_converters.h"` 
 - Für die Bildverarbeitungsfunktionen: jpg2rgb565, fmt2jpg
- Von Edge Impuls dessen SDK Bibliotheken, um die Gesichtserkennungs MLs einzubinden
 
### Code

Beim ESP-NOW für den Sender gibt es die sogenannte Callback-Funktion, die Nachrichten vom Empfänger wieder zurücksendet (Success/Fail).

#### Bildverarbeitung:
- Beim Aufruf der Funktion `takePhoto()`, wird ein Bild aufgenommen. Das Bild wird anschließend mit `jpg2rgb565()`, in ein Buffer im rgb565 Format gespeichert und dabei werden die Farben blau und rot getauscht (For Loop), da die Funktion es sonst falsch verarbeitet. 
Anschließen wird das Bild geschnitten und in das Jpeg Format umgewandelt.
```cpp
    void takePhoto(){
    ...
  rgb_buf = (uint8_t*) ps_malloc(fb->width * fb->height *2);
  jpg2rgb565(fb->buf, fb->len, (uint8_t*)rgb_buf, JPG_SCALE_NONE); //Das kann geändert werden: JPG_SCALE_(NONE/2X/4X/8X)

  //Farben tauschen, weil Endian Reihenfolge anders ist
  for (size_t i = 0; i < fb->width * fb->height *2; i += 2) { //sizeof(rgb_buf)
    uint8_t tmp = rgb_buf[i];
    rgb_buf[i] = rgb_buf[i + 1];
    rgb_buf[i + 1] = tmp;
  }
  
  photocutting(10, 10, 10, 10);
  
  jpg_buf = (uint8_t*) ps_malloc(fb->width * fb->height *2);
  fmt2jpg((uint8_t*)rgb_cut, rgblen, rgbwidth, rgbheight, PIXFORMAT_RGB565, 40, &jpg_buf, &jpg_len);
  free(rgb_cut);
  esp_camera_fb_return(fb);
  transmitting();
}
```

#### Bildsendung (ESP-NOW):
- Nachdem ein Bild aufgenommen wurde, wird die Funktion `transmitting()` aufgerufen, die ermittelt wie groß die Paket Anzahl für ESP-NOW ist und setzt den Statuscode, dass ein Bild gesendet wird. In sendData, werden die Einstelldaten gesendet.
```cpp
void transmitting(){
  photo_info.jpegsize = jpg_len ;
  Serial.println("Größe des Fotos: " + (String)photo_info.jpegsize);
  photo_info.position = 0;
  photo_info.gesamtpakete = ceil(photo_info.jpegsize / maxpackage);
  Serial.println("Packetanzahl: " +  (String)photo_info.gesamtpakete);

  photo_info.phase = 0x01;
  sendData();
}
```

- Die Übermittlung erfolgreich war, werden die Datenpakete nacheinander versendet. Der Bildbuffer wird Wert für Wert abgegangen und nacheinander versendet und merkt sich die Position, bei welchem Paket man sich derzeit befindet und übermittelt diese auch. Dabei muss aber auch die Datenpaketgröße sich gemerkt werden, da die Größe vom letzten Paket sich änder kann. Nach Abschluss der Sendung, werden alle Variablen auf Null gesetzt.
```cpp
void sendnextPaket(){
  sendnextPackageFlag = 0;
  if (letztespaket && photo_info.position > photo_info.gesamtpakete){
    photo_info.position = 0;
    photo_info.gesamtpakete = 0;
    free(jpg_buf);
    letztespaket = 0;
    sendnextPackageFlag = 0;
    tookPhotoFlag = 0;
    return;
  }

  int dataSize = maxpackage;
  if (photo_info.position == photo_info.gesamtpakete){
      Serial.println("\n************************");
      dataSize = photo_info.jpegsize - ((photo_info.position) * maxpackage);
      Serial.println("Packetgröße: " + String(dataSize));
      letztespaket = 1;
  }
  photo_info.phase = 0x02;
  for (int i = 0; i < dataSize; i++){
    photo_info.data[i] = jpg_buf[photo_info.position * maxpackage + i];
  }
  photo_info.position++;
  sendData();
}
```

#### Bildempfangen (ESP-NOW):
- Der Bildempfang wird mittels der Statuscodes (nur 2) abgewickelt und den Paketpositionen. Die Daten werden in einem Bildbuffer gespeichert (fb_ptr).
```cpp
void OnDataRec(const uint8_t * mac, const uint8_t *incomingdata, int len) {
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
      //Serial.println("Stück Position: " + String(photo_info.position));
      int dataSize = maxpackage;

      //Sollte potentiell als letztes gehen
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
```

#### AsyncWebServer, Bild speichern:
Wenn ein Bild empfangen wurde, wird der fb_ptr in einem Ringbuffer gespeichert. Dieser dient als temporärer Speicher, der alte Bild überschreibt.
```cpp
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
```


## Tests und Ergebnisse
Die Tests wurden in Form von funktionierenden Beispielen durchgeführt, die dann zum Source-Code hinzugefügt wurden. Die Output-Beispiele waren von unseren Gesichter.

Es gibt zusätzlich noch den Ordner "not_functional". 
Dort liegt noch ein nicht funktionierender Source-Code, wo die SDK-Bibliothek von Edge Impulse implementiert ist und der Bildverarbeitungs-, Zuschneidungs- und ESP-NOW Code enthalten sind und versucht wurde es zu kombinieren.


## Fazit und Ausblick
Folgende Ziele wurden erreicht:
- Bildverarbeitung sowie Zuschneidung der Bilder
- Übertragung der Bilder zu einem anderen Esp32
- Anzeigen der aufgenommen Bilder
- Gesichtsanalyse mit den Labels: Feind/Freund
  - Konnte aber noch nicht mit der Hauptarchitektur verbunden werden.
  
### Probleme
Einer der größten Herausforderungen ist die Implementation von Edge Impuls Face Detection und Face Recognition. Wegen Zeitmangel und ein Source Code der etwas unübersichtlich sowie angepasst werden musste, ist die Implementation noch nicht gelungen. Haupsächtlich liegt der Grund hierzu, das die SDK Edge Impuls Object Detection Bibliothek, mittels der eingebauten Funktionen einen Kamerabildpuffer erwartet hat.
Deswegen hat ein Beispiel mit einem Videowebserver und einer Klassifizierung von feindlichen und freundlichen (bitte lächeln) funktioniert.

Die Bildverarbeitung von Jpeg zu RGB hat Zeit gekostet, da die Funktion
`cppjpg2rgb565` von img_converters.h den Farbencode (rot, grün, blau) vertauscht zu blau, grün, rot.

Bei der Bildersendung, wurde es anfänglich mit nur einem Array versucht für eine schneller Übertragung. Die Übertragung verlief immer schief, deswegen wurde dann ein `struct` verwendet.

Bei Vorstellung wurde entdeckt, dass die ESP-NOW Kommunikation gestört werden kann, indem konstant von ein steuerbares Spielauto zu einem Server mit ESP-NOW, kleine Datenpakete gesendet werden, obwohl die unterschiedlichen ESPs andere MAC-Adresse sowie WiFi Kanals hatten. Dies hat dazu geführt, dass die Empfänger Seite keine Bilder mehr annehmen konnte.


### Weiterentwicklung
- Die Edge Impuls Bibliothek müsste angepasst werden.
- Bei der ESP-NOW braucht man mehr und bessere Statuscode (0x01 und 0x02), da dies wahrscheinlich auch ein Grund war, wieso es zu Störung kam.
- Bei Empfänger sollten die Daten direkt in die Memory gespeichert werden und anschließend sollte in ein Bildbuffer gespeichert werden, da Receive Callback Funktion von ESP-NOW nicht zu stark belastet werden sollten.



## Repository-Überblick
Das Repo ist folgendermaßen aufgebaut:
- Der Ordner `information`, besitzt unsortierte Quellenangaben zum Board, ESP-idf oder zu verschiedensten Codeangaben und Beispielen.
- Der Ordner `not_functional`, beinhaltet einen geändert Source Code von Edge Impulse, der nicht funktioniert. Dieser Ordner wird auch in laufe der Zeit entfernt.
- Der Ordner `Tests_Beispiele`, hat mehrere Code Beispiele die funktionieren sollten.
- Im `src` ist der enthaltene Code zum Projekt T.A.U.B.E/N. Dabei ist dieser derzeit in den Teil aufgeteilt der für die Bildverarbeitung, Zuschneiden, Versendung und Anzeigen zuständig ist und der andere Teil ist der Videowebserver auf der Gesichter als Feind oder Freund klassifiziert und markiert werden.

### Setup
Da die Programmierung in der Arduino IDE ist, muss der Code von `src` auch in der Arduino IDE geladen werden. Zusätzlich muss die esp32, AsyncWebServer und AsyncTCP installiert werden.

Für den Codeteil: `espnow_communication` in `src`, werden zwei Esp32 (mit PSRAM) benötigt. Auf dem einen wird `espnow_photo_sender` geladen. Dort müsste die Kamerakonfiguration angepasst werden. Dafür kann z.B. das Arduino IDE Beispiel CameraWebServer in z.B. den Board config nachgeschaut werden. Es muss geachtet werden, dass der Esp32 PSRAM verwenden kann und dieser muss eingestellt werden. Falls es einstellbar ist, sollte `OPI PSRAM`, in der Boardeinstellung, eingestelt werden. 
Als nächstes muss noch die MAC-Adresse vom ESP herausgefunden werden. Dazu gibt es von der Bibliothek `WiFi.h` die Funktion `WiFi.macAddress()`: 
```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);

  // MAC-Adresse auslesen und ausgeben
  Serial.println(WiFi.macAddress());
}

void loop() {
}
```
Die ausgegeben Zahlen, werden in Hex dann in das `broadcastAddress[]` eingefügt:
- Z.B. Wert bekommen 01:02:03:04:05:06, als:  `{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}`
Bilder kann man aufnehmen indem man im Serial Monitor von der Arduino IDE `f` eingibt.

Weitere Hinweise:
- Die Funktion `tryNextChannel`, sollte im markierten Bereich bei `OnDataSent`, bei Kanalsuche auskommentiert werden, um den richtigen WiFi Kanal zu finden. Durch den AsyncWebServer auf dem Empfänger, unterscheidet sich dieser Kanal sehr sprunghaft. 
- In der Funktion `takePhoto()`, können die Werte bei `photocutting()` Werte verändert werden, um das Bild unterschiedlich zu Cutten. Diese sollten aber nicht Null sein oder negativ, da sonst Buffer Overflows entstehen können, durch die Berechnungen in der Funktion. \

Beim `espnow_photo_receiver` bei der `syncwebserver.cpp` Datei die SSID und das Passwort angegeben werden. Die IP-Adresse vom Webserver sollte angezeigt werden.
Die IP-Adresse vom Webserver wird angezeigt und darauf kann man auf die fotografierten Bilder zugreifen.


## Lizenz und Danksagung
Dieses Projekt ist unter der **GNU General Public License v3 (GPLv3)** lizenziert.
Siehe [LICENSE](LICENSE) für Details.


### Bibliotheken


### Danksagung
Vielen Dank an talofer99 für den bereitgestellten Code, auf dem unserer ESP-NOW Bildübertragungs Code basiert: [ESP32CAM Capture and send image over esp now](https://github.com/talofer99/ESP32CAM-Capture-and-send-image-over-esp-now)

Vielen Dank an Xavier Decupyer für den Bildzuschneidungs Code [cropping image](https://simplyexplained.com/blog/esp32-cam-cropping-images-on-device/)

Vielen Dank für den Hinweis von r-downing und seinem bgr565 swapping Code [swapping bgr to rgb](https://github.com/espressif/esp32-camera/issues/422?utm_source=chatgpt.com)









