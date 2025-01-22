#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// =========== SSD1306 / U8g2 includes
#include <U8g2lib.h>
#include <Wire.h>

// ********** BLE Configuration **********
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef0"

#define XRP_WALLET_ADDRESS  "rh1Ms9YB16C5B4kBMDNQnvC7ybqPLHkrWg" // Example wallet address
#define CONTROL_PIN 8

// =========== OLED Screen Dimensions ========
constexpr int SCREEN_WIDTH   = 128;
constexpr int SCREEN_HEIGHT  = 64;

// Virtual “frame” for displaying text
constexpr int VIRTUAL_WIDTH  = 72;
constexpr int VIRTUAL_HEIGHT = 40;

// If the frame is too close to the top, you can shift it down by a few extra pixels
constexpr int SHIFT_DOWN     = 4;

constexpr int X_OFFSET = (SCREEN_WIDTH - VIRTUAL_WIDTH) / 2;
constexpr int Y_OFFSET = ((SCREEN_HEIGHT - VIRTUAL_HEIGHT) / 2) + SHIFT_DOWN;

// =========== Initialize U8g2 =============
// Using your working constructor with SDA=5 and SCL=6
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0, 
  U8X8_PIN_NONE, 
  /* clock=*/ 6, 
  /* data=*/ 5
);

// =========== Machine States =============
enum MachineState {
  STATE_IDLE,
  STATE_TRANSACTION_PROCESSING,
  STATE_TRANSACTION_COMPLETE,
  STATE_ERROR
};

// Global state
MachineState currentState = STATE_IDLE;

// Helper to get string for each state
const char* getStateString(MachineState s) {
  switch(s) {
    case STATE_IDLE:                   return "Idle";
    case STATE_TRANSACTION_PROCESSING: return "Processing";
    case STATE_TRANSACTION_COMPLETE:   return "Complete";
    case STATE_ERROR:                  return "Error";
    default:                           return "???";
  }
}

// =========== BLE Handling ================
class CommandCallback : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic) override {
    Serial.println("Characteristic read request received.");
    // Return the wallet address to the Kivy app
    pCharacteristic->setValue(XRP_WALLET_ADDRESS);
    Serial.print("Sent XRP Wallet Address: ");
    Serial.println(XRP_WALLET_ADDRESS);
  }

  void onWrite(BLECharacteristic* pCharacteristic) override {
    String receivedData = pCharacteristic->getValue().c_str();
    Serial.print("Received data: ");
    Serial.println(receivedData);

    // 1) TRANSACTION:...
    if (receivedData.startsWith("TRANSACTION:")) {
      String transactionID = receivedData.substring(12);
      Serial.print("Processing transaction ID: ");
      Serial.println(transactionID);

      if (transactionID.length() > 0) {
        Serial.println("Transaction validated.");
        digitalWrite(CONTROL_PIN, HIGH);
        Serial.println("Pin 8 set HIGH.");
        // Update state
        currentState = STATE_TRANSACTION_PROCESSING;
        pCharacteristic->setValue("TRANSACTION RECEIVED");
      } else {
        Serial.println("Invalid transaction data.");
        pCharacteristic->setValue("ERROR: INVALID TRANSACTION");
        currentState = STATE_ERROR;
      }
    }
    // 2) TRAN_COMPLETE:...
    else if (receivedData.startsWith("TRAN_COMPLETE:")) {
      String completionData = receivedData.substring(14);
      Serial.print("Transaction completion data: ");
      Serial.println(completionData);

      if (completionData.length() > 0) {
        Serial.println("Transaction completion signal received.");
        digitalWrite(CONTROL_PIN, HIGH);
        Serial.println("Pin 8 set HIGH for transaction completion.");
        currentState = STATE_TRANSACTION_COMPLETE;
        pCharacteristic->setValue("TRAN_COMPLETE RECEIVED");
      } else {
        pCharacteristic->setValue("ERROR: EMPTY COMPLETION DATA");
        currentState = STATE_ERROR;
      }
    }
    else {
      Serial.println("Unknown command");
      pCharacteristic->setValue("ERROR: Unknown command");
      currentState = STATE_ERROR;
    }
  }
};

// =========== Setup and Loop ==============
void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE + OLED...");

  // **** OLED Setup ****
  delay(1000); // Let the display power up
  if (!u8g2.begin()) {
    Serial.println("u8g2.begin() failed. Stuck here.");
    while(true) { delay(100); }
  }
  // Set desired display properties
  u8g2.setContrast(255);
  u8g2.setBusClock(400000);
  u8g2.setFont(u8g2_font_ncenB10_tr);

  // **** BLE Setup ****
  BLEDevice::init("ESP32_BLE_SERVER");
  BLEServer* pServer = BLEDevice::createServer();
  BLEService* pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic* pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new CommandCallback());
  pCharacteristic->setValue("hello");

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  Serial.println("BLE advertising started with UUID:");
  Serial.println(SERVICE_UUID);

  // **** Control Pin ****
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, LOW);

  // Start in IDLE state
  currentState = STATE_IDLE;
}

void loop() {
  // Clear buffer
  u8g2.clearBuffer();

  // Draw a frame for the "virtual" box
  u8g2.drawFrame(X_OFFSET, Y_OFFSET, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

  // Convert current state to text
  const char* textToDraw = getStateString(currentState);

  // Center text in the "virtual" box
  int textWidth  = u8g2.getStrWidth(textToDraw);
  int textHeight = u8g2.getMaxCharHeight();
  int textX      = X_OFFSET + (VIRTUAL_WIDTH - textWidth) / 2;
  int textY      = Y_OFFSET + (VIRTUAL_HEIGHT + textHeight) / 2 - 2; 
  u8g2.setCursor(textX, textY);
  u8g2.print(textToDraw);

  // Send buffer to display
  u8g2.sendBuffer();

  // Slight delay
  delay(500);
}


