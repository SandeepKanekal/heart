#include "HX711.h"

const int dt = 3;
const int sck = 4;

HX711 scale;

long rawADC = 0;
float pressure = 0.0; // pressure in mmHg

// values obtained from experiment. May change depending on transducer model and amplifier.
const float offsetPressure = 5.830111;  // b
const float mmHgPerCount = 3.011542e-5;  // m

unsigned long startTime;

// Formula logic: P = m*rawADC + b
float calcPressure(long inputCounts) {
  return (mmHgPerCount * inputCounts) + offsetPressure;
}

void setup() {
  Serial.begin(115200);

  scale.begin(dt, sck);
  scale.set_gain(128);

  startTime = millis();
}

void loop() {
  unsigned long currentTime = millis() - startTime;

  rawADC = scale.read();
  pressure = calcPressure(rawADC);

  Serial.print(currentTime / 1000.0);
  Serial.print(",");
  Serial.println(pressure);

  delay(10);
}