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

// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress[] = {0x94, 0xB5, 0x55, 0x25, 0x73, 0xA8};

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

// Create a struct_message called myData
struct_message myData;

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0) {
    success = "Delivery Success :)";
  }
  else {
    success = "Delivery Fail :(";
  }
}

void getReadings(){
  // Read Temperature and Humidity
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  lux = lightMeter.readLightLevel();

  //Pembacaan LM393
  ASM   = analogRead(35);
  moisture    = map(ASM, 4095, 0, 0, 950);

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

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  //Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  delay(1000);
}

void loop()
{
  //Get DHT readings
  getReadings();

  myData.id = 1;
  myData.temp = temperature;
  myData.hum = humidity;
  myData.moist = moisture;
  myData.lux = lux;


  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  delay(5000);  
}

