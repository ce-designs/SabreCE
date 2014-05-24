// Derived from LiquidCrystal by David Mellis
// With portions adapted from Elco Jacobs CharacterOLED
// Modified for 4-bit operation of the Winstar 16x2 Character OLED
// By W. Earl for Adafruit - 6/30/12
// Initialization sequence fixed by Technobly - 9/22/2013
// functions for printing large characters added by CE-designs - 2014/15/05

#include "CharacterOLED.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// On power up, the display is initialized as:
// 1. Display clear
// 2. Function set:
//    DL="1": 8-bit interface data
//    N="0": 1-line display
//    F="0": 5 x 8 dot character font
// 3. Power turn off
//    PWR=�0�
// 4. Display on/off control: D="0": Display off C="0": Cursor off B="0": Blinking off
// 5. Entry mode set
//    I/D="1": Increment by 1
//    S="0": No shift
// 6. Cursor/Display shift/Mode / Pwr
//    S/C=�0�, R/L=�1�: Shifts cursor position to the right
//    G/C=�0�: Character mode
//    Pwr=�1�: Internal DCDC power on
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).


CharacterOLED::CharacterOLED()
{
	
}

CharacterOLED::CharacterOLED(uint8_t ver, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init(ver, rs, rw, enable, d4, d5, d6, d7);	
}

void CharacterOLED::init(uint8_t ver, uint8_t rs, uint8_t rw, uint8_t enable,
			 uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	_oled_ver = ver;
	if(_oled_ver != OLED_V1 && _oled_ver != OLED_V2) 
	{
		_oled_ver = OLED_V2; // if error, default to newer version
	}
	_rs_pin = rs;
	_rw_pin = rw;
	_enable_pin = enable;
  
	_data_pins[0] = d4;
	_data_pins[1] = d5;
	_data_pins[2] = d6;
	_data_pins[3] = _busy_pin = d7;

	pinMode(_rs_pin, OUTPUT);
	pinMode(_rw_pin, OUTPUT);
	pinMode(_enable_pin, OUTPUT);
  
	_displayfunction = LCD_FUNCTIONSET | LCD_4BITMODE;
   
	begin(16, 2);  
}

void CharacterOLED::begin(uint8_t cols, uint8_t lines) 
{
	_numlines = lines;
	_currline = 0;
  
	pinMode(_rs_pin, OUTPUT);
	pinMode(_rw_pin, OUTPUT);
	pinMode(_enable_pin, OUTPUT);
  
	digitalWrite(_rs_pin, LOW);
	digitalWrite(_enable_pin, LOW);
	digitalWrite(_rw_pin, LOW);
  
	delayMicroseconds(50000); // give it some time to power up
  
	// Now we pull both RS and R/W low to begin commands
  
	for (int i = 0; i < 4; i++) 
	{
		pinMode(_data_pins[i], OUTPUT);
		digitalWrite(_data_pins[i], LOW);
	}

	// Initialization sequence is not quite as documented by Winstar.
	// Documented sequence only works on initial power-up.  
	// An additional step of putting back into 8-bit mode first is 
	// required to handle a warm-restart.
	//
	// In the data sheet, the timing specs are all zeros(!).  These have been tested to 
	// reliably handle both warm & cold starts.

	// 4-Bit initialization sequence from Technobly
	write4bits(0x03); // Put back into 8-bit mode
	delayMicroseconds(5000);
	if(_oled_ver == OLED_V2) {  // only run extra command for newer displays
	write4bits(0x08);
	delayMicroseconds(5000);
	}
	write4bits(0x02); // Put into 4-bit mode
	delayMicroseconds(5000);
	write4bits(0x02);
	delayMicroseconds(5000);
	write4bits(0x08);
	delayMicroseconds(5000);
  
	command(0x08);	// Turn Off
	delayMicroseconds(5000);
	command(0x01);	// Clear Display
	delayMicroseconds(5000);
	command(0x06);	// Set Entry Mode
	delayMicroseconds(5000);
	command(0x02);	// Home Cursor
	delayMicroseconds(5000);
	command(0x0C);	// Turn On - enable cursor & blink
	delayMicroseconds(5000);
	
	defineCustomCharacters(); // write custom characters to OLED
}

/********** high level commands, for the user! */
void CharacterOLED::clear()
{
	command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	//  delayMicroseconds(2000);  // this command takes a long time!
}

void CharacterOLED::home()
	{
	command(LCD_RETURNHOME);  // set cursor position to zero
	//  delayMicroseconds(2000);  // this command takes a long time!
}

