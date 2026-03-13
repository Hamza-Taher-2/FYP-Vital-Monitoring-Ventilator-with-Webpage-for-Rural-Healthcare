#include <math.h>
#include <DHT.h>

// ================= CONFIG =================
#define DHT_PIN   14     // Any GPIO between 9–20
#define DHT_TYPE  DHT11
// =========================================

// Internal DHT object
static DHT dht(DHT_PIN, DHT_TYPE);
static bool dhtReady = false;

/**
 * @brief Initialise the DHT11 sensor
 * @return true if initialised
 */
bool Humid_begin() {
  dht.begin();
  dhtReady = true;
  return true;
}

/**
 * @brief Read humidity from DHT11
 * @param humidity reference to store humidity (%)
 * @return true if read successful
 */
bool Humid_read(float &humidity) {
  if (!dhtReady) return false;

  float h = dht.readHumidity();

  if (isnan(h)) {
    return false;
  }

  humidity = h;
  return true;
}
