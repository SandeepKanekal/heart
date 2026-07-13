#include "HX711.h"

const byte dt = 2;
const byte sck = 3;

HX711 scale;

long raw;
float airPressure;
const float mmHgPerCount = 3.011542e-5;  // m
const float offsetPressure = 5.8301;  // b

float calcPressure(long inputCounts) {
    return (mmHgPerCount * inputCounts) + offsetPressure;
}

void setup() {
    Serial.begin(115200);
    
    scale.begin(dt, sck);
    scale.set_gain(64);
}

void loop() {
    raw = scale.read();
    airPressure = calcPressure(raw);
    Serial.println(airPressure);
    delay(100);
}
