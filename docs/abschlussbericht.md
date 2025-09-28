# T.A.U.B.E. / N.
### Taktische Aufklärungs- und Beobachtung-Einheit / Netzwerk
- Repository erstellt am 08.08.2025

## Einleitung:
Das ursprüngliche Ziel war es einen Taubenroboter zu bauen die eine Kamera mit Gesichtserkennung hat, einen Motor um z.B. den Kopf zu bewegen sowie einen Lautsprecher womit Taubengeräusche ausgegeben werden.
Durch anfängliche Kamera Problemen an einem Esp32 S3 Wroom und einem kauf von zusätzlichen ESP's mit eingebauten Kameras, wurde sich stattdessen entschieden ein intelligentes Überwachungssystem zu erstellen mit Gesichtsanalyse und darauffolgend die Taubenroboter zu erstellen.
Leider ist das Projekt noch nicht vollständig. Dieses Projekt wird aber dennoch fortgesetzt.

Ein intelligentes Überwachungssystem in Form von z.B. Taubenroboteren/drohnen, kann zur besseren Überwachung führen sowohl positiv als auch negativ. 
Auf der positiven Seite, können diese z.B. an öffentliche Plätze platziert werden, um in Fälle eine Straftat eine gute Beweislage zu sichern oder an kritischen Stellen von z.B. Infrastruktur platziert und bedient werden, um für eine bessere Sicherheit zu bieten.
Wichtig ist aber auch, dass diese Drohnen sehr schnell und leicht missbraucht werden können, von einem Staat, Organisation, Unternehmen oder einzelne Personen an Orten wo diese nicht gebraucht werden oder für illegale Aktivitäten. Das Herstellen von solchen intelligenten Überwachungsdrohnen ist schon mit wenig Kenntnissen relativ gut machbar umzusetzbar und erfordert nur die benötigten Ressourcen die heutzutage sehr leicht zugänglich und günstig sind.
Es ist auch sehr wichtig zu nennen, dass solche Drohnen auch zu sozialen Unruhen führen kann, wie man es schon an Verschwörungstheorien über echte Tauben sehen kann.


## Technischer Hintergrund
Auf der Hardware Seite wurden drei Esp32-CAMs mit OV2640 Kameras verwenden und ein ESP32 S3 Wroom der etwas mehr Leistung hat.
Auf der Software Seite wurden die Bibliotheken von Espressif für die Kamera aufnahme, der Bilderverarbeitung und ESPNow für die Bildübertragung verwendet. Für die Gesichtsformerkennung und der Gesichtsanalyse wurde die SDK Bibliothek Edge Impulse und für das Anzeigen der Bilder AsyncWebServer verwendet.

## Anforderungen
Das intelligente Überwachungssystem soll in der Lage sein, automatisch Gesichterformen zu erkennen und nur den Gesichtsbereich zu senden an einen mehr leistungsfähigen Mikrokontroller der dann die Gesichtsanalyse durchführt.

Die Gesichtsformerkennung soll unter einer Sekunde stattfinden. Die Sendung des Gesichts sollte unter 500 Millisekunden dauern, da sie sonst andere Drohnen vor Sendungen blockiert. Falls andere Drohnen blockiert werden, sollen diese das Gesicht temporär speichern und anschließend erneut senden.

## Architektur
![Tauben Architektur](/images/Architektur.png)
Im Architektur Modell sind drei Esp32-CAMs enthalten mit den OV2640 Kameras. Diese haben die Aufgabe Gesichtsformen zu erkennen, auszuschneiden und anschließend an den Esp32 S3-Wroom zu senden. Der Esp32 S3-Wroom analysiert die ausgeschnitten Gesichter, ob diese z.B. feindlich oder freundlich. Anschließend werden die Bilder vom Esp32 S3-Wroom vom eigenen lokal gehosteten, asynchronischen Web Server angezeigt mit dem jeweiligen Label oder weiterversendet. 

Die Esp32-CAMs und der Esp32 S3-Wroom haben diese Aufgabenzuteilung, da diese Aufgaben zusammen einen ziemlich hohen Ressourcenverbrauch haben (Video + Bildverarbeitung + Gesichtsanalyse + asyncWebServer + weitersenden) und dadurch ein Esp32-CAM nicht die genügend Ressourcen hätte und der Esp32 S3-Wroom langsam wäre.

