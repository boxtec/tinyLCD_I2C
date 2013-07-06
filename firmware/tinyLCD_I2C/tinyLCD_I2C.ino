// $Id: tinyLCD_I2C.ino,v 1.13 2013-07-03 14:59:55 obiwan Exp $

#include <TinyWireS.h>
#include <LiquidCrystal.h>

// Debug Mode
#define DBGME

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
#define LCD_SETDIMENSION 0x9f
#define LCD_SETCURSOR 0x9e
#define LCD_SHOWFIRMWAREREVISION 0x9d

// flags for backlight control
#define LCD_BACKLIGHT 0x81
#define LCD_NOBACKLIGHT 0x80

// I2C address/buffersize
#define I2C_SLAVE_ADDRESS 0x50
#define TWI_RX_BUFFER_SIZE ( 32 )

#define CONTRAST_PIN 2

// Timeout in us after which we close the buffer and write it out
#define CHARBUF_TIMEOUT 150

volatile uint8_t i2c_regs[] =
{
    0xDE, 
    0xAD, 
    0xBE, 
    0xEF, 
};
// Tracks the current register pointer position
volatile byte reg_position;
// buffer index for lcd.print strings
unsigned int buf_ix = 0; 
// store time when the last char to print was received
unsigned long lastwrite = 0;
// char buffer for incoming strings
char bigbuffer[32];
// more data expected
volatile byte data_expected = 0;

// initialize the library with the numbers of the interface pins
// LiquidCrystal(rs, enable, d4, d5, d6, d7);
// Breadboard:
//LiquidCrystal lcd(0, 3, 7, 8, 9, 10);
// tinyLCD_I2C:
LiquidCrystal lcd(3, 1, 9, 8, 7, 5);

void setup() {
  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(receiveEvent);
  //TinyWireS.onRequest(requestEvent);
  analogWrite(CONTRAST_PIN, 10);
  lcd.begin(16,2);
  lcd.clear();
  //test_lcd();
}

void loop() {
  if ( buf_ix > 0 && micros() - lastwrite > CHARBUF_TIMEOUT ) {
    flush_buffer();
  }
  TinyWireS_stop_check();
}


// not used yet:
void requestEvent()
{  
    TinyWireS.send(i2c_regs[reg_position]);
    // Increment the reg position on each read, and loop back to zero
    reg_position = (reg_position+1) % sizeof(i2c_regs);
    lcd.setCursor(3, 1);
    lcd.print("=");
}

void receiveEvent(uint8_t howMany) {
  //static int buf_ix = 0;
  if (howMany < 1)
  {
      // Sanity-check
      return;
  }

  char cmd = TinyWireS.receive();
  // wait for a command
  if ( cmd == 0 ) {
    while (howMany < 1) {
      tws_delay(1);    
    }
    char rxbuffer = TinyWireS.receive();
    commandByte(rxbuffer);
  }
  /*
  // direct print
  else if ( cmd == 1 ) {
    //lcd.print(rxbuffer);
    // are we receiving a larger string ? Keep adding to buffer
    if ( buf_ix == 0 || micros() - lastwrite < CHARBUF_TIMEOUT ) {
      bigbuffer[buf_ix] = rxbuffer;
      buf_ix++;
    }
  }
  */
  else if ( cmd > 1 ) {
    lcd.print(cmd);
  }
}

void commandByte(char c) {
  uint8_t cols, rows, col, row;
  switch (c) {
    case LCD_CLEARDISPLAY:
      #ifdef DBGME
      lcd.print("Clear Display!");
      #endif
      lcd.clear();
      break;
    case LCD_RETURNHOME:
      #ifdef DBGME
      lcd.print("Return Home!");
      #endif
      lcd.home();
      break;
    case LCD_BACKLIGHT:
      #ifdef DBGME
      lcd.print("Backlight!");
      #endif
      break;
    case LCD_NOBACKLIGHT:
      #ifdef DBGME
      lcd.print("No Backlight!");
      #endif
      break;
    case LCD_SETDIMENSION:
      TinyWireS.receive();
      cols = TinyWireS.receive();
      TinyWireS.receive();
      rows = TinyWireS.receive();
      #ifdef DBGME
      lcd.print("Set Dim:");
      lcd.print(int(cols));
      lcd.print(" / ");
      lcd.print(int(rows));
      tws_delay(660);
      #endif
      lcd_begin(cols, rows);
      break;
    case LCD_SETCURSOR:
      if ( TinyWireS.receive() == 0 ) {
        col = TinyWireS.receive();
        if ( TinyWireS.receive() == 0 ) {
          row = TinyWireS.receive();
        }
      }
      #ifdef DBGME
      lcd.print("Set Cursor:");
      lcd.setCursor(0,1);
      lcd.print(int(col));
      lcd.print(" - ");
      lcd.print(int(row));
      tws_delay(660);
      #endif
      lcd.setCursor(col, row);
      break;
    case LCD_SHOWFIRMWAREREVISION:
      lcd_revision();
      break;
    default:
      #ifdef DBGME
      lcd.print("Unknown Com!");
      lcd.setCursor(1,1);
      lcd.print(int(c));
      #endif
      break;
  }
}

// start the display, helper for init()
void lcd_begin(uint8_t cols, uint8_t rows) {
  lcd.begin(cols,rows);
  lcd.home();
  lcd.clear();
}

// display revision
void lcd_revision() {
  lcd.clear();
  lcd.print("tinyLCD_I2C :");
  lcd.setCursor(0, 1);
  lcd.print("$Revision: 1.13 $");
}

// flush receive buffer and output to LCD
void flush_buffer() {
  lcd.print(bigbuffer);
  buf_ix = 0;
  memset(&bigbuffer[0], 0, sizeof(bigbuffer));
}

void test_lcd() {
  lcd.print("==T=E=S=T=======");
  delay(1200);
  lcd.setCursor(0,1);
  lcd.print("XXXXXXXXXXXXXXX");
  delay(1200);
  lcd.clear();
  lcd.print("XXXXXXXXXXXXXXXX");
  lcd.setCursor(0,1);
  lcd.print("================");
  delay(2000);
  lcd.clear();
  lcd.print("Uptime now (s):");
  while ( 1 ) {
    lcd.setCursor(0,1);
    lcd.print(millis()/1);
    lcd.print(" us");
  }  
}
