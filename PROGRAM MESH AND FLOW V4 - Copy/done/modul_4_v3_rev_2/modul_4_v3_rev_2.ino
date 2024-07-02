
//ping pong 

#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RF24 radio(10, 9);
const uint64_t otherModuleId = 0xF0F0F0F0E3LL;
const uint64_t otherModuleId3 = 0xF0F0F0F0E2LL;
const uint64_t selfModuleId = 0xF0F0F0F0E4LL;

int moduleType = 1;
unsigned long previousMillis = 0;
const long interval = 2000;
unsigned long previousCT1Millis = 0;
const long intervalCT1 = 1000;
unsigned long previousCT2Millis = 0;
const long intervalCT2 = 1500;

byte messageData[50];
byte combineSelfData[150];

bool dataReceived = false;
int dataCounter = 0;
int dataCounter2 = 0;
int dataCounter3 = 0;
int dataCounterNewCustomer = 0;

const int module2Pin = 7;
unsigned long ledOnMillis = 0;
const unsigned long ledDuration = 200;
const int sensorPin = A0;
unsigned long pulseDuration = 0;
float totalLiters = 0;
const float pulseToCubicMeters = 0.00000247;

struct PayloadStruct {
  char param[7];  // only using 6 characters for TX & RX payloads
//  uint8_t counter;
};
PayloadStruct payloadReqConn;
bool isReqConnModeSending = true;
bool isReqConnModeReceive = false;

bool isReadingNRFData = false;
unsigned long previousReadNRFMillis = 0;
const long intervalReadNRF = 10000;

bool isSendingNRFData = false;

unsigned long previousReqConnMillis = 0;
const long intervalReqConn = 5000;

void requestconn() {
  //Sending data
  unsigned long currentMillis = millis();
  if(isReqConnModeSending) {
//    digitalWrite(module2Pin, LOW);
      Serial.println("Sending...");
      radio.openWritingPipe(selfModuleId); //self
//    delay(10);
//    byte data[50];
//    const char* param = "PING";
//    memcpy(&data[0], &selfModuleId, sizeof(selfModuleId));
//    memcpy(&data[strlen(param)-1], &param, strlen(param)+1);
    
    if(moduleType == 1) {
      memcpy(payloadReqConn.param, "PONG", 6);  // set the outgoing message
      radio.stopListening();
      radio.write(&payloadReqConn, sizeof(payloadReqConn));
      isReadingNRFData = true;
      isSendingNRFData = false;
      isReqConnModeSending = false;
      isReqConnModeReceive = false;
    }
    if(moduleType == 3) {
      memcpy(payloadReqConn.param, "PING", 6);  // set the outgoing message
      
      radio.stopListening();   
      radio.write(&payloadReqConn, sizeof(payloadReqConn));
      isReqConnModeSending = false;
      isReqConnModeReceive = true;
    }
//    delay(1000);
  }else if(currentMillis - previousReqConnMillis >= intervalReqConn){
//    isModeSending = true;
//    isModeReceive = false;
    previousMillis = currentMillis;
  }
  //receive data
  if(isReqConnModeReceive) {
    //Serial.println("Receiving...");
//    digitalWrite(module2Pin, HIGH);
    radio.startListening();
    unsigned long start_timeout = millis();  // timer to detect no response
      while (!radio.available()) {             // wait for response or timeout
        if (millis() - start_timeout > 200){ // only wait 200 ms
//          digitalWrite(module2Pin, LOW);
          break;
        }
      }
    radio.stopListening();  // put back in TX mode
//    delay(100);
//    digitalWrite(module2Pin, LOW);
//    delay(100);
  }
  if (radio.available() && isReqConnModeReceive) {
    Serial.println("Receiving...");
    if(moduleType == 1) radio.openReadingPipe(1, otherModuleId); //to
    if(moduleType == 3) radio.openReadingPipe(2, otherModuleId3); //to
//    delay(10);

    PayloadStruct received;
    radio.read(&received, sizeof(received));  // get payload from RX FIFO
//    payloadReqConn.counter = received.counter;       // save incoming counter for next outgoing counter

    Serial.println(received.param);
    if(strcmp(received.param,"PING") == 0 && moduleType == 1) {
      isReqConnModeSending = true;
      isReqConnModeReceive = false;
      
//      digitalWrite(module2Pin, HIGH);
//      delay(100);
//      digitalWrite(module2Pin, LOW);
    }else if(strcmp(received.param,"PONG") == 0 && moduleType == 3) {
      isReqConnModeSending = false;
      isReqConnModeReceive = false;

      isReadingNRFData = false;
      isSendingNRFData = true;
      
//      digitalWrite(module2Pin, HIGH);
//      delay(100);
//      digitalWrite(module2Pin, LOW);
    }
  }  
}

