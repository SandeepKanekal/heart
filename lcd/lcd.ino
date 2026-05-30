#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const byte BUTTON_A = 31;
const byte BUTTON_B = 33;
const byte BUTTON_C = 35;

// Initialize with a temporary address; we will re-initialize dynamically in setup()
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Function to automatically find the LCD's I2C address
byte scanI2CAddress() {
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found LCD at address: 0x");
      Serial.println(address, HEX);
      return address;
    }
  }
  Serial.println("No I2C device found! Defaulting to 0x27");
  return 0x27;
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(1000); // Give serial monitor time to connect

  // 1. Scan and find the address automatically
  byte autoAddress = scanI2CAddress();
  
  // 2. Re-assign the detected address to the LCD object
  lcd = LiquidCrystal_I2C(autoAddress, 16, 2);
  
  // 3. Initialize the screen as normal
  lcd.begin(16, 2);      
  lcd.backlight();
  
  lcd.setCursor(0, 0);   
  lcd.print("Auto-Address Ok!");
  
  lcd.setCursor(0, 1);   
  lcd.print("Address: 0x" + String(autoAddress, HEX));

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("press button");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  while(digitalRead(BUTTON_B) == HIGH){
    Serial.println("test");
    // do nothing
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Button pressed!");
  lcd.setCursor(0, 1);
  lcd.print("Starting!");

  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Done!");
}

void loop() {
  
}
