/*
  This code is based on / adapts logic from the Pulsatile Pump project 
  by Herwald SE, Sze DY, Ennis DB, Vezeridis AM.
  Original License: CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// motor
const byte motorPin = 9;
int baseline = 5;
// data from haemod.uk/aortic
const byte flowVector[100] = {
  5,54,107,158,201,233,251,255,253,243,230,213,196,178,162,146,132,119,107,95,82,67,47,22,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};

// transducer
const int pressurePin = A1;
const double sense = 1.0 / (5.0 * pow(10.0, -6.0) * 5.0 * 1000.0);
int rawADC = 0;
float voltage = 0.0;
float pressure = 0.0;
float V0 = 0.0;
unsigned long startTime;

// push buttons
const byte pulseButton = 31;
const byte fracButton = 33;
const byte baseButton = 35;
const byte confButton = 37;

LiquidCrystal_I2C lcd(0x27, 16, 2);

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

void lcdMessage(const String& msg1, const String& msg2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg1);
  lcd.setCursor(0, 1);
  lcd.print(msg2);
}

// Formula: P = (V - V0) * Sensitivity
float calcPressure(float inputVoltage) {
  return (inputVoltage - V0) * sens;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();

  delay(1000);

  // find lcd address
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTramission() == 0) {
      I2CAddress = address;
      break;
    }
  }

  lcd = LiquidCrystal_I2C(I2CAddress, 16, 2);

  // initialise the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello World!")

  // set up the pins
  pinMode(motorPin, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(pulseButton, INPUT_PULLUP);
  pinMode(fracButton, INPUT_PULLUP);
  pinMode(baseButton, INPUT_PULLUP);
  pinMode(confButton, INPUT_PULLUP);

  // set parameters
  lcdMessage("Set pulse rate", "BPM = " + String(pulseRate));
  while (!buttonPressed(conButton)){
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
  while (!buttonPressed(conButton)){
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
  while (!buttonPressed(conButton)){
    if (buttonPressed(baseButton)){
      if (baseline < 20) {
        baseline = baseline + 5;
      } else {
        baseline = 5;
      }
      lcdMessage("Set baseline", "baseline = " + String(baseline));
    }
  }

  delay(1000);

  // set V0
  lcdMessage("Disconnect", "Transducer");
  delay(2000);
  lcdMessage("Press conf.", "Button");

  while (!buttonPressed(confButton)) {
    delay(10);
  }

  lcdMessage("Setting V0", "Please wait...");
  delay(500);

  int sampleADC = analogRead(pressurePin);
  V0 = sampleADC * (5.0/1023.0);

  lcdMessage("V0 set to:", String(V0) + " V");
  delay(1500);

  lcdMessage("Connect", "Transducer");
  delay(2000);
  lcdMessage("Press conf.", "Button");

  while (!buttonPressed(confButton)) {
    delay(10);
  }

  delay(500);

  lcdMessage("BPM=" + String(pulseRate) + " PF=" + String(pulseFraction), "baseline=" + String(baseline));
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
  digitalWrite(13, HIGH);  // turn on the built in led when the heartbeat begins
  for (int i = 0; i < bufferLength; i++){
    int currentFlow = flowVector[i];
    if (currentFlow < baseline) currentFlow = baseline;

    analogWrite(motorPin, currentFlow);

    if (i > 0) digitalWrite(13, LOW);

    unsigned long interval = 60000UL / ((unsigned long)pulseRate * bufferLength);

    unsigned long currentTime = millis() - startTime;

    // calculate pressure
    rawADC = analogRead(pressurePin);
    voltage = rawADC * (5.0 / 1023.0);
    pressure = calcPressure(voltage); 

    Serial.print(currentTime / 1000.0);
    Serial.print(",");
    Serial.println(pressure);
     
    delay(interval);
  }
}
