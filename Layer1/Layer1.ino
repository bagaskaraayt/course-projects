/*  Flow of the code

  1 - Put WiFi in STA Mode
  2 - Intialize ESPNOW
  3 - Add peer device
  4 - Define Send Callback Function
  5 - Define Receive Callback Function

*/

#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>
#include <LoRa.h>
#include <SPI.h>

//LoRa Pin
#define ss 5
#define rst 14
#define dio0 2

DHT dht (15, DHT11);
BH1750 lightMeter(0x23);

// Define variables to store DHT readings to be sent
float temperature;
float humidity;
float ASM;
float moisture;
float lux;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  char msg[50];
  int id;
  float temp;
  float hum;
  float moist;
  float lux;
} struct_message;

// Create a struct_message called DHTReadings to hold sensor readings
struct_message outgoingReadings;

// Create a struct_message called myData
struct_message myData;
struct_message sensorData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;

// Variable to store if sending data was successful
String success;

// Create an array with all the structures
struct_message boardsStruct[3] = {board1, board2};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  if(myData.id==1){
    memcpy(&board1, static_cast<const void*>(&myData), sizeof(board1));
  }
  if(myData.id==2){
    memcpy(&board2, static_cast<const void*>(&myData), sizeof(board2));
  }
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);

  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].temp = myData.temp;
  boardsStruct[myData.id-1].hum = myData.hum;
  boardsStruct[myData.id-1].moist = myData.moist;
  boardsStruct[myData.id-1].lux = myData.lux;
  Serial.printf("temp value: %f \n", boardsStruct[myData.id-1].temp);
  Serial.printf("hum value: %f \n", boardsStruct[myData.id-1].hum);
  Serial.printf("moist value: %f \n", boardsStruct[myData.id-1].moist);
  Serial.printf("lux value: %f \n", boardsStruct[myData.id-1].lux);
  Serial.println();
}

void getReadings(){
  // Read Temperature and Humidity
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  lux = lightMeter.readLightLevel();

  //Pembacaan LM393
  ASM   = analogRead(35);
  moisture    = map(ASM, 4095, 0, 0, 950);

  Serial.println("Layer1");
  Serial.print("Temperature :");Serial.println(temperature);
  Serial.print("Humidity :");Serial.println(humidity);
  Serial.print("Moisture :");Serial.println(moisture);
  Serial.print("Light : ");Serial.print(lux);Serial.println(" lux");

}

void setup(){
  // Init Serial Monitor
  Serial.begin(115200);
  Serial.println("Code start");

  dht.begin();
  Wire.begin();
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  //Begin LoRa
  LoRa.setPins(ss, rst, dio0);
  LoRa.begin(433E6);
  LoRa.setSyncWord(0xA5);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  delay(1000);

}

void loop(){

  getReadings();
  Serial.println();
  sensorData.id = 0;
  sensorData.temp = temperature;
  sensorData.hum = humidity;
  sensorData.moist = moisture;
  sensorData.lux = lux;

  //Send Packet
  Serial.println("Sending Packet1 to Bstation: ");
  LoRa.beginPacket();
  LoRa.print("Temp L1: ");LoRa.print(sensorData.temp);LoRa.println(" *C");
  LoRa.print("Humidity L1: ");LoRa.print(sensorData.hum);LoRa.println("%");
  LoRa.print("Light L1: ");LoRa.print(sensorData.lux);LoRa.println(" lx");
  LoRa.print("Moisture L1: ");LoRa.println(sensorData.moist);
  LoRa.println();
  LoRa.endPacket();

  Serial.println("Sending Packet2 to Bstation: ");
  LoRa.beginPacket();
  LoRa.print("Temp L2.1: ");LoRa.print(board1.temp);LoRa.println(" *C");
  LoRa.print("Humidity L2.1: ");LoRa.print(board1.hum);LoRa.println("%");
  LoRa.print("Light L2.1: ");LoRa.print(board1.lux);LoRa.println(" lx");
  LoRa.print("Moisture L2.1: ");LoRa.println(board1.moist);
  LoRa.println();
  LoRa.endPacket();

  Serial.println("Sending Packet3 to Bstation: ");
  Serial.println();
  LoRa.beginPacket();
  LoRa.print("Temp L2.2: ");LoRa.print(board2.temp);LoRa.println(" *C");
  LoRa.print("Humidity L2.2: ");LoRa.print(board2.hum);LoRa.println("%");
  LoRa.print("Light L2.2: ");LoRa.print(board2.lux);LoRa.println(" lx");
  LoRa.print("Moisture L2.2: ");LoRa.println(board2.moist);
  LoRa.println();
  LoRa.endPacket();

  delay(5000);  
}
