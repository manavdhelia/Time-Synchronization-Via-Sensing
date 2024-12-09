// Arduino IDE code for ESP32 to collect data from IMU sensor, Ambient Light sensor, and Pressure sensor
// and synchronize timestamps with a Raspberry Pi via serial communication.

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> // For pressure sensor
#include <Adafruit_MPU6050.h> // For IMU sensor
#include <Adafruit_VEML6070.h> // For light sensor

// Sensor objects
Adafruit_BME280 bme;
Adafruit_MPU6050 mpu;
Adafruit_VEML6070 veml;

// Variables for sensor readings
float pressure, light, accelX, accelY, accelZ;
unsigned long lastEventTime = 0;
unsigned long localTimestamp = 0;
float correctionOffset = 0.0;

// Thresholds for event detection
float pressureThreshold = 1000.0; // Example threshold for pressure sensor
float lightThreshold = 500.0;    // Example threshold for light sensor
float accelThreshold = 1.0;      // Example threshold for IMU sensor

// Serial communication settings
#define SERIAL_BAUDRATE 115200

void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  // Initialize sensors
  if (!bme.begin(0x76)) {
    Serial.println("Failed to initialize BME280 sensor!");
    while (1);
  }

  if (!mpu.begin()) {
    Serial.println("Failed to initialize MPU6050 IMU!");
    while (1);
  }

  veml.begin(VEML6070_1_T);
  if (false) {
    Serial.println("Failed to initialize VEML6070 light sensor!");
    while (1);
  }

  Serial.println("Sensors initialized.");
}

void loop() {
  // Read sensors
  pressure = bme.readPressure() / 100.0F; // Convert to hPa
  light = veml.readUV();
  sensors_event_t accel;
  mpu.getAccelerometerSensor()->getEvent(&accel);
  accelX = accel.acceleration.x;
  accelY = accel.acceleration.y;
  accelZ = accel.acceleration.z;

  // Check for events
  bool pressureEvent = (pressure > pressureThreshold);
  bool lightEvent = (light > lightThreshold);
  bool accelEvent = (sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ) > accelThreshold);

  if (pressureEvent || lightEvent || accelEvent) {
    localTimestamp = millis() + correctionOffset;
    Serial.print("EVENT: ");
    Serial.print("Pressure: ");
    Serial.print(pressureEvent ? "YES" : "NO");
    Serial.print(", Light: ");
    Serial.print(lightEvent ? "YES" : "NO");
    Serial.print(", Acceleration: ");
    Serial.print(accelEvent ? "YES" : "NO");
    Serial.print(", Timestamp: ");
    Serial.println(localTimestamp);

    // Send timestamp to Raspberry Pi
    Serial.print("TIMESTAMP:");
    Serial.println(localTimestamp);
    delay(100); // Simulate communication latency

    // Wait for correction from Raspberry Pi
    while (Serial.available() > 0) {
      String receivedData = Serial.readStringUntil('\n');
      if (receivedData.startsWith("CORRECTION:")) {
        correctionOffset = receivedData.substring(11).toFloat();
        localTimestamp += correctionOffset;
        Serial.print("Corrected Timestamp: ");
        Serial.println(localTimestamp);
      }
    }
  }

  delay(1000); // Sample every second
}
