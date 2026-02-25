// Alarm_6.ino
// Buzzer + Web "interrupt" banner when readings exceed limits.
// Works as a subfile (tab) in Arduino IDE, called from FYP_6.ino.

#include <Arduino.h>

// ---------------- USER LIMITS (EDIT THESE) ----------------
// Heart rate (BPM)
static float HR_MIN   = 0;
static float HR_MAX   = 120;

// Blood oxygen (SpO2 %)
static float SPO2_MIN = 0;
static float SPO2_MAX = 100;

// Temperature (°C)
static float TEMP_MIN = 2.0;
static float TEMP_MAX = 30.0;

// Pressure (kPa)  (edit to match your sensor/meaning)
static float PRESS_MIN = 0.0;
static float PRESS_MAX = 110.0;

// ---------------- Buzzer settings ----------------
static int      g_alarmPin = 15;     // GP15 default
static uint16_t BEEP_FREQ_HZ = 2200; // audible tone for the amp/speaker
static uint16_t BEEP_ON_MS   = 140;
static uint16_t BEEP_OFF_MS  = 120;

// ---------------- Alarm state ----------------
static bool   g_alarmActive = false;
static String g_alarmText   = "";

// Beep pattern (non-blocking)
static bool     g_beepRunning = false;
static uint8_t  g_beepCount   = 0;     // number of beeps already completed
static bool     g_beepOn      = false; // currently sounding
static uint32_t g_beepT0      = 0;

// Minimal JSON string escaper (quotes + backslashes + newlines)
static String jsonEscape(const String &in) {
  String o;
  o.reserve(in.length() + 8);
  for (size_t i = 0; i < in.length(); i++) {
    char c = in[i];
    if (c == '\\') o += "\\\\";
    else if (c == '"') o += "\\\"";
    else if (c == '\n') o += "\\n";
    else if (c == '\r') { /* skip */ }
    else o += c;
  }
  return o;
}

static bool outOfRange(float v, float vMin, float vMax) {
  if (!isfinite(v)) return false;
  return (v < vMin) || (v > vMax);
}

static void startBeep3() {
  g_beepRunning = true;
  g_beepCount   = 0;
  g_beepOn      = false;
  g_beepT0      = 0;
}

// Called frequently; advances the beep state machine
static void updateBeep() {
  if (!g_beepRunning) return;

  const uint32_t now = millis();

  if (g_beepT0 == 0) {
    // Start immediately
    g_beepOn = true;
    g_beepT0 = now;
    tone(g_alarmPin, BEEP_FREQ_HZ);
    return;
  }

  if (g_beepOn) {
    if (now - g_beepT0 >= BEEP_ON_MS) {
      // turn off
      noTone(g_alarmPin);
      g_beepOn = false;
      g_beepT0 = now;
      g_beepCount++; // one beep completed
      if (g_beepCount >= 3) {
        g_beepRunning = false;
      }
    }
  } else {
    // off time
    if (now - g_beepT0 >= BEEP_OFF_MS) {
      if (!g_beepRunning) return;
      g_beepOn = true;
      g_beepT0 = now;
      tone(g_alarmPin, BEEP_FREQ_HZ);
    }
  }
}

// ---------------- Public API (called from main) ----------------
void Alarm_begin(int pinGP) {
  g_alarmPin = pinGP;
  pinMode(g_alarmPin, OUTPUT);
  digitalWrite(g_alarmPin, LOW);
  noTone(g_alarmPin);

  g_alarmActive = false;
  g_alarmText   = "";
  g_beepRunning = false;
}

// Optional: call from main if you want to set limits in code instead of editing above
void Alarm_setLimits(float hrMin, float hrMax,
                     float spo2Min, float spo2Max,
                     float tempMin, float tempMax,
                     float pressMin, float pressMax) {
  HR_MIN = hrMin; HR_MAX = hrMax;
  SPO2_MIN = spo2Min; SPO2_MAX = spo2Max;
  TEMP_MIN = tempMin; TEMP_MAX = tempMax;
  PRESS_MIN = pressMin; PRESS_MAX = pressMax;
}

// Evaluate readings + update buzzer + store message for the webpage.
void Alarm_update(float bpm, float spo2, float tempC, float kpa) {
  // Build alarm message(s)
  String msg = "";
  bool any = false;

  if (outOfRange(bpm, HR_MIN, HR_MAX)) {
    any = true;
    msg += "Heart Rate out of range (" + String((int)round(bpm)) + " BPM; limits " + String((int)HR_MIN) + "-" + String((int)HR_MAX) + ")";
  }
  if (outOfRange(spo2, SPO2_MIN, SPO2_MAX)) {
    if (any) msg += " | ";
    any = true;
    msg += "SpO2 out of range (" + String((int)round(spo2)) + "%; limits " + String((int)SPO2_MIN) + "-" + String((int)SPO2_MAX) + ")";
  }
  if (outOfRange(tempC, TEMP_MIN, TEMP_MAX)) {
    if (any) msg += " | ";
    any = true;
    msg += "Temperature out of range (" + String(tempC, 1) + "C; limits " + String(TEMP_MIN, 1) + "-" + String(TEMP_MAX, 1) + ")";
  }
  if (outOfRange(kpa, PRESS_MIN, PRESS_MAX)) {
    if (any) msg += " | ";
    any = true;
    msg += "Pressure out of range (" + String(kpa, 1) + " kPa; limits " + String(PRESS_MIN, 1) + "-" + String(PRESS_MAX, 1) + ")";
  }

  // Rising edge: start beeps once
  if (any && !g_alarmActive) {
    startBeep3();
  }

  g_alarmActive = any;
  g_alarmText   = any ? msg : "";

  updateBeep();
}

bool Alarm_active() {
  return g_alarmActive;
}

String Alarm_text() {
  return g_alarmText;
}

// Convenience for JSON
String Alarm_textJsonEscaped() {
  return jsonEscape(g_alarmText);
}
