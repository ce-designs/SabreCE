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

#define SETTINGS_COUNT 13
#define NEXT 0x01
#define PREVIOUS 0x02



class GUI : public CharacterOLED, public Sabre
{
	//variables
	public:

	CharacterOLED OLED;
	
	uint8_t GUI_State;		// for holding the current GUI state and the corresponding GUI sub state
	uint8_t GUI_Substate;	// for holding the corresponding GUI sub state
	
	uint8_t SelectedInputSetting;
	uint8_t SelectedMenu;
	
	bool InputNameEditMode();	
	
	Sabre sabreDAC;
	
	
	
	protected:
		
	private:
			
	struct GUI_Settings
	{
		uint8_t homeScreen;
		uint8_t mainMenu;
		
	}guiConfig;
		
	enum GUI_states
	{
		HomeScreen, InputSettingsMenu, MainMenu
	};
	
	enum HomeScreen_States
	{
		DefaultHS, NoVolumeNumbersHS, NoInputNumberHS
	};
	
	enum MainMenu_States
	{
		PresetsMenu, DisplayMenu, DateTimeMenu
	};
	
	enum Dac_Setting
	{
		InputName, FirFilter, IIRBandwidth, NotchDelay, Quantizer, DpllBandwidth, DpllBw128, OverSamplingFilter, InputFormat, SerialDataMode, SpdifSource, BitMode, AutoDeemphasis
	};

	uint8_t CursorPosition;	// holds the cursor position when changing a input name
	bool TimerEnabled;
		
	//functions
	public:
	GUI(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
	
	void start();
	
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
	void printInputSettingsMenu(uint8_t selectedInput);

	void PrepareForMenuPrinting();

	void printSelectedInputSettings(uint8_t value);
	void printSelectedInputSettings(uint8_t value, int code);
	
	void printMainMenu();
	
	void printPointer();
	void printEmptyRow(uint8_t row);
	void printLockSymbol(uint8_t col, uint8_t row);
	
	void printSelectedChar();
	void printNextChar();
	void PrintPreviousChar();

	void SetCursorPosition(uint8_t value);
	
	void stopInputNameEditMode();	
		
	protected:
	private:
	
	void SetPointerValue(uint8_t value, uint8_t *pointervalue, uint8_t maxValue);
	
	
	GUI( const GUI &c );
	GUI& operator=( const GUI &c );

}; //GUI

#endif //__GUI_H__


