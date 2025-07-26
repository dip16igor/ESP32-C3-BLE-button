#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define BUTTON1_PIN 0
#define BUTTON2_PIN 1
#define LED_PIN 8 // Built-in LED pin for ESP32-C3 Super mini
#define LED_BLUE_PIN 2
#define LED_GREEN_PIN 3
#define LED_RED_PIN 4

BLECharacteristic *buttonChar;
bool isAdvertising = false;
BLEServer *pServer = nullptr; // Global pointer to the server

// Global variables for pressure simulation
float pressure = 0.0f;
float targetPressure = 0.0f;
// Low-pass filter coefficient.
// Update period is ~50ms. Rise time to 95% is ~1.25s.
// 3*tau = 1.25s => tau = 417ms. alpha = T/tau = 50/417 ~= 0.12
const float filterAlpha = 0.12f;

class ButtonCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic) override
  {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0)
    {
      uint8_t command = (uint8_t)value[0];
      Serial.print("Received command via BLE write: ");
      Serial.println(command);

      switch (command)
      {
      case 1:
        targetPressure = 40.0f;
        digitalWrite(LED_GREEN_PIN, HIGH);
        digitalWrite(LED_RED_PIN, LOW);
        break;
      case 2:
        targetPressure = -40.0f;
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(LED_RED_PIN, HIGH);
        break;
      case 0:
      case 3: // Handle 3 as a command to return to 0, same as original logic
      default:
        targetPressure = 0.0f;
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(LED_RED_PIN, LOW);
        break;
      }
    }
  }
  void onRead(BLECharacteristic *pCharacteristic) override
  {
    digitalWrite(LED_GREEN_PIN, !digitalRead(LED_GREEN_PIN)); // Toggle green LED on read
    pCharacteristic->setValue(pressure);                      // Provide the current pressure value on read
  }
  void onNotify(BLECharacteristic *pCharacteristic) override
  {
    // This callback is invoked after a notification is sent.
    // Serial output is commented out to avoid flooding the console during frequent sends.
    // Serial.println("Notification sent to client");
  }
  void onStatus(BLECharacteristic *pCharacteristic, Status s, uint32_t code) override
  {
    if (s == SUCCESS_NOTIFY)
    {
      // Serial output and LED blinking are commented out to avoid flooding the console.
      // digitalWrite(LED_GREEN_PIN, !digitalRead(LED_GREEN_PIN));
      // Serial.println("Notification was successful");
    }
    else
    {
      Serial.println("Notification failed");
    }
  }
};

class ServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer) override
  {
    isAdvertising = false;            // Set the flag to indicate we are not advertising
    digitalWrite(LED_PIN, HIGH);      // Turn off the built-in LED when a client connects
    digitalWrite(LED_BLUE_PIN, HIGH); // Turn on the blue LED
    // Reset pressure on new connection
    pressure = 0.0f;
    targetPressure = 0.0f;
    Serial.println("BLE client connected");
  }
  void onDisconnect(BLEServer *pServer) override
  {
    digitalWrite(LED_BLUE_PIN, LOW); // Turn off the blue LED
    Serial.println("BLE client disconnected, restarting advertising");
    pServer->getAdvertising()->start(); // The correct way to restart advertising
    isAdvertising = true;               // Set the flag to indicate we are advertising
    digitalWrite(LED_PIN, LOW);         // Turn on the built-in LED to indicate advertising mode
  }
};

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Turn on built-in LED initially
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  digitalWrite(LED_BLUE_PIN, LOW);  // Turn off blue LED
  digitalWrite(LED_GREEN_PIN, LOW); // Turn off green LED
  digitalWrite(LED_RED_PIN, LOW);   // Turn off red LED

  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }
  Serial.println("=== ESP32-C3 BLE Pressure Service Emulator ===");

  // Initialize buttons
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  BLEDevice::init("C1-Emulator");
  BLEDevice::setMTU(23);
  pServer = BLEDevice::createServer(); // Initialize the global server pointer
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pressureService = pServer->createService("12345678-1234-1234-1234-123456789abc");
  buttonChar = pressureService->createCharacteristic(
      "abcdefab-1234-5678-9abc-abcdefabcdef",
      BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_WRITE_NR);
  buttonChar->addDescriptor(new BLE2902());
  buttonChar->setCallbacks(new ButtonCallbacks());
  buttonChar->setValue(pressure); // Initialize with a float value
  pressureService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(pressureService->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->start(); // Start advertising
  isAdvertising = true;  // Set the flag to indicate we are advertising
  Serial.println("BLE Pressure service started! Connect and write to the characteristic.");
  digitalWrite(LED_PIN, HIGH); // Turn off the built-in LED after setup
}

void loop()
{
  static unsigned long lastUpdateTime = 0;
  // Static variables to store the previous state of the buttons
  static bool button1_was_pressed = false;
  static bool button2_was_pressed = false;

  // Read the current state of the buttons (LOW = pressed)
  bool button1_is_pressed = (digitalRead(BUTTON1_PIN) == LOW);
  bool button2_is_pressed = (digitalRead(BUTTON2_PIN) == LOW);

  // --- Button handling logic ---

  // Event: button 1 was just pressed
  if (button1_is_pressed && !button1_was_pressed)
  {
    targetPressure = 50.0f;
  }

  // Event: button 2 was just pressed
  if (button2_is_pressed && !button2_was_pressed)
  {
    targetPressure = -50.0f;
  }

  // Event: button 1 was just released
  if (!button1_is_pressed && button1_was_pressed)
  {
    // If button 2 is still pressed, set its target value, otherwise set to 0
    targetPressure = button2_is_pressed ? -50.0f : 0.0f;
  }

  // Event: button 2 was just released
  if (!button2_is_pressed && button2_was_pressed)
  {
    // If button 1 is still pressed, set its target value, otherwise set to 0
    targetPressure = button1_is_pressed ? 50.0f : 0.0f;
  }

  // Save the current button states for the next iteration
  button1_was_pressed = button1_is_pressed;
  button2_was_pressed = button2_is_pressed;

  // Update and send the pressure value every 50 ms
  if (millis() - lastUpdateTime >= 50)
  {
    // Smoothly change the pressure using the filter
    pressure = filterAlpha * targetPressure + (1.0f - filterAlpha) * pressure;

    // "Snap" to the target value when the difference is very small
    if (abs(pressure - targetPressure) < 0.01f)
    {
      pressure = targetPressure;
    }

    // Send a notification if a client is connected
    if (pServer != nullptr && pServer->getConnectedCount() > 0)
    {
      buttonChar->setValue(pressure);
      buttonChar->notify();
    }

    lastUpdateTime = millis();
  }

  // Blink the LED in advertising mode
  if (isAdvertising)
  {
    static unsigned long lastBlinkTime = 0;
    if (millis() - lastBlinkTime >= 300)
    {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle the built-in LED
      lastBlinkTime = millis();
    }
  }
  else
  {
    // If not advertising (i.e., connected), the LED is off
    digitalWrite(LED_PIN, HIGH);
  }
}
