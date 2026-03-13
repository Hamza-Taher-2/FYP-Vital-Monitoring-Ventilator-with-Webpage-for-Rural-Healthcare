// Heart_W.ino
// Blood Oxygen + Heart Rate 


// Config ---------------------------------------------------------------------------
#include <Wire.h>
#include "DFRobot_BloodOxygen_S.h"

#define I2C_ADDRESS 0x57


// Private State ---------------------------------------------------------------------------
static DFRobot_BloodOxygen_S_I2C MAX30102(&Wire, I2C_ADDRESS);
static bool g_heartOk = false;


// Public State ---------------------------------------------------------------------------

bool Heart_begin() {

  // Pin allocation
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  // Loop to activate
  for (int i = 0; i < 10; i++) {
    if (MAX30102.begin()) {
      g_heartOk = true;
      break;
    }
    delay(200);
  }

  // Sensor Detection
  if (!g_heartOk) {
    return false;
  }

  // Starts collecting the data
  MAX30102.sensorStartCollect();

  return true;
}

bool Heart_read(float &bpm, float &spo2) {

  // Sets NAN if not detected
  if (!g_heartOk) {
    bpm = NAN;
    spo2 = NAN;
    return false;
  }

  // Uses library
  MAX30102.getHeartbeatSPO2();
  float b = MAX30102._sHeartbeatSPO2.Heartbeat;
  float s = MAX30102._sHeartbeatSPO2.SPO2;

  // Filtering Data
  if (b <= 0 || b > 250) b = NAN;
  if (s <= 0 || s > 100) s = NAN;

  // Sets the value
  bpm = b;
  spo2 = s;
  return true;
}
