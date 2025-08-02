
// Define this to make the board act as the sender
//#define IS_SENDER

#include <esp_now.h>
#include <WiFi.h>

////////////////////////////////////////////////////////////////////////////////////
#ifdef IS_SENDER  //sender code
////////////////////////////////////////////////////////////////////////////////////

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UUIDs
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
String input = "";

// Replace with actual MAC of the receiver
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

void sendCommand(const String &msg) {
  esp_now_send(broadcastAddress, (uint8_t *)msg.c_str(), msg.length() + 1);
}

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      input = "";
      for (int i = 0; i < value.length(); i++) input += value[i];
      sendCommand(input);
    }
  }
};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("ESP-NOW Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  BLEDevice::init("ESP32_Display_Controller");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  Serial.println("BLE Ready. Connect and send commands.");
}

void loop() {}

////////////////////////////////////////////////////////////////////////////////////
#else  //receiver code
////////////////////////////////////////////////////////////////////////////////////

#include <Adafruit_NeoPixel.h>
#include "Digits.h"

#define PIN D0
#define NUMPIXELS 60
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define ORIENTATION0  //comment out to flip the display by 180 degrees

const bool* digits[] = {
  number0, number1, number2, number3, number4,
  number5, number6, number7, number8, number9,
  blank, letterN, letterR
};

int r = 0, g = 255, b = 0;
int lastNumber = 0;
String input = "";
bool newData;

int getPixelIndex(int x, int y) {
#ifdef ORIENTATION0
  return x * 6 + (5 - y);
#else
  return (9 - x) * 6 + y;
#endif
}

void drawDigit(int number, int xOffset, int yOffset, int r, int g, int b) {
  const bool* layout = digits[number];
  for (int y = 0; y < 6; y++) {
    for (int x = 0; x < 5; x++) {
      int i = y * 5 + x;
      if (layout[i]) {
        int px = x + xOffset;
        int py = y + yOffset;
        int index = getPixelIndex(px, py);
        pixels.setPixelColor(index, r, g, b);
      }
    }
  }
}

void OnDataRecv(const esp_now_recv_info_t* esp_now_info, const uint8_t* incomingData, int len) {
  input = String((char*)incomingData);
  Serial.println(input);
  input.trim();
  newData = true;
}

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.setBrightness(10);
  WiFi.mode(WIFI_STA);
  btStop();  // If using only WiFi
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if (newData) {
    pixels.clear();


    if (input == "N") {
      drawDigit(11, 2, 0, r, g, b);
    } else if (input == "R") {
      drawDigit(12, 3, 0, r, g, b);
    } else if (input == "OFF") {
      drawDigit(10, 0, 0, r, g, b);
    } else if (input.startsWith("COLOR#")) {
      String hexColor = input.substring(6);
      if (hexColor.length() == 6) {
        long number = strtol(hexColor.c_str(), NULL, 16);
        r = (number >> 16) & 0xFF;
        g = (number >> 8) & 0xFF;
        b = number & 0xFF;
        Serial.print("Color set to: ");
        Serial.print(r);
        Serial.print(", ");
        Serial.print(g);
        Serial.print(", ");
        Serial.println(b);
        int firstDigit = lastNumber / 10;
        int secondDigit = lastNumber % 10;
        int xOffset = (firstDigit == 0) ? -3 : 0;
        if (firstDigit == 0) firstDigit = 10;
        drawDigit(firstDigit, 0, 0, r, g, b);
        drawDigit(secondDigit, xOffset + 5, 0, r, g, b);
        pixels.show();
      }
    } else {
      int num = input.toInt();
      lastNumber = num;
      int firstDigit = num / 10;
      int secondDigit = num % 10;
      int xOffset = (firstDigit == 0) ? -3 : 0;
      if (firstDigit != 0) drawDigit(firstDigit, 0, 0, r, g, b);
      drawDigit(secondDigit, xOffset + 5, 0, r, g, b);
    }
    pixels.show();
    input = "";
    newData = false;
  }
}

#endif