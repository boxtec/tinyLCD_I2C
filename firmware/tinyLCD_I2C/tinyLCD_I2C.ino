// $Id: tinyLCD_I2C.ino,v 1.13 2013-07-03 14:59:55 obiwan Exp $

#include <TinyWireS.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
// Debug Mode
//#define DBGME

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDIMENSION 0x9f
#define LCD_SETCURSOR 0x9e
#define LCD_SHOWFIRMWAREREVISION 0x9d
#define LCD_SETADDRESS 0x9c
#define LCD_SETCONTRAST 0x9b

// flags for backlight control
#define LCD_BACKLIGHT 0x81
#define LCD_NOBACKLIGHT 0x80
#define LCD_SETBACKLIGHT 0x82

// I2C default address
#define I2C_SLAVE_ADDRESS 0x50

#define CONTRAST_PIN 2
#define BACKLIGHT_PIN 3
#define SPI_SS 10
#define SPI_SS_PORT PINA
#define SPI_SS_MASK 1

#define EEBASE_ADDR 24

extern void usiTwiSlaveOvlHandler(); // from usiTwiSlave.c

// SPI RX buffer
#define USI_SPI_RX_BUFFER_SIZE  ( 32 )
#define USI_SPI_RX_BUFFER_MASK  ( USI_SPI_RX_BUFFER_SIZE - 1 )

#if ( USI_SPI_RX_BUFFER_SIZE & USI_SPI_RX_BUFFER_MASK )
#  error USI SPI RX buffer size is not a power of 2
#endif

volatile uint8_t spiRxHead = 0;
volatile uint8_t spiRxTail = 0;
volatile uint8_t spiRxBuf[USI_SPI_RX_BUFFER_SIZE];
volatile uint8_t spiRxEndBlock = 0;

// Timeout in us after which we close the buffer and write it out
#define CHARBUF_TIMEOUT 150

volatile byte interface_mode = 0; // I2C


// initialize the library with the numbers of the interface pins
// LiquidCrystal(rs, enable, d4, d5, d6, d7);
// Breadboard:
//LiquidCrystal lcd(0, 3, 7, 8, 9, 10);
// tinyLCD_I2C:
LiquidCrystal lcd(3, 1, 9, 8, 7, 5);
uint8_t slave_address;


uint8_t common_available() {
	if (interface_mode) {
		return (spiRxHead - spiRxTail) & USI_SPI_RX_BUFFER_MASK;
	} else {
		return TinyWireS.available();
	}
}


void setup() {
	slave_address = read_address();
	if (! slave_address) {
		slave_address = I2C_SLAVE_ADDRESS;
	}
	TinyWireS.begin(slave_address);
	TinyWireS.onReceive(receive_event);
	//TinyWireS.onRequest(requestEvent);
	
	analogWrite(CONTRAST_PIN, 10);
	analogWrite(BACKLIGHT_PIN, 255);
	lcd.begin(16,2);
	lcd_revision();
	delay(500);
	lcd.clear();
	// check SPI status
	pinMode(SPI_SS, INPUT);
	if (digitalRead(SPI_SS) == HIGH) {
		interface_mode = 1; // SPI
		init_spi();
	}
	// enable pin change interrupt
	PCMSK0 = _BV(PCINT0);
	GIMSK = _BV(PCIE0);
	//test_lcd();
}

void loop() {
	if (interface_mode) {
		if (spiRxEndBlock) {
			spiRxEndBlock = 0;
			uint8_t avail = common_available();
			if (avail) {
				receive_event(avail);
			}
		}
	} else {
		TinyWireS_stop_check();
	}
}

void init_spi() {
	pinMode(4, INPUT); // DI
	pinMode(6, INPUT_PULLUP); // USCK
	pinMode(5, OUTPUT); // DO
}


void spi_buffer_write(uint8_t data) {
	spiRxHead = ( spiRxHead + 1 ) & USI_SPI_RX_BUFFER_MASK;
	// check buffer size
	if (spiRxHead == spiRxTail) {
	// overrun
		spiRxHead = (spiRxHead + USI_SPI_RX_BUFFER_SIZE - 1) & USI_SPI_RX_BUFFER_MASK;
	} else {
		spiRxBuf[ spiRxHead ] = data;
	}
}

uint8_t spi_buffer_read() {
	// sanity check
	if ( spiRxHead == spiRxTail ) {
		return 255;
	}
	
	// calculate buffer index
	spiRxTail = ( spiRxTail + 1 ) & USI_SPI_RX_BUFFER_MASK;
	
	// return data from the buffer.
	return spiRxBuf[ spiRxTail ];
}

ISR(PCINT0_vect) {
	if (interface_mode) {
		if (SPI_SS_PORT & SPI_SS_MASK) {
			// finished reading bytes, set flag and stop interrupt
			USICR = 0; // reset SPI hardware, allowing DO to be used for LCD
			spiRxEndBlock = 1;
		} else {
			// SS activated, start receiving and activate interrupt
			USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USIOIE); // setup SPI mode 0, external clock, overflow interrupt enabled
		}
	} else {
		if (SPI_SS_PORT & SPI_SS_MASK) {
			interface_mode = 1;
			init_spi();
		}
	}
}