void setup() {
   radio.begin();
   radio.openWritingPipe(selfModuleId); //self
   radio.openReadingPipe(1, otherModuleId); //to
   radio.openReadingPipe(2, otherModuleId3);
  
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(85);

  pinMode(module2Pin, OUTPUT);
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.display();
  delay(2000);
  display.clearDisplay();
}

  void lcd() {

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
  
    int16_t x = (display.width() - 8 * 6) / 2; 
    int16_t y = 0;
    
    display.setCursor(x, y);
    display.print("MODUL 4");
    
    // Place "Jumlah Data Modul 1" below "MODUL 2"
    x = 10;
    y = 9;
    
    display.setCursor(x, y);
    display.print("Jumlah Data Modul 3 = ");
    display.println(dataCounterNewCustomer);  
    display.display();
  }


void bacanrf() {
  unsigned long currentMillis = millis();
  
  radio.openReadingPipe(1, otherModuleId);
  radio.startListening();

  if (radio.available()) {
    radio.read(&messageData, sizeof(messageData));
    dataReceived = true;
    dataCounter++;

    uint64_t customerId;
    memcpy(&customerId, &messageData[0], sizeof(customerId));

    if (customerId == 19466422ULL) {
      dataCounterNewCustomer++;
    }
    previousReadNRFMillis = currentMillis;
  }else{
    if(currentMillis - previousReadNRFMillis >= intervalReadNRF) {
      isReqConnModeSending = true;
      isReqConnModeReceive = false;
      
//      isReadingNRFData = false;
//      isSendingNRFData = true;
      moduleType = 3;
      Serial.println("next step");
      previousReadNRFMillis = currentMillis;
    }
  }

  if(dataReceived) {
    Serial.print("Data yang diterima dalam bentuk byte array: ");
    for (int i = 0; i < sizeof(messageData); i++) {
      Serial.print(messageData[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  
    uint64_t customerId;
    float userTotalUsage;
    float userTotalCost;
    char currentDate[11];
    char currentTime[8];
  
    memcpy(&customerId, &messageData[0], sizeof(customerId));
    memcpy(&userTotalUsage, &messageData[8], sizeof(userTotalUsage));
    memcpy(&userTotalCost, &messageData[12], sizeof(userTotalCost));
    memcpy(currentDate, &messageData[16], sizeof(currentDate));
    memcpy(currentTime, &messageData[27], sizeof(currentTime));
  
    Serial.print("ID Pelanggan: ");
    Serial.println(static_cast<unsigned long>(customerId));
  
    Serial.print("Total Penggunaan Air: ");
    Serial.print(userTotalUsage, 2);
    Serial.println(" m³");
    Serial.print("Biaya: Rp");
    Serial.println(userTotalCost);
    Serial.print("Tanggal: ");
    Serial.println(currentDate);
    Serial.print("Waktu: ");
    Serial.println(currentTime); 
//    createdata();
  }
  lcd(); // Update OLED display
}

void createdata(){
  pulseDuration = pulseIn(sensorPin, FALLING);
  float currentLiters = pulseDuration * pulseToCubicMeters;
  totalLiters += currentLiters;

  const uint64_t newCustomerId = 19466422ULL;
  const float newUserTotalUsage = totalLiters / 1000.0;
  const float newUserTotalCost = newUserTotalUsage * 2600.0;
  const char *newCurrentDate = "2023-09-30";
  const char *newCurrentTime = "16:45";

  byte newData[30];
  memcpy(&newData[0], &newCustomerId, sizeof(newCustomerId));
  memcpy(&newData[8], &newUserTotalUsage, sizeof(newUserTotalUsage));
  memcpy(&newData[12], &newUserTotalCost, sizeof(newUserTotalCost));
  memcpy(&newData[16], newCurrentDate, strlen(newCurrentDate) + 1);
  memcpy(&newData[27], newCurrentTime, strlen(newCurrentTime) + 1);

  Serial.print("ID Pelanggan Baru: ");
  Serial.println(static_cast<unsigned long>(newCustomerId));
  
  Serial.print("Total Penggunaan Air Baru: ");
  Serial.print(newUserTotalUsage, 5);
  Serial.println(" m³");
  Serial.print("Biaya Baru: Rp");
  Serial.println(newUserTotalCost);
  Serial.print("Tanggal Baru: ");
  Serial.println(newCurrentDate);
  Serial.print("Waktu Baru: ");
  Serial.println(newCurrentTime);

  memcpy(&combineSelfData[sizeof(combineSelfData)], &newData, sizeof(newData));
}

void kirimnrf() {
  radio.stopListening();
  radio.openWritingPipe(selfModuleId);
  radio.write(&messageData, sizeof(messageData));
  Serial.print("Data modul 1: ");
  for (int i = 0; i < sizeof(messageData); i++) {
    Serial.print(messageData[i], HEX);
    Serial.print(" ");
  }
   uint64_t customerId;
  float userTotalUsage;
  float userTotalCost;
  char currentDate[11];
  char currentTime[8];

  memcpy(&customerId, &messageData[0], sizeof(customerId));
  memcpy(&userTotalUsage, &messageData[8], sizeof(userTotalUsage));
  memcpy(&userTotalCost, &messageData[12], sizeof(userTotalCost));
  memcpy(currentDate, &messageData[16], sizeof(currentDate));
  memcpy(currentTime, &messageData[27], sizeof(currentTime));

  Serial.print("ID Pelanggan: ");
  Serial.println(static_cast<unsigned long>(customerId));

  Serial.print("Total Penggunaan Air: ");
  Serial.print(userTotalUsage, 2);
  Serial.println(" m³");
  Serial.print("Biaya: Rp");
  Serial.println(userTotalCost);
  Serial.print("Tanggal: ");
  Serial.println(currentDate);
  Serial.print("Waktu: ");
  Serial.println(currentTime);
  Serial.println();
  dataCounter3++;
}

void kirimnrf2() {
  radio.stopListening();

  pulseDuration = pulseIn(sensorPin, FALLING);
  float currentLiters = pulseDuration * pulseToCubicMeters;
  totalLiters += currentLiters;

  const uint64_t newCustomerId = 19466433ULL;
  const float newUserTotalUsage = totalLiters / 1000.0;
  const float newUserTotalCost = newUserTotalUsage * 2600.0;
  const char *newCurrentDate = "2023-09-30";
  const char *newCurrentTime = "16:45";

  byte newData[50];

  memcpy(&newData[0], &newCustomerId, sizeof(newCustomerId));
  memcpy(&newData[8], &newUserTotalUsage, sizeof(newUserTotalUsage));
  memcpy(&newData[12], &newUserTotalCost, sizeof(newUserTotalCost));
  memcpy(&newData[16], newCurrentDate, strlen(newCurrentDate) + 1);
  memcpy(&newData[27], newCurrentTime, strlen(newCurrentTime) + 1);

  Serial.print("ID Pelanggan Baru: ");
  Serial.println(static_cast<unsigned long>(newCustomerId));

  Serial.print("Total Penggunaan Air Baru: ");
  Serial.print(newUserTotalUsage, 5);
  Serial.println(" m³");

  Serial.print("Biaya Baru: Rp");
  Serial.println(newUserTotalCost);

  Serial.print("Tanggal Baru: ");
  Serial.println(newCurrentDate);

  Serial.print("Waktu Baru: ");
  Serial.println(newCurrentTime);

  radio.openWritingPipe(selfModuleId);
  radio.write(&newData, sizeof(newData));

  dataCounter2++;
}

unsigned long rtimer0 = 0, timer0 = 1000;
unsigned long rtimer1 = 0, timer1 = 1500;

void loop() {
  unsigned long currentMillis = millis();
  unsigned long currentMillis2 = millis();
  unsigned long ct = millis();
  unsigned long ct2 = millis();
  requestconn();

  if(isReadingNRFData) {
    bacanrf();

//    Serial.print("Jumlah data diterima (ID 19466411): ");
//    Serial.println(dataCounterNewCustomer);
  
    if (dataReceived) {
      digitalWrite(module2Pin, HIGH);
      ledOnMillis = millis();
      dataReceived = false;
    }
  
    if (millis() - ledOnMillis >= ledDuration) {
      digitalWrite(module2Pin, LOW);
    }
  
    lcd(); // Update OLED display
  }else if(isSendingNRFData){
    if (currentMillis - previousCT1Millis >= intervalCT1 && dataCounter3 < 5) {
      kirimnrf();
      previousCT1Millis = ct;
      Serial.println("[1] senddata to another module");
    }
  
    if (currentMillis2 - previousCT2Millis >= intervalCT2 && dataCounter2 < 5) {
      kirimnrf2();
      previousCT2Millis = ct2;
      Serial.println("[2] senddata to another module");
    }
  }else{
  }
}
