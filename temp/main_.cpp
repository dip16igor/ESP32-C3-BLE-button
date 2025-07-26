#include <Arduino.h>
#include <NimBLEDevice.h>

#define BUTTON1_PIN 0
#define BUTTON2_PIN 1

NimBLECharacteristic *buttonChar;

// Callback для обработки записи в характеристику
class ButtonCharCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
    Serial.println("onWrite callback called!");
    std::string value = pCharacteristic->getValue();
    Serial.print("Received via BLE write: ");
    for (size_t i = 0; i < value.size(); ++i)
    {
      Serial.printf("%02X ", (uint8_t)value[i]);
    }
    Serial.println();
  }
};

class MyServerCallbacks : public NimBLEServerCallbacks
{
public:
  MyServerCallbacks()
  {
    Serial.println("MyServerCallbacks constructed");
  }
  void onConnect(NimBLEServer *pServer)
  {
    Serial.println("BLE client connected");
  }
  void onDisconnect(NimBLEServer *pServer)
  {
    Serial.println("BLE client disconnected, restarting advertising");
    NimBLEDevice::getAdvertising()->start();
  }
};

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }
  Serial.println("=== ESP32-C3 BLE BUTTON SERVICE ===");

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  NimBLEDevice::init("ESP32C3-BLE-Button");
  NimBLEDevice::setMTU(23);
  NimBLEServer *pServer = NimBLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks());
  // Создаём сервис с произвольным UUID
  NimBLEService *buttonService = pServer->createService("12345678-1234-1234-1234-123456789abc");
  // Характеристика для передачи состояния кнопок (2 байта: бит0 - кнопка1, бит1 - кнопка2)
  buttonChar = buttonService->createCharacteristic(
      "abcdefab-1234-5678-9abc-abcdefabcdef",
      NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR,
      0x11 // явные права: чтение и запись
  );
  buttonChar->setValue((uint8_t *)"\0\0", 2);
  buttonChar->setCallbacks(new ButtonCharCallbacks());
  Serial.println("ButtonCharCallbacks set!");
  buttonService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

  NimBLEAdvertisementData advData;
  advData.setName("ESP32C3-BLE-Button");
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->addServiceUUID(buttonService->getUUID());

  // pAdvertising->setScanResponse(true);         // чтобы имя точно попало в scan response
  pAdvertising->setName("ESP32C3-BLE-Button"); // ЯВНО добавить имя устройства!
  pAdvertising->start();
  Serial.println("BLE Button service started! Connect and subscribe to notifications.");
}

void loop()
{
  static uint8_t lastState = 0;
  uint8_t state = 0;
  if (digitalRead(BUTTON1_PIN) == LOW)
    state |= 0x01;
  if (digitalRead(BUTTON2_PIN) == LOW)
    state |= 0x02;

  if (state != lastState)
  {
    buttonChar->setValue(&state, 1);
    buttonChar->notify();
    Serial.printf("Button state changed: %02X\n", state);
    lastState = state;
  }
  delay(20); // антидребезг
}