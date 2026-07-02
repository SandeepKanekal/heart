const byte readPin = A0;

void setup() {
    Serial.begin(115200);
}

void loop() {
    int raw = analogRead(readPin);
    Serial.println(raw);
    delay(10);
}
