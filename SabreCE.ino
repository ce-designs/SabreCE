/*
* SabreMaster
*
* Created: 19-12-2013 23:05:30
* Author: CE-Designs
*/

#pragma region #DEFINE_DIRECTIVES

#define CHARACTER_OLED

#define INCREASE 0x00
#define DECREASE 0x01

#pragma endregion #DEFINE_DIRECTIVES

#pragma region HEADER_FILE_INCLUSIONS

#include "Sabre.h"
#include "GUI.h"
#include "Wire.h"
#include "EEPROM.h"
#include "IRremote.h"
#ifdef CHARACTER_OLED
#include "CharacterOLED.h"
#else
#include "OLEDFourBit.h"
#endif
#include "RTClib.h"
#include "EEPROM_Anything.h"
#include "Helper.h"

#pragma endregion HEADER_FILE_INCLUSIONS

#pragma region CONSTANTS

const byte remoteDelay = 140;					// Delay in milliseconds after each key press of the remote
const unsigned int CenterKeyDuration = 1000;	// Time in milliseconds that the center key needs to be pressed before it invokes a method
const unsigned int StatusUpdateInterval = 1000;	// the interval between each status update
const byte LargeAttnuStartPos = 13;				// the start position for printing the large attenuation
const unsigned long DebounceTime = 50;			// Time for software debounce
const byte readDelay = 70;						// Delay in milliseconds for reading the volume and channel buttons
const byte buttonDelay = 140;					// Delay in milliseconds after each button press

const int buttonChUp = 0;		// pin 0 connects to the channel up button
const int buttonChDown = 1;		// pin 1 connects to the channel down button
const int buttonMute = 2;		// pin 2 connects to the mute button
const int buttonVolUp = 3;		// pin 3 connects to the volume up button
const int buttonVolDown = 4;	// pin 4 connects to the volume down button

#pragma endregion CONSTANTS

#pragma region GLOBAL_VARIABLES

Sabre dac;							// dac
GUI gui(25, 26, 27, 28, 29, 30, 31);// define the pins for the OLED
IRrecv	irrecv(10);					// set the IR receiver pin

decode_results results;				// object for storing the IR decode results

unsigned long IRcode = 0;			// for storing the current IR code
unsigned long lastKey = 0;			// for remembering the last IR Remote key
unsigned long StatusMillis = 0;		// for recording the time of the last status update
unsigned long DebounceMillis = 0;	// for recording the time of the last button press
unsigned long AutoMillis = 0;		// for recording the time of last auto off comparison

unsigned int CenterKeyCount = 0;	// for counting the successive received key codes of the center key

bool Toggle;						// used by timer2, see TIMER_METHODS

#pragma endregion GLOBAL_VARIABLES

#pragma region SETUP

void initializeButtonPins()
{
	pinMode(buttonChDown, INPUT);
	pinMode(buttonChUp, INPUT);
	pinMode(buttonMute, INPUT);
	pinMode(buttonVolDown, INPUT);
	pinMode(buttonVolUp, INPUT);
	
	digitalWrite(buttonChDown, HIGH);
	digitalWrite(buttonChUp, HIGH);
	digitalWrite(buttonMute, HIGH);
	digitalWrite(buttonVolDown, HIGH);
	digitalWrite(buttonVolUp, HIGH);
}

void setup()
{
	//Serial.begin(9600);		// for debugging
	
	Wire.begin();			// join the I2C bus
	
	dac.begin(false, 100);	// true = dual mono mode | false = stereo mode, Clock frequency (MHz)
	
	irrecv.enableIRIn();		// Start the receiver
	
	initializeButtonPins();		// initialize the pins for the buttons
	
	RTC_DS1307::begin();
	
	gui.begin();			// Start the user interface + initialize the display
	gui.setSabreDac(&dac);
	
	dac.setVolume(gui.MainMenuSettings.defaultAttnu);
	
	gui.printHomeScreen(dac.SelectedInput, dac.Attenuation);
	
	DebounceMillis = millis();
}

