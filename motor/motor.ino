/*
  This code is based on / adapts logic from the Pulsatile Pump project 
  by Herwald SE, Sze DY, Ennis DB, Vezeridis AM.
  Original License: CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const byte motorPin = 9;
byte baseline = 5;
const byte flowVector[100] = {
  5,54,107,158,201,233,251,255,253,243,230,213,196,178,162,146,132,119,107,95,82,67,47,22,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};  // data from http://haemod.uk/aortic
byte pulseRate = 60;
byte pulseFraction = 25;

const byte pulseButton = 31;
const byte fracButton = 33;
const byte baseButton = 35;
const byte conButton = 37; // confirmation button

LiquidCrystal_I2C lcd(0x20, 16, 2);

bool buttonPressed(byte pin){
  if (digitalRead(pin) == LOW){
    delay(20);
    if (digitalRead(pin) == LOW){
      while (digitalRead(pin) == LOW){
      }
      delay(20);
      return true;
    }
  }
  return false;
}

void lcdMessage(const String& msg1, const String& msg2){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg1);
  lcd.setCursor(0, 1);
  lcd.print(msg2);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  delay(1000);

  byte I2CAddress = 0x20;
  
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found LCD at address: 0x");
      Serial.println(address, HEX);
      I2CAddress = address;
      break;
    }
  }

  lcd = LiquidCrystal_I2C(I2CAddress, 16, 2);

  // initialise the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print('Hello world');

  // set up the pins
  pinMode(motorPin, OUTPUT);
  pinMode(13, OUTPUT);

  // set up the buttons
  pinMode(pulseButton, INPUT_PULLUP);
  pinMode(fracButton, INPUT_PULLUP);
  pinMode(baseButton, INPUT_PULLUP);
  pinMode(conButton, INPUT_PULLUP);

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

  lcdMessage("BPM = " + String(pulseRate) + " PF = " + String(pulseFraction), "baseline = " + String(baseline));

  // turn on the motor
  analogWrite(motorPin, 0);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  int bufferLength = 125 - pulseFraction;
  digitalWrite(13, HIGH);  // turn on the built in led when the heartbeat begins

  for (int i = 0; i < bufferLength; i++){
    int currentFlow = flowVector[i];
    if (currentFlow < baseline) currentFlow = baseline;

    analogWrite(motorPin, currentFlow);

    if (i > 0) digitalWrite(13, LOW);
    Serial.println(currentFlow);

    delay(60 * 1000/(pulseRate*bufferLength));
  }
}
