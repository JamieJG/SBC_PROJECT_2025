#include <Wire.h>
#include <BleMouse.h>
#include <MPU6050.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// MPU6050
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// Servicio y característica personalizados
#define SERVICE_CUSTOM_UUID (uint16_t)0xFFF0
#define CHARACTERISTIC_CUSTOM_UUID (uint16_t)0xFFF1

BLECharacteristic* pCharacteristicCustom = nullptr;

class CustomBleMouse : public BleMouse {
public:
  CustomBleMouse(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel)
    : BleMouse(deviceName, deviceManufacturer, batteryLevel) {}

protected:
  void onStarted(BLEServer* pServer) override {
    // Servicio personalizado para coordenadas del giroscopio
    BLEService* customService = pServer->createService(SERVICE_CUSTOM_UUID);
    pCharacteristicCustom = customService->createCharacteristic(
      CHARACTERISTIC_CUSTOM_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristicCustom->addDescriptor(new BLE2902());
    customService->start();
    Serial.println("✅ Servicio personalizado (0xFFF0) iniciado.");
  }
};

CustomBleMouse bleMouse("Gyro Mouse", "ESP32", 100);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("Iniciando MPU6050...");
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 no conectado.");
    while (1)
      ;
  }
  Serial.println("MPU6050 conectado.");

  Serial.println("Iniciando BLE...");
  bleMouse.begin();  // Esto inicializa BLEDevice y arranca el servicio
}

void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  if (bleMouse.isConnected() && pCharacteristicCustom != nullptr) {
    char data[30];
    snprintf(data, sizeof(data), "%d,%d,%d", gx, gy, gz);
    pCharacteristicCustom->setValue(data);
    pCharacteristicCustom->notify();
    Serial.printf("🔁 Coordenadas: %s\n", data);
  }

  delay(3000);
}