#pragma endregion SETUP

#pragma region MAIN_LOOP

// Main Loop
void loop()
{
	// IR routines
	if (irrecv.decode(&results)) 
	{
		setIRcode();
		irrecv.resume(); // Receive the next value
	}
	
	// If a remote control key is pressed than let the GUI class
	// decide on which function to call
	if (IRcode != 0)
	{
		switch (IRcode)
		{
			case KEY_UP:
			lastKey = KEY_UP;
			handleKeyUp();
			break;
			case KEY_DOWN:
			lastKey = KEY_DOWN;
			handleKeyDown();
			break;
			case KEY_LEFT:
			lastKey = KEY_LEFT;
			handleKeyLeft();
			break;
			case KEY_RIGHT:
			lastKey = KEY_RIGHT;
			handleKeyRight();			
			break;
			case KEY_CENTER:
			lastKey = KEY_CENTER;
			handleKeyCenter();			
			break;
			lastKey = KEY_MENU;
			case KEY_MENU:
			handleKeyMenu();			
			break;
			case KEY_PLAY_PAUSE:
			lastKey = KEY_PLAY_PAUSE;
			handlePlayPause();			
			break;
			default:
			lastKey = 0;	// probably received a code from another remote control, so reset the last keyCode value to prevent any unwanted operation
			break;
		}
		delay(remoteDelay);
		IRcode = 0;
	}
	
	// read the buttons
	if (millis() - DebounceMillis >= DebounceTime)
	{
		byte b_down_value = digitalRead(buttonVolDown);
		byte b_up_value = digitalRead(buttonVolUp);
		if (b_down_value == LOW || b_up_value == LOW)
		{
			delay(readDelay);	// wait a bit to give the user a chance to press both volume buttons before the controller reacts
			b_down_value = digitalRead(buttonVolDown);	// read again
			b_up_value = digitalRead(buttonVolUp);	// read again
			if (b_down_value == LOW && b_up_value == LOW)
			{
				handleKeyCenter();		// user pressed both keys, so enter the settings menu
			}
			else if (b_down_value == LOW)
			{
				handleKeyDown();
			}
			else if (b_up_value == LOW)
			{
				handleKeyUp();
			}
			AutoMillis = millis();
			DebounceMillis = millis();
		}
		else if (digitalRead(buttonChDown) == LOW || digitalRead(buttonChUp) == LOW)
		{
			delay(readDelay);
			b_down_value = digitalRead(buttonChDown);	// read again
			b_up_value = digitalRead(buttonChUp);		// read again
			if (b_down_value == LOW && b_up_value == LOW)
			{
				handleKeyMenu();		// user pressed both keys, so enter the main menu
			}
			else if (b_down_value == LOW)
			{
				handleKeyLeft();
			}
			else if (b_up_value == LOW)
			{
				handleKeyRight();
			}
			delay(buttonDelay);
			AutoMillis = millis();
			DebounceMillis = millis();
		}
		else if (digitalRead(buttonMute) == LOW)
		{
			handlePlayPause();			// mute or unmute DACs
			delay(buttonDelay);
			AutoMillis = millis();
			DebounceMillis = millis();
		}
	}
	
	// print status every StatusUpdateInterval (ms) and only when the home screen is showing
	if (millis() - StatusMillis >= StatusUpdateInterval)
	{
		if (gui.GUI_State == GUI::HomeScreen && gui.display())
		{
			dac.getStatus();
			dac.setSampleRate();	// read the sr from the DAC's register and store it
			gui.setSabreDac(&dac);
			gui.printSampleRate(0, 0);
			gui.printInputFormat(4, 3);
			
			if (gui.MainMenuSettings.displayAutoOff && millis() - AutoMillis >= (gui.MainMenuSettings.displayAutoOffTime * 1000))
			{
				toggleDisplay();
				lastKey = 0;	// reset the last keyCode value to prevent unwanted operation
				AutoMillis = millis();
			}
			else if (gui.MainMenuSettings.showAutoClock && millis() - AutoMillis >= (gui.MainMenuSettings.showAutoClockTime * 1000))
			{
				toggleDisplay();
				gui.printLargeTime();
				AutoMillis = millis();
			}
		}
		else if (gui.GUI_State == GUI::HomeScreen && !gui.MainMenuSettings.displayAutoOff && !gui.display())
		{
			gui.printLargeTime();
		}
		
		StatusMillis = millis();
	}
	
} // end of Main Loop

