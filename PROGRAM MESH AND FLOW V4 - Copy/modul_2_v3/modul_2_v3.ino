
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
const uint64_t beforeModuleId = 0xF0F0F0F0E2LL;
const uint64_t moduleId = 0xF0F0F0F0E3LL;
unsigned long previousMillis = 0;
const long interval = 2000;

byte messageData[50];
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

void setup() {
  radio.begin();
  radio.openReadingPipe(1, beforeModuleId);
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
    display.print("MODUL 2");
    
    // Place "Jumlah Data Modul 1" below "MODUL 2"
    x = 10;
    y = 9;
    
    display.setCursor(x, y);
    display.print("Jumlah Data Modul 1 = ");
    display.println(dataCounterNewCustomer);
    
    display.display();
  }

//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setTextColor(SSD1306_WHITE);
//  display.setCursor(0, 0);
//  display.print("MODUL 2: ");
//  display.setCursor(2, 1);
//  display.print("Jumlah Data Modul 1: ");
//  display.println(dataCounterNewCustomer);
//  display.display();


void bacanrf() {
  radio.openReadingPipe(1, beforeModuleId);
  radio.startListening();

  if (radio.available()) {
    radio.read(&messageData, sizeof(messageData));
    dataReceived = true;
    dataCounter++;

    uint64_t customerId;
    memcpy(&customerId, &messageData[0], sizeof(customerId));

    if (customerId == 19466411ULL) {
      dataCounterNewCustomer++;
    }
  }

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

  lcd(); // Update OLED display
}

void kirimnrf() {
  radio.stopListening();
  radio.openWritingPipe(moduleId);

  radio.write(&messageData, sizeof(messageData));
  Serial.print("Data modul 1: ");
  for (int i = 0; i < sizeof(messageData); i++) {
    Serial.print(messageData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  dataCounter3++;
}

void kirimnrf2() {
  radio.stopListening();

  pulseDuration = pulseIn(sensorPin, FALLING);
  float currentLiters = pulseDuration * pulseToCubicMeters;
  totalLiters += currentLiters;

  const uint64_t newCustomerId = 19466422ULL;
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

  radio.openWritingPipe(moduleId);
  radio.write(&newData, sizeof(newData));

  dataCounter2++;
}

unsigned long rtimer0 = 0, timer0 = 1000;
unsigned long rtimer1 = 0, timer1 = 1500;

void loop() {
  unsigned long ct = millis();
  unsigned long ct2 = millis();

  bacanrf();

  Serial.print("Jumlah data diterima (ID 19466411): ");
  Serial.println(dataCounterNewCustomer);

  if (ct - rtimer0 >= timer0 >= interval && dataCounter3 < 30) {
    kirimnrf();
    rtimer0 = ct;
  }

  if (ct2 - rtimer1 >= timer1 >= interval && dataCounter2 < 30) {
    kirimnrf2();
    rtimer1 = ct2;
  }

  if (dataReceived) {
    digitalWrite(module2Pin, HIGH);
    ledOnMillis = millis();
    dataReceived = false;
  }

  if (millis() - ledOnMillis >= ledDuration) {
    digitalWrite(module2Pin, LOW);
  }

  lcd(); // Update OLED display
}
