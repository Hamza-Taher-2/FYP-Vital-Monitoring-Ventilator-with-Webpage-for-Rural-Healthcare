// Servo_W.ino
// Potentiometer + Motor
/*

// Config ---------------------------------------------------------------------------
#include <Arduino.h>
#include <Servo.h>
#include <math.h>


// Private State ---------------------------------------------------------------------------
static Servo myServo;
// Internal pin storage for the file
static int _oscServoPin;
static int _oscSpeedPot;
static int _oscSwingPot;
// Sets the oscillation state
static int   direction = 1;
static float position  = 90.0f;
// Sets the time
static unsigned long lastMoveTime = 0;
// Smooths the pot values
static float speedSmooth = 0.0f;
static float swingSmooth = 0.0f;


// Public State ---------------------------------------------------------------------------
void ServoOsc_begin(int servoPin, int speedPotPin, int swingPotPin) {

  // Sets internal pins
  _oscServoPin = servoPin;
  _oscSpeedPot = speedPotPin;
  _oscSwingPot = swingPotPin;

  // Maps RP2040 
  analogReadResolution(12);       
  myServo.attach(_oscServoPin);
  myServo.write(90);

  // Initialise smoothing so it doesn't crawl at startup
  // Initialises smotthing to stop crawl at the start
  speedSmooth = analogRead(_oscSpeedPot);
  swingSmooth = analogRead(_oscSwingPot);

  // Sets fixed constants
  position = 90.0f;
  direction = 1;
  lastMoveTime = millis();
}

void ServoOsc_update() {
  unsigned long now = millis();

  // SPEED POT --------------------------------------------
  int speedRaw = analogRead(_oscSpeedPot);
  speedSmooth = 0.90f * speedSmooth + 0.10f * speedRaw;

  float x = speedSmooth / 4095.0f;
  float curved = powf(x, 2.4f);

  const int minMs = 2;
  const int maxMs = 20;
  int stepInterval = minMs + (maxMs - minMs) * curved;
  stepInterval = constrain(stepInterval, minMs, maxMs);


  // SWING POT --------------------------------------------
  int swingRaw = analogRead(_oscSwingPot);
  swingSmooth = 0.90f * swingSmooth + 0.10f * swingRaw;

  float amplitude = map((int)swingSmooth, 0, 4095, 20, 2);
  amplitude = constrain(amplitude, 0, 90);

  float minAngle = 90.0f - amplitude;
  float maxAngle = 90.0f + amplitude;


  // SERVO UPDATE --------------------------------------------
  if (now - lastMoveTime >= (unsigned long)stepInterval) {
    lastMoveTime = now;

    if (amplitude < 0.5f) {
      position = 90.0f;
      direction = 1;
    } else {
      position += direction * 2.5f;

      if (position >= maxAngle) { position = maxAngle; direction = -1; }
      if (position <= minAngle) { position = minAngle; direction =  1; }
    }

    // Sets servo state
    myServo.write((int)position);
  }

  static uint32_t dbg = 0;
  if (millis() - dbg > 300) {
    dbg = millis();
    Serial.print("speedRaw="); Serial.print(speedRaw);
    Serial.print(" swingRaw="); Serial.print(swingRaw);
    //Serial.print(" intervalMs="); Serial.print(intervalMs);
    Serial.print(" pos="); Serial.print(position);
    Serial.print(" amp="); Serial.println(amplitude);
  }

}
*/

#include <Arduino.h>
#include <Servo.h>
#include <math.h>

// ---------------- Pins ----------------
static int g_servoPin;
static int g_speedPotPin;
static int g_swingPotPin;


// ---------------- Tuning ----------------
static const int SERVO_ADC_MIN = 40;
static const int SERVO_ADC_MAX = 4050;

static const float EMA_ALPHA = 0.18f;
static const uint32_t SERVO_WRITE_PERIOD_MS = 20;

static const float SWING_MAX_DEG = 90.0f;
static const float SPEED_MIN_DPS = 20.0f;
static const float SPEED_MAX_DPS = 260.0f;

static const float AMP_SLEW_DPS  = 600.0f;
static const float SPD_SLEW_DPS2 = 2500.0f;

// ---------------- State ----------------
static Servo servo;

static float speedFilt = 0.0f;
static float swingFilt = 0.0f;

static float ampDeg = 0.0f;
static float spdDps = 60.0f;

static float posDeg = 90.0f;
static float dir    = 1.0f;