#pragma endregion MAIN_LOOP

#pragma region DISPLAY_METHODS

// toggles between display on/off
void toggleDisplay()
{
	gui.setSabreDac(&dac);
	gui.toggleDisplay();
}

#pragma endregion DISPLAY_METHODS

#pragma region IR_METHODS

// sets the IR code
void setIRcode()
{
	if (results.value == REPEAT)
	{
		IRcode = lastKey;	// make IRcode the same as the last valid IR code
	}
	else
	{
		IRcode = (results.value >> 8) & 0xff;
		IRcode = (IRcode << 1) & 0xff;
	}
}

#pragma endregion IR_METHODS

#pragma region IR_REMOTE_KEY_EVENTS

void handleKeyUp()
{
	switch (gui.GUI_State)
	{
		case GUI::HomeScreen:
		setVolume(INCREASE);
		break;
		case GUI::InputSettingsMenu:
		if (gui.EditMode())
		{
			gui.printNextChar();	// change selected character of the input name
		}
		else
		{
			gui.printSelectedInputSettings(PREVIOUS, gui.SettingsCode());
		}
		break;
		case GUI::MainMenu:
		if (!gui.EditMode())
		{
			gui.printSelectedMainMenuSetting(PREVIOUS);
		}
		else
		{
			if (gui.SelectedMenuSetting == GUI::AdjustTime)
			{
				if (gui.CursorPosition == 1)
				{
					gui.setAndPrintHour(NEXT);
				}
				else
				{
					gui.setAndPrintMinute(NEXT);
				}
			}
			else
			{
				if (gui.CursorPosition == 1)
				{
					gui.setAndPrintDay(NEXT);
				}
				else if (gui.CursorPosition == 2)
				{
					gui.setAndPrintMonth(NEXT);
				}
				else
				{
					gui.setAndPrintYear(NEXT);
				}
			}
		}
		break;
	}
	if (!gui.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

void handleKeyDown()
{
	switch (gui.GUI_State)
	{
		case GUI::HomeScreen:
		setVolume(DECREASE);
		break;
		case GUI::InputSettingsMenu:
		if (gui.EditMode())
		{
			gui.PrintPreviousChar(); // change selected character of the input name
		}
		else
		{
			gui.printSelectedInputSettings(NEXT, gui.SettingsCode());
		}
		break;
		case GUI::MainMenu:
		if (!gui.EditMode())
		{
			gui.printSelectedMainMenuSetting(NEXT);
		}
		else
		{
			if (gui.SelectedMenuSetting == GUI::AdjustTime)
			{
				if (gui.CursorPosition == 1)
				{
					gui.setAndPrintHour(PREVIOUS);
				}
				else
				{
					gui.setAndPrintMinute(PREVIOUS);
				}
			}
			else
			{
				if (gui.CursorPosition == 1)
				{
					gui.setAndPrintDay(PREVIOUS);
				}
				else if (gui.CursorPosition == 2)
				{
					gui.setAndPrintMonth(PREVIOUS);
				}
				else
				{
					gui.setAndPrintYear(PREVIOUS);
				}
			}
		}
		break;
	}
	if (!gui.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

void handleKeyLeft()
{
	switch (gui.GUI_State)
	{
		case GUI::MainMenu:
		changeMainMenuSettings(PREVIOUS);
		SetLastKeyByMainMenu();
		break;
		case GUI::InputSettingsMenu:
		changeInputSetting(PREVIOUS);
		lastKey = 0;
		break;
		default:
		setInput(PREVIOUS);
		lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
		break;
	}
	if (!gui.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

void handleKeyRight()
{
	switch (gui.GUI_State)
	{
		case GUI::MainMenu:
		changeMainMenuSettings(NEXT);
		SetLastKeyByMainMenu();
		break;
		case GUI::InputSettingsMenu:
		changeInputSetting(NEXT);
		lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
		break;
		default:
		setInput(NEXT);
		lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
		break;
	}
	if (!gui.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

void handleKeyCenter()
{	
	switch (gui.GUI_State)
	{
		case GUI::InputSettingsMenu:
		SaveInputSettings();
		gui.printHomeScreen(dac.SelectedInput, dac.Attenuation);
		break;
		case GUI::MainMenu:
		// save changed settings from the main menu and go to the input settings menu
		SaveMenuSettings();
		gui.setSabreDac(&dac);
		gui.printInputSettingsMenu(dac.SelectedInput);
		break;
		default:
		CenterKeyCount++; // count successive received key codes of the center key
		// Enter the input setting menu only after the user pressed the center key for CenterKeyDuration time
		if (CenterKeyCount * remoteDelay > CenterKeyDuration)
		{
			CenterKeyCount = 0;	// reset counter
			gui.setSabreDac(&dac);
			gui.printInputSettingsMenu(dac.SelectedInput);
		}
		break;
		lastKey = 0;	// reset the last keyCode value to prevent unwanted operation
		AutoMillis = millis();
	}
	
}

void handleKeyMenu()
{
	switch (gui.GUI_State)
	{
		case GUI::MainMenu:
		// Save settings and return to the HomeScreen
		SaveMenuSettings();
		gui.printHomeScreen(dac.SelectedInput, dac.Attenuation);
		gui.SelectedMenuSetting = 0;	// reset to default
		break;
		case GUI::InputSettingsMenu:
		SaveInputSettings();
		gui.printMainMenu();
		break;
		default:
		// Enter the Main Menu
		gui.printMainMenu();
		break;
	}
	lastKey = 0;	// reset the last keyCode value to prevent unwanted operation
	AutoMillis = millis();
}

void handlePlayPause() // mute
{
	if (dac.Mute)
	{		
		dac.unMuteDACS();
		if (gui.GUI_State == GUI::HomeScreen)
		{
			gui.printLargeAttenuation(dac.Attenuation, LargeAttnuStartPos);
		}	
	}
	else
	{
		dac.muteDACS();
		if (gui.GUI_State == GUI::HomeScreen)
		{
			gui.printLargeMuteSymbol(LargeAttnuStartPos);
		}
	}
	gui.setSabreDac(&dac);
	lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
	if (!gui.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

#pragma endregion IR_REMOTE_KEY_EVENTS

#pragma region VOLUME_INPUT_SETTING_METHODS

// increases or decreases the volume
void setVolume(byte value)
{
	if (value == INCREASE)
	{
		if (dac.Attenuation > 0)
		{
			dac.setVolume(dac.Attenuation -= 1);
		}
	}
	else
	{
		if (dac.Attenuation < 99)
		{
			dac.setVolume(dac.Attenuation += 1);
		}
	}
	gui.printLargeAttenuation(dac.Attenuation, LargeAttnuStartPos);
}

void setInput(byte value)
{
	Helper::SetPointerValue(value, &dac.SelectedInput, NUMBER_OF_INPUTS -1, 0);
	dac.selectInput(dac.SelectedInput);
	gui.setSabreDac(&dac);
	gui.printInputName(4, 2);
	gui.printLargeInput(dac.SelectedInput, 0);
	dac.writeSelectedInput();
}

void changeInputSetting(int value)
{
	switch (gui.SelectedInputSetting)
	{
		case GUI::InputName:
		gui.setInputNameCursor(value);
		lastKey = 0;
		break;
		case GUI::FirFilter:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].FIR_FILTER, dac.SlowRolloff, 0);
		dac.setFIRrolloffSpeed(dac.Config[dac.SelectedInput].FIR_FILTER);
		gui.setSabreDac(&dac);
		gui.printFIRsetting(1, 1);
		break;
		case GUI::IIRBandwidth:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].IIR_BANDWIDTH, dac.use70K, 0);
		dac.setIIRbandwidth(dac.Config[dac.SelectedInput].IIR_BANDWIDTH);
		gui.setSabreDac(&dac);
		gui.printIIRsetting(1, 1);
		break;
		case GUI::NotchDelay:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].NOTCH_DELAY, dac.MCLK64, 0);
		dac.setNotchDelay(dac.Config[dac.SelectedInput].NOTCH_DELAY);
		gui.setSabreDac(&dac);
		gui.printNotchSetting(1, 1);
		break;
		case GUI::Quantizer:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].QUANTIZER, dac.use9BitsPseudo, 0);
		dac.setQuantizer(dac.Config[dac.SelectedInput].QUANTIZER);
		gui.setSabreDac(&dac);
		gui.printQuantizerSetting(1, 1);
		break;
		case GUI::DpllBandwidth:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].DPLL_BANDWIDTH, dac.DPLL_Best, 0);
		dac.setDPLLbandwidth(dac.Config[dac.SelectedInput].DPLL_BANDWIDTH);
		gui.setSabreDac(&dac);
		gui.printDPLLbandwidthSetting(1, 1);
		break;
		case GUI::DpllBw128:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].DPLL_BW_128X, dac.MultiplyDPLLBandwidthBy128, 0);
		dac.setDPLLBandwidth128x(dac.Config[dac.SelectedInput].DPLL_BW_128X);
		gui.setSabreDac(&dac);
		gui.printDPLLmultiplierSetting(1, 1);
		break;
		case GUI::OverSamplingFilter:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].OSF_FILTER, dac.bypassOSFfilter, 0);
		dac.setOSFfilter(dac.Config[dac.SelectedInput].OSF_FILTER);
		gui.setSabreDac(&dac);
		gui.printOSFfilterSetting(1, 1);
		break;
		case GUI::InputFormat:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].SPDIF_ENABLE, dac.useSPDIF, 0);
		dac.setSPDIFenable(dac.Config[dac.SelectedInput].SPDIF_ENABLE);
		gui.setSabreDac(&dac);
		gui.printSPDIFenableSetting(1, 1);
		break;
		case GUI::SerialDataMode:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].SERIAL_DATA_MODE, dac.RJ, 0);
		dac.setSerialDataMode(dac.Config[dac.SelectedInput].SERIAL_DATA_MODE);
		gui.setSabreDac(&dac);
		gui.printSerialDataModeSetting(1, 1);
		break;
		case GUI::SpdifSource:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].SPDIF_SOURCE, dac.Data8, 0);
		dac.setSPDIFsource(dac.Config[dac.SelectedInput].SPDIF_SOURCE);
		gui.setSabreDac(&dac);
		gui.printSPDIFsourceSetting(1, 1);
		break;
		case GUI::BitMode:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].BIT_MODE, dac.BitMode32, 0);
		dac.setBitMode(dac.Config[dac.SelectedInput].BIT_MODE);
		gui.setSabreDac(&dac);
		gui.printBitmodeSetting(1, 1);
		break;
		case GUI::AutoDeemphasis:
		Helper::SetPointerValue(value, &dac.Config[dac.SelectedInput].DE_EMPHASIS_SELECT, dac.f48kHz, 0);
		dac.setDeEmphasisSelect(dac.Config[dac.SelectedInput].DE_EMPHASIS_SELECT);
		gui.setSabreDac(&dac);
		gui.printDeemphFilterSetting(1, 1);
		break;
	}
}