void receive_event(uint8_t howMany) {
	//static int buf_ix = 0;
	uint8_t cavail = common_available();
	lcd.clear();
	lcd.setCursor(0,1);
	lcd.print(spiRxTail, HEX);
	lcd.write(' ');
	lcd.print(spiRxHead - spiRxTail, HEX);
	lcd.write(' ');
	lcd.print((spiRxHead - spiRxTail) & USI_SPI_RX_BUFFER_MASK, HEX);
	lcd.write(' ');
	lcd.print(spiRxHead, HEX);
	lcd.write(' ');
	lcd.print(cavail, HEX);
	lcd.setCursor(0,0);
	uint8_t cur = spiRxTail;
	while (cur != spiRxHead) {
		cur = (cur + 1) & USI_SPI_RX_BUFFER_MASK;
		lcd.print(spiRxBuf[cur], HEX);
		lcd.write(' ');
	}
	while (common_available()) {
		char cmd = read_byte();
		// wait for a command
		if ( cmd == 0 && howMany > 1 ) {
			char rxbuffer = read_byte();
			//command_byte(rxbuffer, howMany - 2);
		} else if ( cmd > 1 ) {
			//lcd.print(cmd);
		}
	}
}

void command_byte(char c, byte bytesInBuffer) {
  uint8_t col, row, addr, val;
  if (c & 0xC0 == LCD_SETCGRAMADDR) {
    // construct character
    uint8_t cdata[8];
    uint8_t i;
    for (i = 0; i < 8; i++) {
      cdata[i] = read_byte();
    }
    lcd.createChar((c & 0x38) >> 3, cdata);
  } else if (c & 0xF8 == LCD_DISPLAYCONTROL || c & 0xF0 == LCD_CURSORSHIFT || c & 0xFC == LCD_ENTRYMODESET) {
    lcd.command(c);
  }
  switch (c) {
    case LCD_CLEARDISPLAY:
      lcd.clear();
      break;
    case LCD_RETURNHOME:
      lcd.home();
      break;
    case LCD_BACKLIGHT:
      analogWrite(BACKLIGHT_PIN, 255);
      break;
    case LCD_NOBACKLIGHT:
      analogWrite(BACKLIGHT_PIN, 0);
      break;
    case LCD_SETBACKLIGHT:
      val = read_byte();
      analogWrite(BACKLIGHT_PIN, val);
      break;
    case LCD_SETCONTRAST:
      val = read_byte();
      analogWrite(CONTRAST_PIN, val);
      break;
    case LCD_SETDIMENSION:
      col = read_byte();
      row = read_byte();
      lcd_begin(col, row);
      break;
    case LCD_SETCURSOR:
      col = read_byte();
      row = read_byte();
      lcd.setCursor(col, row);
      break;
    case LCD_SHOWFIRMWAREREVISION:
      lcd_revision();
      break;
    case LCD_SETADDRESS:
      addr = read_byte();
      if (addr && ! (addr & 0x80)) {
        write_address(addr);
      }
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

uint8_t read_byte() {
	if (interface_mode) {
		return spi_buffer_read();
	} else {
		return TinyWireS.receive();
	}
}

ISR(USI_OVF_vect) {
	if (interface_mode) {
		USISR = (1 << USIOIF); // clear interrupt flag
		spi_buffer_write(USIDR);
		//USIDR = 0x5A; // set recognizable pattern to simplify debugging using logic analyzer
	} else {
		usiTwiSlaveOvlHandler();
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
  lcd.print(F("tinyLCD_I2C :"));
  lcd.setCursor(0, 1);
  lcd.print(F("$Revision: 1.13 $"));
}


void test_lcd() {
  lcd.print(F("==T=E=S=T======="));
  delay(1200);
  lcd.setCursor(0,1);
  lcd.print(F("XXXXXXXXXXXXXXX"));
  delay(1200);
  lcd.clear();
  lcd.print(F("XXXXXXXXXXXXXXXX"));
  lcd.setCursor(0,1);
  lcd.print(F("================"));
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(F("Address: "));
  lcd.print(slave_address, HEX);
  delay(3000);
  lcd.clear();
  lcd.print(F("Uptime now (s):"));
  while ( 1 ) {
    lcd.setCursor(0,1);
    lcd.print(millis()/1);
    lcd.print(F(" ms"));
  }  
}

void write_address(uint8_t address) {
	EEPROM.write(EEBASE_ADDR, address);
	// write marker to know that we have a custom I2C address
	EEPROM.write(EEBASE_ADDR + 1, 't');
	EEPROM.write(EEBASE_ADDR + 2, 'L');
	EEPROM.write(EEBASE_ADDR + 3, 'C');
	EEPROM.write(EEBASE_ADDR + 4, 'D');
	EEPROM.write(EEBASE_ADDR + 5, 0x00); // no more configs for the moment
}

uint8_t read_address() {
	if (EEPROM.read(EEBASE_ADDR + 1) == 't' &&
	    EEPROM.read(EEBASE_ADDR + 2) == 'L' &&
	    EEPROM.read(EEBASE_ADDR + 3) == 'C' &&
	    EEPROM.read(EEBASE_ADDR + 4) == 'D') {
		return EEPROM.read(EEBASE_ADDR);
	}
	return 0;
}