static uint32_t lastMs = 0;
static uint32_t lastWriteMs = 0;
static int lastWrittenDeg = 90;

static bool ready = false;

// ---------------- Helpers ----------------
static float clampf(float x, float lo, float hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

static float ema(float prev, float x) {
  return prev + EMA_ALPHA * (x - prev);
}

static int readAdcAvg(int pin, int n = 4) {
  long sum = 0;
  for (int i = 0; i < n; i++) sum += analogRead(pin);
  return (int)(sum / n);
}

static float adcTo01(float adc) {
  float x = (adc - (float)SERVO_ADC_MIN) / (float)(SERVO_ADC_MAX - SERVO_ADC_MIN);
  return clampf(x, 0.0f, 1.0f);
}

// ---------------- PUBLIC API ----------------
void Servo_begin(int servoPin, int speedPotPin, int swingPotPin) {
  g_servoPin     = servoPin;
  g_speedPotPin  = speedPotPin;
  g_swingPotPin  = swingPotPin;

  analogReadResolution(12);

  pinMode(SPEED_POT_PIN, INPUT);
  pinMode(SWING_POT_PIN, INPUT);

  servo.attach(SERVO_PIN);
  servo.write(90);

  speedFilt = (float)readAdcAvg(SPEED_POT_PIN, 16);
  swingFilt = (float)readAdcAvg(SWING_POT_PIN, 16);

  ampDeg = 0.0f;
  spdDps = 60.0f;
  posDeg = 90.0f;
  dir = 1.0f;

  lastMs = millis();
  lastWriteMs = lastMs;
  lastWrittenDeg = 90;

  ready = true;
}

void Servo_update() {
  if (!ready) return;

  uint32_t now = millis();
  uint32_t dtMs = now - lastMs;
  if (dtMs == 0) return;
  lastMs = now;
  float dt = dtMs / 1000.0f;

  // ---- read + filter pots ----
  int speedRaw = readAdcAvg(g_speedPotPin, 8);
  int swingRaw = readAdcAvg(g_swingPotPin, 8);

  speedFilt = ema(speedFilt, (float)speedRaw);
  swingFilt = ema(swingFilt, (float)swingRaw);

  float speed01 = adcTo01(speedFilt); // 0..1 linear
  float swing01 = adcTo01(swingFilt); // 0..1 linear

  // ---- LINEAR targets ----
  float targetMaxAngle = swing01 * 90.0f;                 // 0..90 (absolute)
  float targetSpeed    = 20.0f + speed01 * (260.0f-20.0f);// linear

  // deadband so it can sit still at 0 without twitch
  if (targetMaxAngle < 1.0f) targetMaxAngle = 0.0f;

  // ---- slew-limit max angle + speed for smoothness ----
  float maxAngleStep = AMP_SLEW_DPS * dt;   // reuse your AMP_SLEW_DPS
  float aErr = targetMaxAngle - ampDeg;     // ampDeg now means "maxAngle"
  if (aErr >  maxAngleStep) ampDeg += maxAngleStep;
  else if (aErr < -maxAngleStep) ampDeg -= maxAngleStep;
  else ampDeg = targetMaxAngle;

  float maxSpeedStep = SPD_SLEW_DPS2 * dt;
  float sErr = targetSpeed - spdDps;
  if (sErr >  maxSpeedStep) spdDps += maxSpeedStep;
  else if (sErr < -maxSpeedStep) spdDps -= maxSpeedStep;
  else spdDps = targetSpeed;

  // ---- sweep between 0 and ampDeg (0..90) ----
  float minA = 0.0f;
  float maxA = ampDeg;          // <= 90

  if (maxA <= 0.5f) {
    posDeg = 0.0f;
    dir = 1.0f;
  } else {
    posDeg += dir * spdDps * dt;

    if (posDeg > maxA) { posDeg = maxA; dir = -1.0f; }
    if (posDeg < minA) { posDeg = minA; dir =  1.0f; }
  }

  // hard clamp so it NEVER exceeds 90
  if (posDeg < 0) posDeg = 0;
  if (posDeg > 90) posDeg = 90;

  // ---- write at fixed 50Hz ----
  if (now - lastWriteMs >= SERVO_WRITE_PERIOD_MS) {
    lastWriteMs = now;
    int deg = (int)round(posDeg);

    // write every time (smooth), don’t gate on >=1deg
    servo.write(deg);
    lastWrittenDeg = deg;
  }
}
