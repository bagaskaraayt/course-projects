//LoRa
#include <LoRa.h>
#include <SPI.h>
//LoRa Pin
#define ss 5
#define rst 14
#define dio0 2
 
void setup() {

  //Begin LoRa
  LoRa.setPins(ss, rst, dio0);
  LoRa.begin(433E6);
  LoRa.setSyncWord(0xA5);

  //Begin Serial
  Serial.begin(115200);

  delay(1000);
}

void loop() {
  int packetSize = LoRa.parsePacket();    // parse packet
  if (packetSize) 
  {
    Serial.println("");
    Serial.println("Received packet ");
    Serial.println("");
 
    while (LoRa.available())              // read packet
    {
      String LoRaData = LoRa.readString();
      Serial.println(LoRaData); 
    }
  }
}
