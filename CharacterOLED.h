// functions for printing large characters added by CE-designs - 2014/15/05

#ifndef CharacterOLED_h
#define CharacterOLED_h

#include <inttypes.h>
#include "Print.h"

// OLED hardware versions
#define OLED_V1 0x01
#define OLED_V2 0x02

// commands
#define LCD_CLEARDISPLAY	0x01
#define LCD_RETURNHOME		0x02
#define LCD_ENTRYMODESET	0x04
#define LCD_DISPLAYCONTROL	0x08
#define LCD_CURSORSHIFT		0x10
#define LCD_FUNCTIONSET		0x28
#define LCD_SETCGRAMADDR	0x40
#define LCD_SETDDRAMADDR	0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT	0x00
#define LCD_ENTRYLEFT	0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON	0x04
#define LCD_DISPLAYOFF	0x00
#define LCD_CURSORON	0x02
#define LCD_CURSOROFF	0x00
#define LCD_BLINKON		0x01
#define LCD_BLINKOFF	0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE	0x00
#define LCD_MOVERIGHT	0x04
#define LCD_MOVELEFT	0x00

// flags for function set
#define LCD_8BITMODE    0x10
#define LCD_4BITMODE    0x00
#define LCD_JAPANESE    0x00
#define LCD_EUROPEAN_I  0x01
#define LCD_RUSSIAN     0x02
#define LCD_EUROPEAN_II 0x03

// flags for custom characters
#define UPWARDS_ARROW	0x00
#define DOWNWARDS_ARROW 0x01
#define LOCK_SYMBOL		0x02


class CharacterOLED : public Print {
public:
  
  CharacterOLED();
  
  CharacterOLED(uint8_t ver, uint8_t rs, uint8_t rw, uint8_t enable,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  
  void init(uint8_t ver, uint8_t rs, uint8_t rw, uint8_t enable,
	    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
    
  void begin(uint8_t cols, uint8_t rows);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
  virtual size_t write(uint8_t);
  void command(uint8_t);
  char readChar(void);
  void printLargeNumber(uint8_t number, uint8_t col, uint8_t row);
  
  void defineCustomChar0();
  void defineCustomChar1();
  void defineCustomChar2();
  void defineCustomChar3();
  void defineCustomChar4();
  void defineCustomChar5();
  void defineCustomChar6();
  void defineCustomChar7();
  
  void defineUpwardsArrowChar(uint8_t location);
  void defineDownwardsArrowChar(uint8_t location);
  void defineLockedChar(uint8_t location);
  
private:
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void pulseEnable();
  void waitForReady();
  
  void defineCustomCharacters();

  uint8_t _oled_ver;	// OLED_V1 = older, OLED_V2 = newer hardware version.
  uint8_t _rs_pin;		// LOW: command.  HIGH: character.
  uint8_t _rw_pin;		// LOW: write to LCD.  HIGH: read from LCD.
  uint8_t _enable_pin;	// activated by a HIGH pulse.
  uint8_t _busy_pin;	// HIGH means not ready for next command
  uint8_t _data_pins[4];

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _initialized;
  uint8_t _currline;
  uint8_t _numlines;
};

#endif
