/*
* GUI.h
*
* Created: 28-12-2013 17:02:17
* Author: CE-Designs
*/

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#ifndef __GUI_H__
#define __GUI_H__

#include "CharacterOLED.h"
#include "Sabre.h"
#include "RTClib.h"
#include "EEPROM_Anything.h"
#include "Helper.h"

#define INPUT_SETTINGS_COUNT 13
#define MAIN_SETTINGS_COUNT 6

// EEPROM LOCATIONS
#define EEPROM_MENU_SETTINGS 450
#define EEPROM_GUI_FIRST_RUN 460

// MAIN MENU SETTINGS MAX VALUES
#define MAX_TIME 60
#define MAX_ATTNU 99

class GUI : public CharacterOLED, public Sabre
{
	//variables
	public:

	CharacterOLED OLED;
	Sabre sabreDAC;
	
	uint8_t GUI_State;		// for holding the current GUI state and the corresponding GUI sub state
	uint8_t GUI_Substate;	// for holding the corresponding GUI sub state
	
	uint8_t SelectedInputSetting;
	uint8_t SelectedMenuSetting;
	
	uint8_t CursorPosition;	// holds the cursor position when changing a input name
	
	bool EditMode();
	
	bool display() { return !NoDisplay; }
	
	int SettingsCode() {return 8191; } // enables all settings
	
	enum MainMenuSetting
	{
		DisplayAutoOff, DisplayAutoOffTime, DefaultAttnu , ShowAutoClock, ShowAutoClockTime, AdjustTime, AdjustDate
	};
	
	enum GUI_states
	{
		HomeScreen, InputSettingsMenu, MainMenu
	};
	
	enum Dac_Setting
	{
		InputName, FirFilter, IIRBandwidth, NotchDelay, Quantizer, DpllBandwidth, DpllBw128, OverSamplingFilter, InputFormat, SerialDataMode, SpdifSource, BitMode, AutoDeemphasis
	};

	struct mainMenu_t
	{
		uint8_t displayAutoOff;
		uint8_t displayAutoOffTime;
		uint8_t showAutoClock;
		uint8_t showAutoClockTime;
		uint8_t defaultAttnu;
	}MainMenuSettings;
	
	protected:
	
	private:
	
	RTC_DS1307 rtc;
	
	enum HomeScreen_States
	{
		DefaultHS, NoVolumeNumbersHS, NoInputNumberHS
	};
	
	
	bool TimerEnabled;
	bool NoDisplay;
	
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Month;
	uint8_t Day;
	int Year;
	
	//functions
	public:
	GUI(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
	
	virtual void begin();
	void setSabreDac(Sabre *dac);
	
	void printLargeAttenuation(uint8_t Attenuation, uint8_t col);
	void printLargeMuteSymbol(uint8_t col);
	void printLargeInput(uint8_t selectedInput, uint8_t col);
	void printInputName(uint8_t col, uint8_t row);
	void printSampleRate(uint8_t col, uint8_t row);
	void printInputFormat(uint8_t col, uint8_t row);
	
	void printTitleBar(String title);
	void printInputNameSetting(uint8_t col, uint8_t row);
	void printFIRsetting(uint8_t col, uint8_t row);
	void printIIRsetting(uint8_t col, uint8_t row);
	void printNotchSetting(uint8_t col, uint8_t row);
	void printQuantizerSetting(uint8_t col, uint8_t row);
	void printDPLLbandwidthSetting(uint8_t col, uint8_t row);
	void printDPLLmultiplierSetting(uint8_t col, uint8_t row);
	void printOSFfilterSetting(uint8_t col, uint8_t row);
	void printSPDIFenableSetting(uint8_t col, uint8_t row);
	void printSerialDataModeSetting(uint8_t col, uint8_t row);
	void printSPDIFsourceSetting(uint8_t col, uint8_t row);
	void printBitmodeSetting(uint8_t col, uint8_t row);
	void printDeemphFilterSetting(uint8_t col, uint8_t row);
	
	void printHomeScreen(uint8_t selectedInput, uint8_t attenuation);
	void PrepareForMenuPrinting();
	void printInputSettingsMenu(uint8_t selectedInput);
	void printSelectedInputSettings(uint8_t value, int code);
	void printMainMenu();
	void printSelectedMainMenuSetting(uint8_t value);

	void printDisplayAutoOffSetting(uint8_t col, uint8_t row);
	void printDisplayAutoOffTimeSetting(uint8_t col, uint8_t row);
	void printShowAutoClockSetting(uint8_t col, uint8_t row);
	void printAutoClockTimeSetting(uint8_t col, uint8_t row);
	void printAdjustTimeSetting(uint8_t col, uint8_t row);
	void printAdjustDateSetting(uint8_t col, uint8_t row);
	void printDefaultAttnuSetting(uint8_t col, uint8_t row);
	
	void printPointer();
	void printEmptyRow(uint8_t row);
	void printLockSymbol(uint8_t col, uint8_t row);
	void printSelectedChar();
	void printNextChar();
	void PrintPreviousChar();
	
	void setAndPrintHour(uint8_t value);
	void printHour();
	void setAndPrintMinute(uint8_t value);
	void printMinute();
	
	void setAndPrintDay(uint8_t value);
	void printDay();
	void setAndPrintMonth(uint8_t value);
	void printMonth();
	void setAndPrintYear(uint8_t value);
	void printYear();
	
	void saveDateTime();
	
	void printEnabledSetting(bool enabled, uint8_t col, uint8_t row);
	
	void toggleDisplay();
	
	void setInputNameCursor(uint8_t value);
	void setTimeCursor(uint8_t value);
	void setDateCursor(uint8_t value);
	
	void stopTimer();
	void readMainMenuSettings();
	void writeMainMenuSettings();
	
	
	// Date & Time printing
	void printLargeTime();
	void printSmallTime(uint8_t col, uint8_t row);
	void printSmallDate(uint8_t col, uint8_t row);
	
	protected:
	private:
	
	void applyDefaultSettings();
	void startTimer();
	
	
	GUI( const GUI &c );
	GUI& operator=( const GUI &c );

}; //GUI

#endif //__GUI_H__


