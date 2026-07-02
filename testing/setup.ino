const byte motorPin = 2;

void setup() {
    pinMode(motorPin, OUTPUT);
}

void loop() {
    analogWrite(motorPin, 150);
}
