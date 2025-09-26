# T.A.U.B.E. / N.
### Taktische Aufklärungs- und Beobachtung-Einheit / Netzwerk
- Repository erstellt am 08.08.2025

## Einleitung:
Das ursprüngliche Ziel war es einen Taubenroboter zu bauen die eine Kamera mit Gesichtserkennung hat, einen Motor um z.B. den Kopf zu bewegen sowie einen Lautsprecher womit Taubengeräusche ausgegeben werden.
Durch anfängliche Kamera Problemen an einem Esp32 S3 Wroom und einem kauf von zusätzlichen ESP's mit eingebauten Kameras, wurde sich stattdessen entschieden ein intelligentes Überwachungssystem zu erstellen mit Gesichtsanalyse und darauffolgend die Taubenroboter zu erstellen.
Leider ist das Projekt noch nicht vollständig. Dieses Projekt wird aber dennoch

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


## Tests und Ergebnisse
Folgende Ziele wurden erreicht:
- Bildverarbeitung sowie Zuschneidung der Bilder
- Übertragung der Bilder zu einem anderen Esp32
- Anzeigen der aufgenommen Bilder
- Gesichtsanalyse mit den Labels: Feind/Freund
  - Konnte aber noch nicht mit der Hauptarchitektur verbunden werden.


