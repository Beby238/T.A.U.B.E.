# T.A.U.B.E. / N.
### Taktische Aufklärungs- und Beobachtung-Einheit / Netzwerk
- Repository erstellt am 08.08.2025
- Ertsellt von Daniel Babkin und Mikail Gül
\
Dieses Projekt befasst sich damit, ein intelligentes Überwachungssystem zu entwickeln und anschließend daraus eine Taubenroboter/-drohnen zu bauen.
Derzeit wird noch am Überwachungssystem gearbeitet.

Für weitere Informationen sowie den [Abschlussbericht](docs/abschlussbericht.md).

## Lizenz und Danksagung
Dieses Projekt ist unter der **GNU General Public License v3 (GPLv3)** lizenziert.
Siehe [LICENSE](LICENSE) für Details.

Dieses Projekt verwendet die **Edge Impulse SDK-Bibliothek für Object Detection**, lizenziert unter der [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Weitere Informationen findest du im [Edge Impulse GitHub Repository](https://github.com/edgeimpulse/firmware-arduino).

### Bibliotheken/Oss-Komponenten
Dieses Projekt nutzt folgende Open-Source-Bibliotheken:
- [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) (LGPL-3.0 Lizenzs)
- [AsyncTCP](https://github.com/ESP32Async/AsyncTCP) (LGPL-3.0 Lizenz)
- [Edge Impulse](https://docs.edgeimpulse.com/hardware/deployments/run-arduino-2-0) (Apache License, Version 2.0)
- [ESP-NOW](https://www.espressif.com/en/solutions/low-power-solutions/esp-now) (Apache License, Version 2.0)
- [ESP32](https://github.com/espressif/arduino-esp32) (Apache License, Version 2.0)

### Danksagung
Vielen Dank an talofer99 für den bereitgestellten Code, auf dem unserer ESP-NOW Bildübertragungs Code basiert: [ESP32CAM Capture and send image over esp now](https://github.com/talofer99/ESP32CAM-Capture-and-send-image-over-esp-now)

Vielen Dank an Xavier Decupyer für den Bildzuschneidungs Code [cropping image](https://simplyexplained.com/blog/esp32-cam-cropping-images-on-device/)

Vielen Dank für den Hinweis von r-downing und seinem bgr565 swapping Code [swapping bgr to rgb](https://github.com/espressif/esp32-camera/issues/422?utm_source=chatgpt.com)




