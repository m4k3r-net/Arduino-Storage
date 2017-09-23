#include "Storage.h"
#include "GPIO.h"
#include "SPI.h"
#include "Driver/MC23LC1024.h"

// Configure usage of software or hardware spi bus manager
// #define USE_SOFTWARE_SPI
#define USE_HARDWARE_SPI

#if defined(USE_SOFTWARE_SPI)
#include "Software/SPI.h"
Software::SPI<BOARD::D11, BOARD::D12, BOARD::D13> spi;
#elif defined(USE_HARDWARE_SPI)
#include "Hardware/SPI.h"
Hardware::SPI spi;
#endif

// External memory storage (128 Kbyte)
MC23LC1024<BOARD::D10> sram(spi);

// Sample with timestamp and value (analog)
struct sample_t {
  uint32_t timestamp;
  uint16_t value;
};

// Sample element, number of elements and external storage (60 Kbyte)
// Single sample in memory and 10000 vector members on storage
sample_t sample;
const size_t NMEMB = 10000;
Storage::Block vector(sram, &sample, sizeof(sample), NMEMB);

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  // Print size of storage, vector, sample and number of members
  Serial.print(F("sram.SIZE = "));
  Serial.println(sram.SIZE);
  Serial.print(F("vector.size = "));
  Serial.println(vector.size());
  Serial.print(F("vector.SIZE = "));
  Serial.println(vector.SIZE);
  Serial.print(F("vector.NMEMB = "));
  Serial.println(vector.NMEMB);
}

void loop()
{
  // Sample analog values, timestamp and write to storage
  for (size_t i = 0; i < vector.NMEMB; i++) {
    sample.timestamp = micros();
    sample.value = analogRead(A0);
    vector.write(i);
  }

  // Read back samples and calculate min, max and sum
  uint16_t min = UINT16_MAX;
  uint16_t max = 0;
  uint64_t sum = 0;
  for (size_t i = 0; i < vector.NMEMB; i++) {
    vector.read(i);
    if (sample.value < min) min = sample.value;
    if (sample.value > max) max = sample.value;
    sum += sample.value;
  }

  // Read back first and last sample timestamp to calculate
  // average microseconds per sample and write to storage
  uint32_t us = 0;
  vector.read(vector.NMEMB - 1);
  us = sample.timestamp;
  vector.read(0);
  us -= sample.timestamp;
  us /= vector.NMEMB;

  // Print the results; samples per second, min, max and average values
  Serial.println();
  Serial.print(F("sps = "));
  Serial.println(1000000.0 / us);
  Serial.print(F("min = "));
  Serial.println(min);
  Serial.print(F("max = "));
  Serial.println(max);
  Serial.print(F("sum = "));
  Serial.println((float) sum);
  Serial.print(F("avg = "));
  Serial.println((float) sum / vector.NMEMB);

  delay(5000);
}
