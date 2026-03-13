// Temp_W.ino
// Temperature


// Config ---------------------------------------------------------------------------
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 16  // GP16

// Private State ---------------------------------------------------------------------------
// Fixed constants
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);
static bool g_tempOk = false;

// Public State ---------------------------------------------------------------------------
bool Temp_begin() {
  sensors.begin();
  g_tempOk = true;
  return true;
}

bool Temp_read(float &tempC) {
  // Checks of value is NAN
  if (!g_tempOk) {
    tempC = NAN;
    return false;
  }

  // Sets values of temperature
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);

  // Checks of value is NAN
  if (t == DEVICE_DISCONNECTED_C) {
    tempC = NAN;
    return false;
  }

  // Sets the value for temp
  tempC = t;
  return true;
}
