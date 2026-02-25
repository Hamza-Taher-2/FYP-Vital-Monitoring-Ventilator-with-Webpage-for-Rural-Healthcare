#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>
#include <math.h>
#include <LiquidCrystal_I2C.h>

#define SERVO_PIN      17   // any GPIO for servo signal
#define SPEED_POT_PIN  26   // ADC0 (GP26)
#define SWING_POT_PIN  27   // ADC1 (GP27)



// ---------------- WiFi ----------------
const char* ssid     = "Hamza";
const char* password = "raspberry";

WiFiServer server(80);

// ---------------- Time (NTP) ----------------
static const long TIME_OFFSET_SECONDS = 0;   // 0=GMT, 3600=BST
static const int  NTP_UPDATE_MS       = 60 * 1000;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", TIME_OFFSET_SECONDS, NTP_UPDATE_MS);

// ---------------- Sensor subfile prototypes ----------------
bool Heart_begin();
bool Heart_read(float &bpm, float &spo2);

bool Temp_begin();
bool Temp_read(float &tempC);

void Pressure_begin();
bool Pressure_read(float &kpa);

bool Humid_begin();
bool Humid_read(float &humidity);

void Servo_begin(int servoPin, int speedPotPin, int swingPotPin);
void Servo_update();


// ALARM
void Alarm_begin(int pinGP);
void Alarm_update(float bpm, float spo2, float tempC, float kpa);
bool Alarm_active();
String Alarm_text();
String Alarm_textJsonEscaped();





// ---------------- Live values ----------------
static float g_bpm      = NAN;
static float g_spo2     = NAN;
static float g_tempC    = NAN;
static float g_kpa      = NAN;
static float g_humid    = NAN;




// ---------------- LCD (20x4 I2C) ----------------
#define LCD_ADDR 0x20   // change to 0x20 or 0x7C if your scanner shows a different address
LiquidCrystal_I2C lcd(LCD_ADDR, 20, 4);

static uint32_t lastLcdMs = 0;

static void lcdPrintPadded(uint8_t row, const String &s) {
  lcd.setCursor(0, row);
  String out = s;
  if (out.length() > 20) out = out.substring(0, 20);
  while (out.length() < 20) out += ' ';
  lcd.print(out);
}

static String fmt1(float v, const char *suffix) {
  if (!isfinite(v)) return String("--") + suffix;
  return String(v, 1) + suffix;
}

static void updateLcd() {
  // Update ~2 times per second (fast enough, low flicker)
  const uint32_t LCD_MS = 500;
  if (millis() - lastLcdMs < LCD_MS) return;
  lastLcdMs = millis();

  // Row 0: Heart Rate
  String r0 = "HR: " + fmt1(g_bpm, " BPM");
  // Row 1: SpO2
  String r1 = "SpO2: " + fmt1(g_spo2, " %");
  // Row 2: Temperature
  String r2 = "Temp: " + fmt1(g_tempC, " C");
  // Row 3: Pressure
  String r3 = "Press: " + fmt1(g_kpa, " kPa");

  lcdPrintPadded(0, r0);
  lcdPrintPadded(1, r1);
  lcdPrintPadded(2, r2);
  lcdPrintPadded(3, r3);
}







// HR stats
static float    g_bpmSum   = 0;
static uint32_t g_bpmCount = 0;
static float    g_bpmPeak  = -1;
static float    g_bpmLow   = 9999;

static uint32_t lastSampleMs = 0;

static String twoDigits(int v) { return (v < 10) ? "0" + String(v) : String(v); }

static String timeStringNow() {
  timeClient.update();
  time_t raw = (time_t)timeClient.getEpochTime();
  struct tm t;
  gmtime_r(&raw, &t);
  return twoDigits(t.tm_hour) + ":" + twoDigits(t.tm_min);
}

static String dateStringNow() {
  timeClient.update();

  time_t raw = (time_t)timeClient.getEpochTime();
  struct tm t;
  gmtime_r(&raw, &t);

  // Format: DD/MM/YYYY
  String d  = String(t.tm_mday);
  String m  = String(t.tm_mon + 1);
  String yr = String(t.tm_year + 1900);

  if (d.length() == 1) d = "0" + d;
  if (m.length() == 1) m = "0" + m;

  return d + "/" + m + "/" + yr;
}


static String numOrDash(float v, int decimals = 0) {
  if (!isfinite(v)) return "--";
  return String(v, decimals);
}

