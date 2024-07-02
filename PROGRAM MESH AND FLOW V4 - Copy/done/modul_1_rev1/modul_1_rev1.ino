#include <RF24.h>

RF24 radio(10, 9);
const uint64_t otherModuleId = 0xF0F0F0F0E3LL;
const uint64_t selfModuleId = 0xF0F0F0F0E2LL;
unsigned long previousMillis = 0;
const long interval = 2000;

// Flow sensor
const int sensorPin = A0;
unsigned long pulseDuration = 0;
float totalLiters = 0;
const float pulseToCubicMeters = 0.00000247;

int dataCounter = 0; 

struct PayloadStruct {
  char param[7];  // only using 6 characters for TX & RX payloads
  uint8_t counter;
};
PayloadStruct payloadReqConn;
bool isReqConnModeSending = true;
bool isReqConnModeReceive = false;
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
//
//    memcpy(&data[0], &selfModuleId, sizeof(selfModuleId));
//    memcpy(&data[strlen(param)-1], &param, strlen(param)+1);
    memcpy(payloadReqConn.param, "PING", 6);  // set the outgoing message
    radio.stopListening();
    
    radio.write(&payloadReqConn, sizeof(payloadReqConn));

    isReqConnModeSending = false;
    isReqConnModeReceive = true;
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
    radio.openReadingPipe(1, otherModuleId); //to
//    delay(10);

    PayloadStruct received;
    radio.read(&received, sizeof(received));  // get payload from RX FIFO
//    payloadReqConn.counter = received.counter;       // save incoming counter for next outgoing counter

    Serial.println(received.param);
    if(strcmp(received.param,"PONG") == 0) {
      isReqConnModeSending = false;
      isReqConnModeReceive = false;
      Serial.println("connected");

      isSendingNRFData = true;
      
//      digitalWrite(module2Pin, HIGH);
//      delay(100);
//      digitalWrite(module2Pin, LOW);
    }
  }  
}

void kirimnrf() {
  // Read flow sensor data
  pulseDuration = pulseIn(sensorPin, FALLING);
  float currentLiters = pulseDuration * pulseToCubicMeters;
  totalLiters += currentLiters;

  // Dataku
  const uint64_t newCustomerId = 19466411ULL;
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

  Serial.print("ID Pelanggan Baru: ");
  Serial.println(static_cast<unsigned long>(newCustomerId));

  Serial.print("Total Penggunaan Air Baru: ");
  Serial.print(newUserTotalUsage, 8); 
  Serial.println(" m³");
  Serial.print("Biaya Baru: Rp");
  Serial.println(newUserTotalCost);
  Serial.print("Tanggal Baru: ");
  Serial.println(newCurrentDate);
  Serial.print("Waktu Baru: ");
  Serial.println(newCurrentTime);

  radio.write(&newData, sizeof(newData)); 

  dataCounter++; 
}

void setup() {
  radio.begin();
  radio.openWritingPipe(selfModuleId); //self
  radio.openReadingPipe(1, otherModuleId); //to
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(85);

  Serial.begin(115200);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval && dataCounter < 5 && isSendingNRFData) {
    kirimnrf();
    previousMillis = currentMillis;
  }else if(!isSendingNRFData){
    requestconn();
  }
}
