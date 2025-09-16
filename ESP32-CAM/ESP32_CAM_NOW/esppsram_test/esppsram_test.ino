void* fb_buffer = 0;
uint8_t* fb_ptr = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("psramFound() = " + String(psramFound()));
  reservepsram(1200);
  int offset = 0;
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 240; i++) {// HIER ACHTEN WEGEN (len - 3)!!!!!!
      fb_ptr[offset * 240 + i] = i;
      //Serial.println("derzeitige Position: " + String(currentTransmitPosition));
      //Serial.println("Pointer Abschnitt: " + String(currentTransmitPosition * totalTransmitPackages + i));
    }
    offset++;
  }
  Serial.println("Fertig mit setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0 && Serial.read() == 'f'){
    for (int i = 0; i < 1200; i++){
          if ((i + 1)% 240 == 0){
                Serial.println("");
                Serial.println("");
          }
          if ((i + 1)% 40 == 0){
            Serial.println("");
          }
          else{
            Serial.print(String(fb_ptr[i]));
            Serial.print(" ");
          }
      }
  }
}

//initiert psram;
void reservepsram(int size){
  //Serial.println("PSRAM wird festgelegt 38");
  //reserviert z.B. 1 Kb undfb_bufferzeigt auf die niedrigste Adresse
    //ps_malloc und malloc geben einen void* zurück, muss umgewandelt werden (durch Void flexibel)
  fb_buffer = ps_malloc(size); //Nur Beispiel

  fb_ptr = (uint8_t*)fb_buffer; //Typumwandlung für bytewqeise Zugriffe

  //Setze alle Bytes auf 0
  memset(fb_ptr, 0, size);
  Serial.println("");
  Serial.println("Größe vom void Pointer: " + String(sizeof(fb_buffer)));
  Serial.println("Größe vom uint8_t Pointer: " + String(sizeof(fb_ptr)));

  //heap_caps_get_largest_free_block(), um auf Fragmentierung zu achten
  //fb_ptr[42] = 0;
  //Serial.println("RAM erfolgreich festgelegt 50");
}