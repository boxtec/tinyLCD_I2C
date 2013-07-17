#include <Wire.h>
#include <tinyLCD_I2C.h>

tinyLCD_I2C lcd(0x50,16,2);
float vdd;

void setup()
{
  lcd.init();                      // initialize the lcd 
  //lcd.backlight();        // not implemented yet
  delay(1);
  lcd.print("HELLO - WORLD!");
  delay(1200);
  lcd.setCursor(0,1);
  lcd.print("B-O-X-T-E-C-1-3");
  delay(1200);
  lcd.showFirmware();
  delay(1500);
  lcd.clear();
  lcd.print("Backlight");
  for (int i = 255; i >= 0; i--) {
    lcd.setBacklight(i);
    delay(10);
  }
  for (int i = 0; i < 256; i++) {
    lcd.setBacklight(i);
    delay(10);
  }
  delay(1000);
  lcd.clear();
  lcd.print("Contrast");
  for (int i = 0; i < 256; i++) {
    lcd.setContrast(i);
    delay(10);
  }
  for (int i = 255; i >= 0; i--) {
    lcd.setContrast(i);
    delay(10);
  }
  lcd.setContrast(10);
  delay(1000);
  lcd.clear();
  lcd.print("Uptime now (us):");
}

void loop() {
  lcd.setCursor(0,1);
  lcd.print(millis()/1);
  lcd.print(" ms");
  delay(10);
}
