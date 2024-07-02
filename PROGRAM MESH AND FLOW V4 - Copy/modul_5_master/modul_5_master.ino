#include <AntaresESP32HTTP.h>
#include <RF24.h>
#include <WiFi.h>
#include <HardwareSerial.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//rxtx
#define RXD0 3
#define TXD0 1

unsigned long antaresLastSendTime = 0;
const unsigned long antaresSendInterval = 10000; 

#define ACCESSKEY "e8095173e07be574:10c750b962e32b9a"
#define WIFISSID "anggara"
#define PASSWORD "12345678"
#define projectName "PDAM990"
#define deviceName "Datahouse"


uint64_t customerId;
float userTotalUsage;
float userTotalCost;
char currentDate[11];
char currentTime[8];

//byte messageData[50];
bool dataReceived = false;
int dataCounter = 0;
int dataCounterNewCustomer = 0;
const int module2Pin = 14; 
unsigned long ledOnMillis = 0;
const unsigned long ledDuration = 200;

// Flow
const int sensorPin = A0;
unsigned long pulseDuration = 0;
float totalLiters = 0;
// Conversion factor: pulses to cubic meters
const float pulseToCubicMeters = 0.00000247; // Adjust as needed
  
RF24 radio(4, 5);
const uint64_t beforeModuleId5 = 0xF0F0F0F0F3LL;// Alamat Modul 6
const uint64_t beforeModuleId = 0xF0F0F0F0E2LL; // Alamat Modul 1
const uint64_t beforeModuleId2 = 0xF0F0F0F0E3LL; //Alamat Modul 2
const uint64_t beforeModuleId3 = 0xF0F0F0F0E4LL;  // Alamat Modul 3
const uint64_t beforeModuleId4 = 0xF0F0F0F0E5LL;   // Alamat Modul 4

AntaresESP32HTTP antares(ACCESSKEY);
byte messageData[50];
byte newData[50];

void setup() {
  radio.begin();
  radio.openReadingPipe(1, beforeModuleId);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(85);

  antares.setDebug(true);
  antares.wifiConnection(WIFISSID, PASSWORD);

//  Serial1.begin(115200);
  Serial.begin(9600, SERIAL_8N1, RXD0, TXD0);
  pinMode(module2Pin, OUTPUT);

 if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.display();
  delay(2000);
  display.clearDisplay(); 
}
unsigned long rtimer0 = 0, timer0 = 1000;

void loop() {
  
    bacanrf();
    modulini();
       
//if (dataReceived) {
//    digitalWrite(module2Pin, HIGH); 
//    ledOnMillis = millis();
//    dataReceived = false;
//  }
//
//  if (millis() - ledOnMillis >= ledDuration) {
//    digitalWrite(module2Pin, LOW); 
//  }
 }

