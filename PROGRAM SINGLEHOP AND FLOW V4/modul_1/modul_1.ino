#include <RF24.h>

RF24 radio(10, 9);
const uint64_t moduleId = 0xF0F0F0F0E2LL;
unsigned long previousMillis = 0;
const long interval = 2000;

// Flow sensor
const int sensorPin = A0;
unsigned long pulseDuration = 0;
float totalLiters = 0;
const float pulseToCubicMeters = 0.00000247;

int dataCounter = 0; // Counter for data sent

void kirimnrf() {
  // Read flow sensor data
  pulseDuration = pulseIn(sensorPin, FALLING);
  float currentLiters = pulseDuration * pulseToCubicMeters;
  totalLiters += currentLiters;

  // Data that will be sent
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
  Serial.print(newUserTotalUsage, 8); // Display 5 decimal places for precision
  Serial.println(" m³");
  Serial.print("Biaya Baru: Rp");
  Serial.println(newUserTotalCost);
  Serial.print("Tanggal Baru: ");
  Serial.println(newCurrentDate);
  Serial.print("Waktu Baru: ");
  Serial.println(newCurrentTime);

  radio.write(&newData, sizeof(newData)); // Send new data to Module 1

  dataCounter++; // Increment the counter
}

void setup() {
  radio.begin();
  radio.openWritingPipe(moduleId);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(85);

  Serial.begin(115200);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval && dataCounter < 8) {
    kirimnrf();
    previousMillis = currentMillis;
  }
}
