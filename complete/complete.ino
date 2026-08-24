/*
  This code is based on / adapts logic from the Pulsatile Pump project
  by Herwald SE, Sze DY, Ennis DB, Vezeridis AM.
  Original License: CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <Adafruit_ADS1X15.h>

// motor
const byte motorPin = 9;
byte baseline = 5;        // baseline flow rate
byte pulseRate = 60;      // beats per minute
byte pulseFraction = 25;  // percentage of the cycle that is systole
// data from haemod.uk/aortic
const byte flowVector[100] = {
  5, 54, 107, 158, 201, 233, 251, 255, 253, 243, 230, 213, 196, 178, 162, 146, 132, 119, 107, 95, 82, 67, 47, 22, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

// transducer values obtained from experiment. May change depending on transducer model and amplifier.
const float offsetPressure = 3.7781;               // b
const float mmHgPerCount = 1.159309e2 * 0.000125f;  // m
int16_t rawADC;
float pressure;
unsigned long startTime;

// compliance chamber air pressure
const float fluidHeight = 10.0;  // cm
const float rhog = 0.73556;      // for water, change for other fluids
const float compOffsetPressure = 5.8301;
const byte dt = 3;
const byte sck = 2;
const float compmmHgPerCount = 3.011542e-5;  // m
long airPressureRaw;
float airPressure;

HX711 scale;
Adafruit_ADS1115 ads;

// push buttons
const byte pulseButton = 31;
const byte fracButton = 33;
const byte baseButton = 35;
const byte confButton = 37;

byte findLCDAddress() {
  for (byte address = 0x08; address <= 0x77; address++) {
    Wire.beginTransmission(address);

    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 0x10)
        Serial.print("0");
      Serial.println(address, HEX);

      return address;
    }
  }

  return 0;
}

LiquidCrystal_I2C lcd(findLCDAddress(), 20, 4);

bool buttonPressed(byte pin) {
  if (digitalRead(pin) == LOW) {
    while (digitalRead(pin) == LOW) {
      // do nothing
    }
    delay(20);
    return true;
  }
  return false;
}

void lcdMessage(const String &msg1, const String &msg2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg1);
  lcd.setCursor(0, 1);
  lcd.print(msg2);
}

// Formula logic: P = m*rawADC + b
float calcPressure(long inputCounts, float mmHgPerCount, float offsetPressure) {
  return (mmHgPerCount * inputCounts) + offsetPressure;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1)
      ;
  }

  // set up the ADS1115 with AD623
  ads.setGain(GAIN_ONE);
  ads.setDataRate(RATE_ADS1115_250SPS);

  // set up the HX711 with the pressure transducer
  scale.begin(dt, sck);
  scale.set_gain(64);

  // initialise the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello World!");

  // set up the pins
  pinMode(motorPin, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(pulseButton, INPUT_PULLUP);
  pinMode(fracButton, INPUT_PULLUP);
  pinMode(baseButton, INPUT_PULLUP);
  pinMode(confButton, INPUT_PULLUP);

  // set parameters
  lcdMessage("Set pulse rate", "BPM = " + String(pulseRate));
  while (!buttonPressed(confButton)) {
    if (buttonPressed(pulseButton)) {
      if (pulseRate < 120) {
        pulseRate = pulseRate + 5;
      } else {
        pulseRate = 5;
      }
      lcdMessage("Set pulse rate", "BPM = " + String(pulseRate));
    }
  }

  lcdMessage("Set pulse frac.", "PF = " + String(pulseFraction));
  while (!buttonPressed(confButton)) {
    if (buttonPressed(fracButton)) {
      if (pulseFraction < 120) {
        pulseFraction = pulseFraction + 5;
      } else {
        pulseFraction = 25;
      }
      lcdMessage("Set pulse frac.", "PF = " + String(pulseFraction));
    }
  }

  lcdMessage("Set baseline", "baseline = " + String(baseline));
  while (!buttonPressed(confButton)) {
    if (buttonPressed(baseButton)) {
      if (baseline < 20) {
        baseline = baseline + 5;
      } else {
        baseline = 5;
      }
      lcdMessage("Set baseline", "baseline = " + String(baseline));
    }
  }
  delay(500);

  lcdMessage("BPM=" + String(pulseRate) + " PF=" + String(pulseFraction), "BL=" + String(baseline));
  // turn on the motor
  analogWrite(motorPin, 10);
  delay(1000);

  startTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  // update values while looping
  if (buttonPressed(pulseButton)) {
    if (pulseRate < 120) {
      pulseRate = pulseRate + 5;
    } else {
      pulseRate = 5;
    }
    lcdMessage("BPM=" + String(pulseRate) + " PF=" + String(pulseFraction), "baseline=" + String(baseline));
  }

  if (buttonPressed(fracButton)) {
    if (pulseFraction < 120) {
      pulseFraction = pulseFraction + 5;
    } else {
      pulseFraction = 25;
    }
    lcdMessage("BPM=" + String(pulseRate) + " PF=" + String(pulseFraction), "baseline=" + String(baseline));
  }

  if (buttonPressed(baseButton)) {
    if (baseline < 20) {
      baseline = baseline + 5;
    } else {
      baseline = 5;
    }
    lcdMessage("BPM=" + String(pulseRate) + " PF=" + String(pulseFraction), "baseline=" + String(baseline));
  }

  // motor controlling code
  int bufferLength = 125 - pulseFraction;
  unsigned long interval_micros = 60000000UL / ((unsigned long)pulseRate * bufferLength);
  unsigned long nextUpdate = micros();

  digitalWrite(13, HIGH);

  for (int i = 0; i < bufferLength; i++) {
    // Wait until the exact scheduled time
    while ((long)(micros() - nextUpdate) < 0) {
    }

    if (i > 0)
      digitalWrite(13, LOW);

    // Timestamp
    unsigned long currentTime = millis() - startTime;

    // Read sensors
    rawADC = ads.readADC_SingleEnded(0);
    if (scale.is_ready())
      airPressureRaw = scale.read();

    // Convert to pressure
    pressure = calcPressure(rawADC, mmHgPerCount, offsetPressure);
    airPressure = calcPressure(airPressureRaw, compmmHgPerCount, compOffsetPressure - rhog * fluidHeight);

    // Send data
    Serial.print(currentTime / 1000.0);
    Serial.print(",");
    Serial.print(pressure);
    Serial.print(",");
    Serial.println(airPressure);

    nextUpdate += interval_micros;

    // Update the motor
    int currentFlow = flowVector[i];
    if (currentFlow < baseline)
      currentFlow = baseline;

    analogWrite(motorPin, currentFlow);
  }
}