### Gründe für die Zuteilung
- Das Ausschneiden dient dazu, die ESPNow Übertragung zu verkürzen, und um die Gesichtanalyse im Esp32 S3-Wroom zu verkürzen.
- Da es ein Netzwerk ist, muss die Lauffähigkeit von ESPNow gewährleistet sein und die Aufgaben recht sinvoll zugeteilt werden.
- Da der Esp32 S3-Wroom deutlich mehr Ressourcen hat, kann dieser die Gesichter deutlich schneller verarbeiten sowie theoretisch für mehrere gleichzeitig. Dies hat keinen großen Effekt auf die Leistung wenn wenige Esp32-CAMs verbunden sind. Bei mehreren Esp32-CAMS kann aber dadurch die Leistung aufrechterhalten werden.
 - Der Esp32 S3-Wroom dient zusätzlich auch als Schnittstelle, da nur er über eine WebServer zugreifbar ist.
 

## Implementierung
Es wurde in C++ geschrieben.
Es wurden folgenden Bibliotheken verwendet:
- "esp_camera.h"
 - Kamera Einstellung sowie Video/Foto Aufnamen Funktionen
- <esp_now.h>
 - Übertragungsprotokoll, dass die WiFi Bibliothek verwendet für den Verbindungsaufbau.
- <WiFi.h>
 - Einrichtung des Esp32 S3 Wroom als Station und für ESPNow
- "esp_wifi.h"
 - Für die ESPNow Channeleinstellung und Suche, da der Empfänger durch AsyncWebServer einen eigenartigen Channel Einstellungen hat
- "img_converters.h" 
 - Für die Bildverarbeitungsfunktionen: jpg2rgb565, fmt2jpg
- Von Edge Impuls dessen SDK Bibliotheken, um die Gesichtserkennungs MLs einzubinden
 
### Code:

Beim ESPNow für den Sender gibt es die sogenannte Callback-Funktion, die Nachrichten vom Empfänger wieder zurücksendet (Success/Fail).

Bildverarbeitung:
```cpp
...
```

Bildsendung (ESPNow):

AsyncWebServer:

Gesichtsanalyse ML (Video Stream):




## Tests und Ergebnisse
Die Tests wurden in Form von funktionierenden Beispielen durchgeführt, die dann zum Source-Code hinzugefügt wurden. Die Output-Beispiele waren von unseren Gesichter.

Es gibt zusätzlich noch den Ordner "not_functional". 
Dort liegt noch ein nicht funktionierender Source-Code, wo die SDK-Bibliothek von Edge Impulse implementiert ist und der Bildverarbeitungs-, Zuschneidungs- und ESPNow Code enthalten sind und versucht wurde es zu kombinieren.


## Fazit und Ausblick
Einer der größten Herausforderungen ist die Implementation von Edge Impuls Face Detection und Face Recognition. Wegen Zeitmangel und ein Source Code der etwas unübersichtlich sowie angepasst werden musste, ist die Implementation noch nicht gelungen. Aber es wurde ein Source erstellt der mit einem Videowebserver, die Klassifizierung von feindlichen und freundlichen (bitte lächeln) funktioniert. Da diese Implementation eines laufenden Videostreams zusammen mit dem Senden von Daten potentiell recht Ressourcen aufwendig sein kann, wurde sich vorab entgegen eines laufenden Viedostreams in der Hauptarchitektur entschieden. Jedoch kann diese Implementierung zusatzlich gesteset und eventuell mit in dei Hauptarchitektur verbunden werden. 

Bildverarbeitung und Sendung waren...



Folgende Ziele wurden erreicht:
- Bildverarbeitung sowie Zuschneidung der Bilder
- Übertragung der Bilder zu einem anderen Esp32
- Anzeigen der aufgenommen Bilder
- Gesichtsanalyse mit den Labels: Feind/Freund
  - Konnte aber noch nicht mit der Hauptarchitektur verbunden werden.