void SaveInputSettings()
{
	if (gui.EditMode())
	{
		// stop edit mode and timer
		gui.stopTimer();
		// copy the edited input name from the gui class
		dac.Config[dac.SelectedInput] = gui.sabreDAC.Config[dac.SelectedInput];
	}
	dac.writeInputConfiguration();	// write settings to the EEPROM
	gui.SelectedInputSetting = 0;	// reset the Setting variable so the next time the menu starts at the first setting
}


#pragma endregion VOLUME_INPUT_SETTING_METHODS

#pragma region MAIN_MENU_SETTING_METHODS

void changeMainMenuSettings(uint8_t value)
{
	switch (gui.SelectedMenuSetting)
	{
		case GUI::DisplayAutoOff:
		Helper::SetPointerValue(value, &gui.MainMenuSettings.displayAutoOff, 1, 0);
		gui.printDisplayAutoOffSetting(2, 1);
		break;
		case GUI::DisplayAutoOffTime:
		Helper::SetPointerValue(value, &gui.MainMenuSettings.displayAutoOffTime, MAX_TIME, 0);
		gui.printDisplayAutoOffTimeSetting(2, 1);
		break;
		case GUI::DefaultAttnu:
		Helper::SetPointerValue(value, &gui.MainMenuSettings.defaultAttnu, MAX_ATTNU, 0);
		gui.printDefaultAttnuSetting(2, 1);
		break;
		case GUI::ShowAutoClock:
		Helper::SetPointerValue(value, &gui.MainMenuSettings.showAutoClock, 1, 0);
		gui.printShowAutoClockSetting(2, 1);
		break;
		case GUI::ShowAutoClockTime:
		Helper::SetPointerValue(value, &gui.MainMenuSettings.showAutoClockTime, MAX_TIME, 0);
		gui.printAutoClockTimeSetting(2, 1);
		break;
		case GUI::AdjustTime:
		gui.setTimeCursor(value);
		break;
		case GUI::AdjustDate:
		gui.setDateCursor(value);
		break;
		default:
		// do nothing
		break;
	}
}