void CharacterOLED::setCursor(uint8_t col, uint8_t row)
{
	uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row >= _numlines ) 
	{
		row = 0;  //write to first line if out off bounds
	}
  
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void CharacterOLED::noDisplay() 
{
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CharacterOLED::display() 
{
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void CharacterOLED::noCursor() 
{
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CharacterOLED::cursor() 
{
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void CharacterOLED::noBlink() 
{
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CharacterOLED::blink() 
{
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void CharacterOLED::scrollDisplayLeft(void) 
{
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void CharacterOLED::scrollDisplayRight(void) 
{
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void CharacterOLED::leftToRight(void) 
{
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void CharacterOLED::rightToLeft(void) 
{
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void CharacterOLED::autoscroll(void) 
{
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void CharacterOLED::noAutoscroll(void) 
{
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void CharacterOLED::createChar(uint8_t location, uint8_t charmap[]) 
{
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) 
	{
		write(charmap[i]);
	}
}

/*********** mid level commands, for sending data/cmds */

inline void CharacterOLED::command(uint8_t value) 
{
	send(value, LOW);
	waitForReady();
}

inline size_t CharacterOLED::write(uint8_t value) 
{
	send(value, HIGH);
	waitForReady();
}

/************ low level data pushing commands **********/

// write either command or data
void CharacterOLED::send(uint8_t value, uint8_t mode) 
{
	digitalWrite(_rs_pin, mode);
	pinMode(_rw_pin, OUTPUT);
	digitalWrite(_rw_pin, LOW);
	
	write4bits(value>>4);
	write4bits(value);
}

void CharacterOLED::pulseEnable(void) 
{
	digitalWrite(_enable_pin, HIGH);
	delayMicroseconds(50);    // Timing Spec?
	digitalWrite(_enable_pin, LOW);
}

void CharacterOLED::write4bits(uint8_t value) 
{
	for (int i = 0; i < 4; i++) 
	{
		pinMode(_data_pins[i], OUTPUT);
		digitalWrite(_data_pins[i], (value >> i) & 0x01);
	}
	delayMicroseconds(50); // Timing spec?
	pulseEnable();
}

// Poll the busy bit until it goes LOW
void CharacterOLED::waitForReady(void) 
{
	unsigned char busy = 1;
	pinMode(_busy_pin, INPUT);
	digitalWrite(_rs_pin, LOW);	
	digitalWrite(_rw_pin, HIGH);      
	do
	{
		digitalWrite(_enable_pin, LOW);
		digitalWrite(_enable_pin, HIGH);

		delayMicroseconds(10);
		busy = digitalRead(_busy_pin);
		digitalWrite(_enable_pin, LOW);
  	
		pulseEnable();		// get remaining 4 bits, which are not used.
	}
	while(busy);
  
	pinMode(_busy_pin, OUTPUT);
	digitalWrite(_rw_pin, LOW);
}

char CharacterOLED::readChar(void){
	char value=0x00;
	for (int i = 0; i < 4; i++) {
		pinMode(_data_pins[i], INPUT);
	}
	digitalWrite(_rs_pin, HIGH);
	digitalWrite(_rw_pin, HIGH);
	pulseEnable();
	delayMicroseconds(600);
	for (int i = 0; i < 4; i++) {
		value = value | (digitalRead(_data_pins[i]) << (i+4));
	}
	pulseEnable();
	delayMicroseconds(600);
	for (int i = 0; i < 4; i++) {
		value = value | (digitalRead(_data_pins[i]) << (i));
	}
	return value;
}


/************ private custom character functions **********/

char bn1[]={0xFF,2,1, 2,1,0x20, 2,2,1, 2,2,0xFF, 0xFF,0x20,0xFF, 0xFF,2,2, 0,2,2, 2,2,0xFF, 0xFF,2,1, 0xFF,2,1};
char bn2[]={0xFF,0x20,0xFF, 0x20,0xFF,0x20 ,0,6,5, 0x20,2,0xFF, 5,6,0xFF, 2,2,1, 0xFF,6,7, 0x20,0,5, 0xFF,6,0xFF, 5,6,0xFF};
char bn3[]={4,3,0xFF, 3,0xFF,3, 0xFF,3,3, 3,3,0xFF, 0x20,0x20,0xFF, 3,3,0xFF, 4,3,0xFF, 0x20,0xFF,0x20, 4,3,0xFF, 3,3,0xFF};

void CharacterOLED::defineCustomCharacters(void)
{
	defineCustomChar0();
	defineCustomChar1();
	defineCustomChar2();
	defineCustomChar3();
	defineCustomChar4();
	defineCustomChar5();
	defineCustomChar6();
	defineCustomChar7();
}

void CharacterOLED::defineCustomChar0(void)
{
	// Custom Character 0
	byte cc0[8] = { B00000, B00111,	B01111, B11111,
	B11111,	B11111,	B11111,	B11111 };
	createChar(0, cc0);
}
void CharacterOLED::defineCustomChar1(void)
{
	// Custom Character 1
	byte cc1[8] = {	B11100,	B11110,	B11111,	B11111,
	B11111,	B11111,	B11111,	B11111 };
	createChar(1, cc1);
}

void CharacterOLED::defineCustomChar2(void)
{
	// Custom Character 2
	byte cc2[8] = { B11111,	B11111,	B11111,	B11111,
	B11111, B00000,	B00000,	B00000 };
	createChar(2, cc2);
}

void CharacterOLED::defineCustomChar3(void)
{
	// Custom Character 3
	byte cc3[8] = { B00000,	B00000,	B00000,	B11111,
	B11111,	B11111,	B11111,	B11111 };
	createChar(3, cc3);
}

void CharacterOLED::defineCustomChar4(void)
{
	// Custom Character 4
	byte cc4[8] = {	B11111,	B11111,	B11111,	B11111,
	B11111,	B11111,	B01111,	B00111 };
	createChar(4, cc4);
}

void CharacterOLED::defineCustomChar5(void)
{
	// Custom Character 5
	byte cc5[8] = { B11111,	B11111,	B11111,	B11111,
	B11111,	B11111,	B00000,	B00000 };
	createChar(5, cc5);
}

void CharacterOLED::defineCustomChar6(void)
{
	// Custom Character 6
	byte cc6[8] = { B00000,	B11111,	B11111,	B11111,
	B11111,	B11111,	B00000,	B00000 };
	createChar(6, cc6);
}

void CharacterOLED::defineCustomChar7(void)
{
	// Custom Character 7
	byte cc7[8] = { B00000,	B11100,	B11110,	B11111,
	B11111,	B11111,	B11111,	B11111 };
	createChar(7, cc7);
}

void CharacterOLED::defineUpwardsArrowChar(uint8_t location)
{
	byte cc[8] = { B00100,	B01110,	B10101,	B00100,
	B00100,	B00100,	B00100,	B00000 };
	createChar(location, cc);
}

void CharacterOLED::defineDownwardsArrowChar(uint8_t location)
{
	byte cc[8] = { B00000,	B00100,	B00100,	B00100,
	B00100,	B10101,	B01110,	B00100 };
	createChar(location, cc);
}

void CharacterOLED::defineLockedChar(uint8_t location)
{
	byte cc[8] = { 14,	17,	17,	31,
	27,	27,	31,	0 };
	createChar(location, cc);
}

void CharacterOLED::printLargeNumber(uint8_t number, uint8_t col, uint8_t row)
{
	if (number<10)
	{
		setCursor(col, row);		// Printing line 1 of the number
		write(bn1[(number%10)*3]);
		write(bn1[(number%10)*3+1]);
		write(bn1[(number%10)*3+2]);
		setCursor(col, row + 1);		// Printing line 2 of the number
		write(bn2[(number%10)*3]);
		write(bn2[(number%10)*3+1]);
		write(bn2[(number%10)*3+2]);
		setCursor(col, row + 2);		// Printing line 3 of the number
		write(bn3[(number%10)*3]);
		write(bn3[(number%10)*3+1]);
		write(bn3[(number%10)*3+2]);
	}
	if (number>9)
	{
		setCursor(col, row);		// Printing line 1 of the number
		write(bn1[(number/10)*3]);
		write(bn1[(number/10)*3+1]);
		write(bn1[(number/10)*3+2]);
		write(0x20); // Blank
		write(bn1[(number%10)*3]);
		write(bn1[(number%10)*3+1]);
		write(bn1[(number%10)*3+2]);

		setCursor(col, row + 1);		// Printing line 2 of the two-digit number
		write(bn2[(number/10)*3]);
		write(bn2[(number/10)*3+1]);
		write(bn2[(number/10)*3+2]);
		write(0x20); // Blank
		write(bn2[(number%10)*3]);
		write(bn2[(number%10)*3+1]);
		write(bn2[(number%10)*3+2]);

		setCursor(col, row + 2);		// Printing line 3 of the two-digit number
		write(bn3[(number/10)*3]);
		write(bn3[(number/10)*3+1]);
		write(bn3[(number/10)*3+2]);
		write(0x20); // Blank
		write(bn3[(number%10)*3]);
		write(bn3[(number%10)*3+1]);
		write(bn3[(number%10)*3+2]);
	}
}
