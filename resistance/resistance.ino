#include "HX711.h"
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

const byte dt = 1;
const byte sck = 2;

Adafruit_ADS1115 ads;
HX711 scale;

long hxrawADC;
long adrawADC;
float hxpressure;
float adpressure;
float deltaPressure;

// values to be obtained from experiment
const float hxoffsetPressure = 5.8301;  // b
const float hxmmHgPerCount = 3.011542e-5;  // m

const float adoffsetPressure = -2.7141;
const float admmHgPerCount = 5.928369e-1;

unsigned long startTime;

// Formula logic: P = m*rawADC + b
float calcPressure(long inputCounts, float mmHgPerCount, float offsetPressure) {
  return (mmHgPerCount * inputCounts) + offsetPressure;
}

void setup() {
  Serial.begin(115200);
  
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  scale.begin(dt, sck);
  scale.set_gain(128);

  startTime = millis();
}

void loop() {
  unsigned long currentTime = millis() - startTime;

  hxrawADC = scale.read();
  hxpressure = calcPressure(hxrawADC, hxmmHgPerCount, hxoffsetPressure);

  adrawADC = ads.readADC_SingleEnded(0);
  adpressure = calcPressure(adrawADC, admmHgPerCount, adoffsetPressure);

  deltaPressure = adpressure - hxpressure;

  Serial.print(currentTime / 1000.0);
  Serial.print(","); 
  Serial.println(deltaPressure);

  delay(10);
}
