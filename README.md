# T.A.U.B.E. / N.
### Taktische Aufklärungs- und Beobachtung-Einheit / Netzwerk
- Repository erstellt am 08.08.2025


## Sources:
- https://github.com/Freenove/Freenove_ESP32_S3_WROOM_Board
- https://www.youtube.com/watch?v=cVvs7UjOS1E
- https://mischianti.org/esp32-cam-high-resolution-pinout-and-specs/
  - Pin Belegung des CAM-Slots
- https://randomnerdtutorials.com/esp32-cam-troubleshooting-guide/?utm_source=chatgpt.com
  - Probleme mit Lösungen
- https://www.donskytech.com/esp32-exception-decoder/
- https://www.reddit.com/r/esp32/comments/1aoi80y/bought_an_esp32s3_and_tried_the_psram/
- https://docs.espressif.com/projects/esp-idf/en/v4.3-rc/esp32/api-guides/fatal-errors.html
- https://knowledge.oasis-x.io/ov-2640-esp32-setup-guide-advanced-110
- https://randomnerdtutorials.com/esp32-cam-troubleshooting-guide/
- https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/  
  - ESP32 CAM
- https://github.com/Bodmer/TFT_eSPI/issues/3332
- https://stackoverflow.com/questions/63399901/how-to-fix-guru-meditation-error-core-1-paniced-loadprohibited-error
- Memory Testing:
  - https://lonelybinary.com/en-uk/blogs/tinkerblock-esp32-s3-starter-kit/01_6_memory_testing_guide

### ESP32-Cam Hinweise:
- für die Board LED: GPIO 4
- Siehe ESP32-CAM für die Konfieguration

# Lafvin ESP32 S3 Wroom
- https://super-starter-kit-for-esp32-s3-wroom.readthedocs.io/en/latest/preparation/Notes_For_GPIO.html
  - Für die Kamera stehen die Belegung
-  https://super-starter-kit-for-esp32-s3-wroom.readthedocs.io/en/latest/index.html
-  Laut der Anleitung ist das passende Arduino Kamera Modell: CAMERA_MODEL_ESP32S3_EYE
-  Es gibt noch, einen VideoWebServer zu erstellen (unter Camera Web Server):
  -  https://super-starter-kit-for-esp32-s3-wroom.readthedocs.io/en/latest/1.C_Tutorial/25_camera_web_server.html

## ESP IDF:
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html
- https://github.com/espressif/esp-idf
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/establish-serial-connection.html
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/linux-macos-setup.html
- https://www.youtube.com/watch?v=oHHOCdmLiII
- Für Debugging:
  - https://www.youtube.com/watch?v=uq93H7T7cOQ
- API Schnittstelle mit Funktionen:
  - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/index.html

## Funktionen:
- Kamera mit Gesichtserkennung
  - Gesichtserkennung auf Lächeln oder wütend aussehen (Staatsfeind, Staatsfreund)
  - ON/OFF
- Lautsprecher: (Für Aussagen, Gruuu Geräusche)
- Benutzeroberfläche --> wahrscheinlich Webserver
  - Geräusche Steuern, Es als Lautsprecher verwenden oder von Text ablesen
  - Statusupdate ()
- Flattern (Mit Motoren)
- Sende Möglichkeiten

