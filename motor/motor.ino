/*
  This code is based on / adapts logic from the Pulsatile Pump project 
  by Herwald SE, Sze DY, Ennis DB, Vezeridis AM.
  Original License: CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
*/

const byte motorPin = 9;
byte baseline = 5;
byte pulseRate = 60;
byte pulseFraction = 25;

// data from haemod.uk/aortic
const byte flowVector[100] = {
  5,54,107,158,201,233,251,255,253,243,230,213,196,178,162,146,132,119,107,95,82,67,47,22,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  // set up the pins
  pinMode(motorPin, OUTPUT);
  pinMode(13, OUTPUT);

  // turn on the motor
  analogWrite(motorPin, 10);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, HIGH); // turn on the built in led when the heartbeat begins
  for (int i = 0; i < bufferLength; i++){
    int currentFlow = flowVector[i];
    if (currentFlow < baseline) currentFlow = baseline;

    analogWrite(motorPin, currentFlow);

    if (i > 0) digitalWrite(13, LOW);
    Serial.println(currentFlow);

    unsigned long interval = 60000UL / ((unsigned long)pulseRate * bufferLength);
    delay(interval);
  }
}