// sets the lastKey variable to 0 if needed
void SetLastKeyByMainMenu()
{
	if (gui.SelectedMenuSetting != gui.DisplayAutoOffTime && gui.SelectedMenuSetting != gui.DefaultAttnu) // && gui.SelectedMenuSetting != gui.ShowAutoClockTime)
	{
		lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
	}
}


void SaveMenuSettings()
{
	if (gui.EditMode())
	{
		gui.stopTimer();
		gui.saveDateTime();
	}
	gui.writeMainMenuSettings();
	gui.SelectedMenuSetting = 0;
}

#pragma endregion MAIN_MENU_SETTING_METHODS

#pragma region TIMER_METHODS



// timer for blinking the cursor
ISR(TIMER1_COMPA_vect)	//timer1 interrupt 1Hz toggles the cursor
{	
	if (gui.GUI_State == GUI::InputSettingsMenu)
	{
		gui.setInputNameCursor(0); // set cursor back to correct position
		if (Toggle)
		{
			gui.OLED.write(0x5F);
		}
		else
		{
			gui.printSelectedChar();
		}
	}
	else if (gui.GUI_State == GUI::MainMenu)
	{
		if (Toggle)
		{
			if (gui.SelectedMenuSetting == GUI::AdjustTime)
			{
				gui.setTimeCursor(255);
			}
			else
			{
				gui.setDateCursor(255);
			}
		}
		else
		{
			if (gui.SelectedMenuSetting == GUI::AdjustTime)
			{
				gui.setTimeCursor(0);
			}
			else
			{
				gui.setDateCursor(0);
			}
		}
	}
	Toggle = !Toggle;	
}

#pragma endregion TIMER_METHODS


