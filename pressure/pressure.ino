#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int pressurePin = A1;
const double sens = 1.0 / (5.0 * pow(10.0, -6.0) * 5.0 * 1000.0); 

int rawADC = 0;
float voltage = 0.0;
float pressure = 0.0; // pressure in mmHg
float V0 = 0.0;

const byte confButton = 37;

unsigned long startTime;
LiquidCrystal_I2C lcd(0x27, 16, 2);

bool buttonPressed(byte pin){
  if (digitalRead(pin) == LOW){
    delay(20);
    if (digitalRead(pin) == LOW){
      while (digitalRead(pin) == LOW){
        // Wait for release
      }
      delay(20);
      return true;
    }
  }
  return false;
}

void lcdMessage(const String& msg1, const String& msg2 = ""){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg1);
  lcd.setCursor(0, 1);
  lcd.print(msg2);
}

// Formula logic: P = (V - V0) * Sensitivity
float calcPressure(float inputVoltage) {
  return (inputVoltage - V0) * sens;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  delay(1000);

  pinMode(confButton, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcdMessage("Disconnect", "Transducer");
  delay(2000);
  lcdMessage("Press conf.", "Button");

  while (!buttonPressed(confButton)) {
    delay(10); 
  }
  
  lcdMessage("Setting V0", "Please wait...");
  delay(500);
  
  int sampleADC = analogRead(pressurePin);
  V0 = sampleADC * (5.0 / 1023.0); 
  
  lcdMessage("V0 set to:", String(V0) + " V");
  delay(1500);

  startTime = millis();
}

void loop() {
  unsigned long currentTime = millis() - startTime;

  rawADC = analogRead(pressurePin);
  voltage = rawADC * (5.0 / 1023.0);
  pressure = calcPressure(voltage); 

  Serial.print(currentTime);
  Serial.print(",");
  Serial.print(pressure);
  Serial.println();

  delay(10);
}