const byte readPin = A0;

int rawVals[100];

double calcMean(int rawVals[100]) {
    int sum = 0;
    for (int i = 0; i<100; i++) {
        sum += rawVals[i];
    }
    return sum / 100.0;
}

void setup() {
    Serial.begin(115200);

    for (int i = 0; i < 100; i++) {
        rawVals[i] = analogRead(readPin);
        delay(100);
    }

    Serial.println(calcMean(rawVals));
}

void loop() {

}
