# ESP32-CAM-MB
- Das ist die Dokumentation über Programme die in dem ESP32-CAM enthalten sind.

## ESP Netzwerk (ESP-NOW)

### Sources:
- https://www.youtube.com/watch?v=bEKjCDDUPaU
- https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/
- https://github.com/espressif/esp-now/tree/master
- https://www.espressif.com/en/solutions/low-power-solutions/esp-now
- https://randomnerdtutorials.com/esp-now-two-way-communication-esp32/
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html
- Transfering Photo:
  - https://www.youtube.com/watch?v=0s4Bm9Ar42U
- Playlist:
  - https://www.youtube.com/watch?v=Ydi0M3Xd_vs&list=PLYutciIGBqC1diV8-nsTI0DY-pYNWxDaY
 
### documentation:
- https://demo-dijiudu.readthedocs.io/en/latest/api-reference/wifi/esp_now.html
 
### Basic Information:
- Geräte sind entweder Initator oder Responder (beides geht auch)
  - Initator sendet Daten, Responder bekommt die Daten <-- häufig gibt es viele Iniator und ein Responder
- max. 250 bytes pro Packet <-- eigene Lösung geben
  - zwischen 6 - 20 Stationen
- MAC-Adresse wird benötigt --> muss herausgefunden werden (gibt sketches)
- Zwei ESPs können betrieben werden, indem eine weitere Arduino IDE Instanz ausgeführt wird und der andere Port ausgewählt ist
- Die MAC Adresse kann auch ermittelt werden, indem wir mittels WIFI.scanNetworks() , verwenden um die SSID vom Gerät zu nehmen und die MAC-Adresse bekommen

### Sendevorgang:
- WIFI muss vor NOW initialisiert werden


## ESP Photo + NOW
- https://www.youtube.com/watch?v=0s4Bm9Ar42U
- https://github.com/talofer99/ESP32CAM-Capture-and-send-image-over-esp-now/blob/master/Camera/Camera.ino
  - config.grab_mode = CAMERA_GRAB_LATEST; // Um das letzte Bild zu bekommen
- Foto schneiden:
  - https://simplyexplained.com/blog/esp32-cam-cropping-images-on-device/
- jpegzurgb umwandlung (ganz unten):
  - https://stackoverflow.com/questions/66046097/esp32-store-rgb-from-decoded-jpeg-mcus-in-buffer
    - also mit #include "img_converters.h", und dessen Funktion jpg2rgb565
  - potentiell auch mit TJPG_Decoder:
    -   https://github.com/Bodmer/TJpg_Decoder/blob/master/examples/FLash_array/Flash_Jpg/Flash_Jpg.ino
- Hinweise zu JPEG Verarbeitung:
  - https://github.com/espressif/esp32-camera/issues/422?utm_source=chatgpt.com
 
## ESP AI (wahrscheinlich mit Edge Impulse):
- https://github.com/espressif/esp-who/tree/master
- https://arxiv.org/pdf/2105.12931
- https://www.makerguides.com/train-an-object-detection-model-with-edge-impulse-for-esp32-cam/?utm_source=chatgpt.com
- https://circuitdigest.com/microcontroller-projects/esp32-cam-face-recognition-using-edge-impulse?utm_source=chatgpt.com
- https://developer.espressif.com/blog/hand-gesture-recognition-on-esp32-s3-with-esp-deep-learning/
- https://github.com/espressif/esp-dl/tree/master
- https://arm-software.github.io/CMSIS_6/latest/General/index.html
- Tutorial für Objekt Erkennung (Face Detectio/Recognition)
  - https://docs.edgeimpulse.com/tutorials/end-to-end/object-detection-centroids

## Using PSRAM
- https://thingpulse.com/esp32-how-to-use-psram/
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/external-ram.html#configuring-external-ram
- https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-psram.c
 
## andere interessante ESP sachen:
- https://youtu.be/sLW_r0OVyok?si=TYqVd2hErxwQghLa
- https://youtu.be/dZjbGpynQJA?si=6eSoZuWkUTj2rPaq
- Gallery Photo:
  - https://randomnerdtutorials.com/esp32-cam-post-image-photo-server/
- https://randomnerdtutorials.com/esp32-cam-take-photo-display-web-server/?utm_source=chatgpt.com
