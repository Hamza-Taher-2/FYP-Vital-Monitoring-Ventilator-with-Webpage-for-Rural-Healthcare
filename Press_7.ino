// Press_W.ino
// Pressure Sensor


/*
// Config ---------------------------------------------------------------------------
#include <Arduino.h>
#define PRESSURE_PIN 28   // GP28 (ADC2)

// ADC constants
static const float VREF    = 3.3f;
static const int   ADC_MAX = 4095;

// MPX5100DP transfer function constants
static const float VS      = 3.3f;   // set this to the sensor supply you are ACTUALLY using
static const float OFFSET  = 0.04f;  // 4% of VS at 0 kPa
static const float SLOPE   = 0.009f; // 0.9% of VS per kPa

void Pressure_begin() {
  // No beginning needed
}

bool Pressure_read(float &kpa) {
  int reading = analogRead(PRESSURE_PIN);
  float vout = (reading / (float)ADC_MAX) * VREF;

  // MPX5100DP: P = (Vout/Vs - 0.04) / 0.009
  float pressure_kpa = ((vout / VS) - OFFSET) / SLOPE;

  if (pressure_kpa < 0) pressure_kpa = 0;
  kpa = pressure_kpa;
  return true;
}
*/

// Press_W.ino - MPX5100DP (3.3V supply, tare-to-zero)
#include <Arduino.h>
#define PRESSURE_PIN A2   // GP28 (ADC2)

static const float VREF    = 3.3f;
static const int   ADC_MAX = 4095;

static const float VS     = 3.3f;   // your sensor supply
static const float SLOPE  = 0.009f; // MPX5100DP slope term

static const int   N_SAMPLES = 32;

static bool  zeroed = false;
static float v0 = 0.0f;  // baseline Vout at rest (measured)

void Pressure_begin() {
  // optional:
  // analogReadResolution(12);
}

static float readVoutAvg() {
  long sum = 0;
  for (int i = 0; i < N_SAMPLES; i++) {
    sum += analogRead(PRESSURE_PIN);
    delayMicroseconds(150);
  }
  float raw = sum / (float)N_SAMPLES;
  return (raw / (float)ADC_MAX) * VREF;
}

bool Pressure_read(float &kpa) {
  float vout = readVoutAvg();

  // Tare at startup: whatever "rest" voltage is becomes zero pressure
  if (!zeroed) {
    v0 = vout;
    zeroed = true;
  }

  // Convert *change* in voltage to kPa
  // From Vout = Vs*(0.009*P + 0.04)  => dV = Vs*0.009*dP  => dP = dV/(Vs*0.009)
  float dp_kpa = (vout - v0) / (VS * SLOPE);

  if (dp_kpa < 0) dp_kpa = 0;   // clamp after tare
  kpa = dp_kpa;
  return true;
}
