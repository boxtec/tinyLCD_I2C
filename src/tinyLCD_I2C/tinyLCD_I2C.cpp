// $Id: tinyLCD_I2C.cpp,v 1.12 2013-07-03 14:52:53 obiwan Exp $
//

#include "tinyLCD_I2C.h"
#include <inttypes.h>
#include "Wire.h"
#if ARDUINO < 100
  #include <WProgram.h>
#else
  #include <Arduino.h>
#endif

tinyLCD_I2C::tinyLCD_I2C(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows)
{
  _Addr = lcd_addr;
  _cols = lcd_cols;
  _rows = lcd_rows;
}

void tinyLCD_I2C::init(){
	init_priv();
}

void tinyLCD_I2C::init_priv()
{
	Wire.begin();
	begin(_cols, _rows);  
}

void tinyLCD_I2C::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  displayDimension(cols, lines);
  clear();
  home();
}

void tinyLCD_I2C::displayDimension(uint8_t cols, uint8_t lines) {
  uint8_t data[2];
  data[0] = cols;
  data[1] = lines;
  send(LCD_SETDIMENSION, 0, 2, data);
}

// clear display
void tinyLCD_I2C::clear(){
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
}

// jump to 0,0
void tinyLCD_I2C::home(){
	command(LCD_RETURNHOME);  // set cursor position to zero
}

// set cursor position
void tinyLCD_I2C::setCursor(uint8_t col, uint8_t row) {
  uint8_t data[2];
  data[0] = col;
  data[1] = row;
  send(LCD_SETCURSOR, 0, 2, data);
}


// Turn the display on/off (quickly)
void tinyLCD_I2C::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void tinyLCD_I2C::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void tinyLCD_I2C::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void tinyLCD_I2C::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void tinyLCD_I2C::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void tinyLCD_I2C::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void tinyLCD_I2C::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void tinyLCD_I2C::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void tinyLCD_I2C::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void tinyLCD_I2C::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void tinyLCD_I2C::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void tinyLCD_I2C::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void tinyLCD_I2C::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	send(LCD_SETCGRAMADDR | (location << 3), 0, 8, charmap);
}

// Turn the (optional) backlight off/on
void tinyLCD_I2C::noBacklight(void) {
	command(LCD_NOBACKLIGHT);
}

void tinyLCD_I2C::backlight(void) {
	command(LCD_BACKLIGHT);
}

void tinyLCD_I2C::setBacklight(uint8_t value) {
	send(LCD_SETBACKLIGHT, 0, 1, &value);
}

void tinyLCD_I2C::setContrast(uint8_t new_val) {
	send(LCD_SETCONTRAST, 0, 1, &new_val);
}

void tinyLCD_I2C::changeI2CAddress(uint8_t new_address) {
	send(LCD_SETADDRESS, 0, 1, &new_address);
}

/*********** mid level commands, for sending data/cmds */

inline void tinyLCD_I2C::command(uint8_t value) {
	send(value, 0);
}

inline size_t tinyLCD_I2C::write(uint8_t value) {
	send(value, Rs);
	return 0;
}



/************ low level data pushing commands **********/

// write either command or data
void tinyLCD_I2C::send(uint8_t value, uint8_t mode, uint8_t len, uint8_t *data) {
	int ret = 0;
	uint8_t repeat = 4;
	do {
		Wire.beginTransmission(_Addr);
		if ( mode == 0 ) {
			Wire.write(mode);
		}
		Wire.write((int)(value));
		uint8_t i = 0;
		while (len--) {
			Wire.write(data[i++]);
		}
		ret = Wire.endTransmission();
	} while (ret && repeat--);
}



// Alias functions

void tinyLCD_I2C::cursor_on(){
	cursor();
}

void tinyLCD_I2C::cursor_off(){
	noCursor();
}

void tinyLCD_I2C::blink_on(){
	blink();
}

void tinyLCD_I2C::blink_off(){
	noBlink();
}

void tinyLCD_I2C::load_custom_character(uint8_t char_num, uint8_t *rows){
		createChar(char_num, rows);
}


void tinyLCD_I2C::printstr(const char c[]){
	//This function is not identical to the function used for "real" I2C displays
	//it's here so the user sketch doesn't have to be changed 
	print(c);
}

// display firmware revision
void tinyLCD_I2C::showFirmware(){
	command(LCD_SHOWFIRMWAREREVISION);// clear display, set cursor position to zero
}

// unsupported API functions
void tinyLCD_I2C::off(){}
void tinyLCD_I2C::on(){}
void tinyLCD_I2C::setDelay (int cmdDelay,int charDelay) {}
uint8_t tinyLCD_I2C::status(){return 0;}
uint8_t tinyLCD_I2C::keypad (){return 0;}
uint8_t tinyLCD_I2C::init_bargraph(uint8_t graphtype){return 0;}
void tinyLCD_I2C::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end){}
void tinyLCD_I2C::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_row_end){}

