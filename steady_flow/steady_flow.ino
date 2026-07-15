const byte motorPin = 4;

void setup() {
  pinMode(motorPin, OUTPUT);
}

void loop() {
    analogWrite(motorPin, 150);
}