static void sampleSensors() {
  const uint32_t SAMPLE_MS = 150;
  if (millis() - lastSampleMs < SAMPLE_MS) return;
  lastSampleMs = millis();

  float bpm = NAN, spo2 = NAN, tempC = NAN, kpa = NAN, humid = NAN;

  // Heart sensor
  if (Heart_read(bpm, spo2) && isfinite(bpm) && isfinite(spo2)) {
    g_bpm  = bpm;
    g_spo2 = spo2;
  } else {
    g_bpm  = NAN;
    g_spo2 = NAN;
  }

  // Temperature
  if (Temp_read(tempC) && isfinite(tempC)) {
    g_tempC = tempC;
  }

  // Pressure
  if (Pressure_read(kpa) && isfinite(kpa)) {
    g_kpa = kpa;
  }

  // Humidity
  if (Humid_read(humid) && isfinite(humid)) {
    g_humid = humid;
  }

  // Heart-rate statistics
  if (isfinite(g_bpm) && g_bpm > 0) {
    g_bpmSum += g_bpm;
    g_bpmCount++;
    if (g_bpm > g_bpmPeak) g_bpmPeak = g_bpm;
    if (g_bpm < g_bpmLow)  g_bpmLow  = g_bpm;
  }
}


// ---------------- HTTP helpers ----------------
static void sendResponse(WiFiClient &client, const char *contentType, const String &body) {
  client.println("HTTP/1.1 200 OK");
  client.print("Content-Type: "); client.println(contentType);
  client.println("Connection: close");
  client.print("Content-Length: "); client.println(body.length());
  client.println();
  client.print(body);
}

static void send404(WiFiClient &client) {
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println("404");
}

static String readLine(WiFiClient &client) {
  String s = client.readStringUntil('\n');
  s.trim();
  return s;
}

static void drainHeaders(WiFiClient &client) {
  while (client.connected()) {
    String h = client.readStringUntil('\n');
    if (h == "\r" || h.length() == 0) break;
  }
}



static String apiJson() {
  String t = timeStringNow();
  String dateS = dateStringNow();


  String bpmS  = isfinite(g_bpm) ? String((int)round(g_bpm)) : String("--");
  String avgS  = (g_bpmCount > 0) ? String((int)round(g_bpmSum / (float)g_bpmCount)) : String("--");
  String peakS = (g_bpmPeak >= 0) ? String((int)round(g_bpmPeak)) : String("--");
  String lowS  = (g_bpmLow < 9999) ? String((int)round(g_bpmLow)) : String("--");


  String tempS = isfinite(g_tempC) ? String(g_tempC, 1) : String("--");
  String spo2S = isfinite(g_spo2)  ? String((int)round(g_spo2)) : String("--");
  String kpaS  = isfinite(g_kpa)   ? String(g_kpa, 2) : String("--");
  String humS = isfinite(g_humid) ? String(g_humid, 0) : String("--");

  String s = "{";
  s += "\"date\":\"" + dateS + "\",";
  s += "\"time\":\"" + t + "\",";
  s += "\"bpm\":\"" + bpmS + "\",";
  s += "\"avg\":\"" + avgS + "\",";
  s += "\"peak\":\"" + peakS + "\",";
  s += "\"low\":\"" + lowS + "\",";
  s += "\"tempC\":\"" + tempS + "\",";
  s += "\"spo2\":\"" + spo2S + "\",";
  s += "\"kpa\":\"" + kpaS + "\"";
  s += ",\"hum\":\"" + humS + "\"";
  s += ",\"alarm\":" + String(Alarm_active() ? "true" : "false");
  s += ",\"alarmText\":\"" + Alarm_textJsonEscaped() + "\"";
  s += "}";
  return s;
}

static void handleHttp() {
  WiFiClient client = server.accept();
  if (!client) return;
  client.setTimeout(5);

  uint32_t start = millis();
  while (!client.available() && (millis() - start) < 1500) {
  delay(1);
  }
  if (!client.available()) { client.stop(); return; }

  String req = readLine(client);   // e.g. "GET /api HTTP/1.1"
  drainHeaders(client);
  


  if (req.startsWith("GET /api/data")) {
    sendResponse(client, "application/json; charset=utf-8", apiJson());

  }
  else if (req.startsWith("GET / ")) {
    sendDashboard(client);
  }
  else {
    send404(client);
  }

  delay(1);
  client.stop();
}









void setup() {

  Servo_begin(SERVO_PIN, SPEED_POT_PIN, SWING_POT_PIN);


  Alarm_begin(15); // GP15 buzzer/audio signal

  Serial.begin(115200);
  delay(200);

  // LCD init (Pico W default I2C: SDA=GP4, SCL=GP5)
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcdPrintPadded(0, "Booting...");

  Heart_begin();
  Temp_begin();
  Pressure_begin();
  Humid_begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting\n");
  while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print("."); }
  Serial.println();
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  timeClient.begin();

  server.begin();
  Serial.println("HTTP server on port 80. Open http://<IP>/");
}










void loop() {
  Servo_update();
  sampleSensors();
  Alarm_update(g_bpm, g_spo2, g_tempC, g_kpa);
  handleHttp();
  updateLcd();

}