void modulini(){
  
   radio.stopListening();
   pulseDuration = pulseIn(sensorPin, FALLING);
   float currentLiters = pulseDuration * pulseToCubicMeters;
   totalLiters += currentLiters;
  
  const uint64_t newCustomerId = 19466455ULL; 
  const float newUserTotalUsage = totalLiters / 1000.0; 
  const float newUserTotalCost = newUserTotalUsage * 2600.0; 
  const char* newCurrentDate = "2023-09-30";
  const char* newCurrentTime = "16:45";

  byte newData[50];

    memcpy(&newData[0], &newCustomerId, sizeof(newCustomerId));
    memcpy(&newData[8], &newUserTotalUsage, sizeof(newUserTotalUsage));
    memcpy(&newData[12], &newUserTotalCost, sizeof(newUserTotalCost));
    memcpy(&newData[16], newCurrentDate, strlen(newCurrentDate) + 1);
    memcpy(&newData[27], newCurrentTime, strlen(newCurrentTime) + 1);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Center "MODUL 2" at the top
    int16_t x = (display.width() - 8 * 6) / 2; // Assuming 6 characters for "MODUL 2"
    int16_t y = 0;
    
    display.setCursor(x, y);
    display.print("MASTER");
    
    // Place "Jumlah Data Modul 1" below "MODUL 2"
    x = 10;
    y = 9;
    
    display.setCursor(x, y);
    display.print("ID = ");
    display.println(newCustomerId);
    display.print("Biaya = Rp.");
    display.println(newUserTotalCost);
    delay(500);
    
    display.display();
    
    Serial.print("*");
    Serial.println(newCustomerId);
    Serial.print(",");
    Serial.println(newUserTotalUsage);
    Serial.print(",");
    Serial.println(newUserTotalCost);
    Serial.print(",");
    Serial.println(newCurrentDate);
    Serial.print(",");
    Serial.println(newCurrentTime);
    Serial.println("#");
    
  unsigned long send5 = millis();
  unsigned long rtimer1 = 0, timer1 = 1000;
  if (send5 - rtimer1 >= timer1) 
   {  
   sendDataToAntares(newCustomerId, newUserTotalUsage, newUserTotalCost, newCurrentDate, newCurrentTime);
   }
}
void bacanrf() {

  radio.openReadingPipe(1, beforeModuleId5);
  radio.openReadingPipe(2, beforeModuleId);
  radio.openReadingPipe(3, beforeModuleId2);
  radio.openReadingPipe(4, beforeModuleId3); 
  radio.openReadingPipe(5, beforeModuleId4);
  radio.startListening();

  if (radio.available()) {
    radio.read(&messageData, sizeof(messageData));
    dataCounter++;
    uint64_t customerId;
    memcpy(&customerId, &messageData[0], sizeof(customerId));
    
//    if (customerId == 19466444ULL) 
//      {  
//        dataReceived = true;
//      }
  }

  memcpy(&customerId, &messageData[0], sizeof(customerId));
  memcpy(&userTotalUsage, &messageData[8], sizeof(userTotalUsage));
  memcpy(&userTotalCost, &messageData[12], sizeof(userTotalCost));
  memcpy(currentDate, &messageData[16], sizeof(currentDate));
  memcpy(currentTime, &messageData[27], sizeof(currentTime));

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Center "MODUL 2" at the top
    int16_t x = (display.width() - 8 * 6) / 2; // Assuming 6 characters for "MODUL 2"
    int16_t y = 0;
    
    display.setCursor(x, y);
    display.print("MASTER");
    
    // Place "Jumlah Data Modul 1" below "MODUL 2"
    x = 10;
    y = 9;
    
    display.setCursor(x, y);
    display.print("ID = ");
    display.println(customerId);
    display.print("Biaya = Rp.");
    display.println(userTotalCost);
    delay(500);
    
    
    display.display();
    
    Serial.print("*");
    Serial.print(customerId);
    Serial.print(",");
    Serial.print(userTotalUsage);
    Serial.print(",");
    Serial.print(userTotalCost);
    Serial.print(",");
    Serial.print(currentDate);
    Serial.print(",");
    Serial.print(currentTime);
    Serial.println("#");

   delay(50);
   sendDataToAntares(customerId, userTotalUsage, userTotalCost, currentDate, currentTime);
  }

void sendDataToAntares(uint64_t id, float usage, float cost, const char* date, const char* time) {
  if ((id == 19466411ULL || id == 19466422ULL || id == 19466433ULL || id == 19466444ULL || id == 19466455ULL) &&
      (millis() - antaresLastSendTime >= antaresSendInterval)) {

    antares.add("ID Pelanggan", String(static_cast<unsigned long>(id)));

    char formattedCubicMeters[20];
    snprintf(formattedCubicMeters, sizeof(formattedCubicMeters), "%.2f m³", usage);
    antares.add("Total Penggunaan Air", String(formattedCubicMeters));

    char formattedCost[20];
    snprintf(formattedCost, sizeof(formattedCost), "Rp %.2f", cost);
    antares.add("Biaya", String(formattedCost));

    antares.add("Tanggal", String(date));
    antares.add("Waktu", String(time));

    antares.send(projectName, deviceName);

    antaresLastSendTime = millis();
  }
}
