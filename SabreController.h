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

#ifndef __SABRECONTROLLER_H__
#define __SABRECONTROLLER_H__

//#define CHARACTER_OLED

#ifdef CHARACTER_OLED
#include "CharacterOLED.h"
#else
#include "OLEDFourBit.h"
#endif
#include "Sabre.h"
#include "RTClib.h"
#include "EEPROM_Anything.h"
#include "Helper.h"

#define INPUT_SETTINGS_COUNT 13
#define MAIN_SETTINGS_COUNT 6

#define INPUT_NAME_SIZE 8
#define NUMBER_OF_INPUTS 4
#define SR_LENGTH 8


// FIRST RUN VALUE

#define FIRST_RUN 0x02

//// EEPROM LOCATIONS FOR STORING DATA ///

// locations 0 - 400 are reserved for the input config!!

#define EEPROM_SELECTED_INPUT 400
#define EEPROM_DEF_ATTNU 401
#define EEPROM_SABRE_FIRST_RUN 402
#define EEPROM_MENU_SETTINGS 450
#define EEPROM_GUI_FIRST_RUN 460

// MAIN MENU SETTINGS MAX VALUES
#define MAX_TIME 60
#define MAX_ATTNU 99

class SabreController 
{
	//variables
	public:
	
#ifdef CHARACTER_OLED
	CharacterOLED OLED;
#else
	OLEDFourBit OLED;
#endif

	Sabre sabreDAC;
	
	uint8_t GUI_State;		// for holding the current GUI state and the corresponding GUI sub state
	uint8_t GUI_Substate;	// for holding the corresponding GUI sub state
	
	uint8_t SelectedInputSetting;
	uint8_t SelectedMenuSetting;
	
	uint8_t CursorPosition;	// holds the cursor position when changing a input name
	
	uint8_t SelectedInput;	// holds the current selected input
	
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


	#pragma region STRUCTS
	
	struct config_t	// Holds the configuration of each of the configured inputs
	{
		// NON DAC-REGISTER RELATED INPUT SETTINGS
		char INPUT_NAME[INPUT_NAME_SIZE];
		uint8_t INPUT_ENABLED;
		// DAC REGISTER SETTINGS OF THE INPUT
		uint8_t SPDIF_SOURCE;
		uint8_t FIR_FILTER;
		uint8_t IIR_BANDWIDTH;
		uint8_t QUANTIZER;
		uint8_t DIFFERENTIAL_MODE;
		uint8_t SPDIF_ENABLE;
		uint8_t DPLL_BANDWIDTH;
		uint8_t DPLL_BW_128X;
		uint8_t DPLL_BW_DEFAULTS;
		uint8_t NOTCH_DELAY;
		uint8_t SERIAL_DATA_MODE;
		uint8_t	BIT_MODE;
		uint8_t JITTER_REDUCTION;
		uint8_t DEEMPH_FILTER;
		uint8_t DE_EMPHASIS_SELECT;
		uint8_t OSF_FILTER;
		uint8_t AUTO_DEEMPH;
		// END OF DAC SETTINGS
	} Config[NUMBER_OF_INPUTS + 1];
	
	
	struct mainMenu_t
	{
		uint8_t displayAutoOff;
		uint8_t displayAutoOffTime;
		uint8_t showAutoClock;
		uint8_t showAutoClockTime;
		uint8_t defaultAttenuation;
	}MainMenuSettings;
	
	#pragma endregion STRUCTS
	
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
	SabreController(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
	
	virtual void begin(uint8_t mode, uint8_t f_clock);
	
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
	
	void writeInputConfiguration();
	void writeSelectedInput();
	void selectInput(uint8_t value);
	
	protected:
	private:
	
	void applyDefaultSettings();
	void startTimer();
	
	void resetInputNames();
			
	void readInputConfiguration();
	void applyInputConfiguration(uint8_t input);
	
	void writeInputConfiguration(uint8_t input);
	
	//void writeDefaultAttenuation();

	static bool firstRun(); 	// for checking if it is the first run of the controller

}; //GUI

#endif //__GUI_H__


