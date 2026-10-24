#include <Arduino.h>

#define SIMULATION_MODE 1

// ESP32 GPIO assignments
#define MQ2_PIN 34
#define LDR_PIN 35
#define TRIG_PIN 5
#define ECHO_PIN 18

#define SDA_PIN 21
#define SCL_PIN 22

#define SAMPLE_INTERVAL_MS 1000

unsigned long lastSample = 0;
unsigned long sequenceNumber = 0;

#if !SIMULATION_MODE
#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
bool bmeReady = false;
#endif

void printNullableFloat(float value, bool valid, int decimals = 2) {
  if (valid && isfinite(value)) {
    Serial.print(value, decimals);
  } else {
    Serial.print("null");
  }
}

void printNullableInt(int value, bool valid) {
  if (valid) {
    Serial.print(value);
  } else {
    Serial.print("null");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(MQ2_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

#if !SIMULATION_MODE
  analogReadResolution(12);
  analogSetPinAttenuation(MQ2_PIN, ADC_11db);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  Wire.begin(SDA_PIN, SCL_PIN);
  bmeReady = bme.begin(0x76);

  if (!bmeReady) {
    bmeReady = bme.begin(0x77);
  }
#endif

  Serial.println("{\"type\":\"status\",\"message\":\"ESP32_READY\"}");
}

void readSensors(
  float &temperature,
  float &humidity,
  float &pressure,
  int &gasRaw,
  int &lightRaw,
  float &distanceCm,
  bool &environmentValid,
  bool &gasValid,
  bool &lightValid,
  bool &distanceValid
) {
#if SIMULATION_MODE
  // Deterministic simulated readings for software development.
  float phase = (sequenceNumber % 60) / 60.0f;

  temperature = 25.0f + 2.0f * sinf(phase * 6.2831853f);
  humidity = 55.0f + 5.0f * sinf(phase * 6.2831853f);
  pressure = 1013.0f + 2.0f * cosf(phase * 6.2831853f);

  gasRaw = 700 + (sequenceNumber % 20) * 5;
  lightRaw = 1800 + (sequenceNumber % 10) * 20;
  distanceCm = 100.0f - (sequenceNumber % 30);

  environmentValid = true;
  gasValid = true;
  lightValid = true;
  distanceValid = true;

#else
  environmentValid = bmeReady;

  if (environmentValid) {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0f;

    environmentValid =
      isfinite(temperature) &&
      isfinite(humidity) &&
      isfinite(pressure) &&
      humidity >= 0.0f &&
      humidity <= 100.0f;
  }

  gasRaw = analogRead(MQ2_PIN);
  lightRaw = analogRead(LDR_PIN);

  gasValid = true;
  lightValid = true;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);

  distanceValid = duration > 0;

  if (distanceValid) {
    distanceCm = duration * 0.0343f / 2.0f;

    if (distanceCm < 2.0f || distanceCm > 400.0f) {
      distanceValid = false;
    }
  }
#endif
}

void loop() {
  unsigned long now = millis();

  if (now - lastSample < SAMPLE_INTERVAL_MS) {
    delay(5);
    return;
  }

  lastSample = now;

  float temperature = NAN;
  float humidity = NAN;
  float pressure = NAN;
  float distanceCm = NAN;

  int gasRaw = 0;
  int lightRaw = 0;

  bool environmentValid = false;
  bool gasValid = false;
  bool lightValid = false;
  bool distanceValid = false;

  readSensors(
    temperature,
    humidity,
    pressure,
    gasRaw,
    lightRaw,
    distanceCm,
    environmentValid,
    gasValid,
    lightValid,
    distanceValid
  );

  Serial.print("{\"type\":\"sensor_data\"");
  Serial.print(",\"schema_version\":1");
  Serial.print(",\"seq\":");
  Serial.print(sequenceNumber++);
  Serial.print(",\"uptime_ms\":");
  Serial.print(now);

  Serial.print(",\"temperature_c\":");
  printNullableFloat(temperature, environmentValid);

  Serial.print(",\"humidity_pct\":");
  printNullableFloat(humidity, environmentValid);

  Serial.print(",\"pressure_hpa\":");
  printNullableFloat(pressure, environmentValid);

  Serial.print(",\"gas_raw\":");
  printNullableInt(gasRaw, gasValid);

  Serial.print(",\"light_raw\":");
  printNullableInt(lightRaw, lightValid);

  Serial.print(",\"distance_cm\":");
  printNullableFloat(distanceCm, distanceValid);

  Serial.print(",\"status\":{");
  Serial.print("\"bme280\":");
  Serial.print(environmentValid ? "true" : "false");
  Serial.print(",\"mq2\":");
  Serial.print(gasValid ? "true" : "false");
  Serial.print(",\"ldr\":");
  Serial.print(lightValid ? "true" : "false");
  Serial.print(",\"hc_sr04\":");
  Serial.print(distanceValid ? "true" : "false");
  Serial.println("}}");
}
