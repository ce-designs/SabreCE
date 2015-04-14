/*
* SabreMaster
*
* Created: 19-12-2013 23:05:30
* Author: CE-Designs
*/

#pragma region #DEFINE_DIRECTIVES



#define INCREASE 0x00
#define DECREASE 0x01

#pragma endregion #DEFINE_DIRECTIVES

#pragma region HEADER_FILE_INCLUSIONS

#include "Sabre.h"
#include "SabreController.h"
#include "Wire.h"
#include "EEPROM.h"
#include "IRremote.h"
#ifdef SabreController::CHARACTER_OLED
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

//Sabre dac;							// dac
SabreController controller(25, 26, 27, 28, 29, 30, 31);// define the pins for the OLED
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
	
	// Start the dac, user interface and initialize the display	
	controller.begin(controller.sabreDAC.NormalMode, 100);		// (normal/mono, clock frequency)	
		
	irrecv.enableIRIn();		// Start the receiver	
	initializeButtonPins();		// initialize the pins for the buttons	
	
	controller.printHomeScreen(controller.SelectedInput, controller.sabreDAC.Attenuation);
	
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
			if (lastKey != KEY_CENTER && controller.GUI_State != SabreController::HomeScreen)
			{				
				lastKey = 0;	// probably received a code from another remote control, so reset the last keyCode value to prevent any unwanted operation
			}			
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
		if (controller.GUI_State == SabreController::HomeScreen 
			&& controller.display())
		{		
			controller.sabreDAC.getStatus();
			controller.sabreDAC.getSampleRate();
			controller.printSampleRate(0, 0);
			controller.printInputFormat(4, 3);
			
			if (controller.MainMenuSettings.displayAutoOff 
				&& millis() - AutoMillis >= (controller.MainMenuSettings.displayAutoOffTime * 1000))
			{
				toggleDisplay();
				lastKey = 0;	// reset the last keyCode value to prevent unwanted operation
				AutoMillis = millis();
			}
			else if (controller.MainMenuSettings.showAutoClock 
				&& millis() - AutoMillis >= (controller.MainMenuSettings.showAutoClockTime * 1000))
			{
				toggleDisplay();
				controller.printLargeTime();
				AutoMillis = millis();
			}
		}
		else if (controller.GUI_State == SabreController::HomeScreen 
			&& !controller.MainMenuSettings.displayAutoOff && !controller.display())
		{
			controller.printLargeTime();
		}
		
		StatusMillis = millis();
	}
	
} // end of Main Loop

#pragma endregion MAIN_LOOP

#pragma region DISPLAY_METHODS

// toggles between display on/off
void toggleDisplay()
{
	
	controller.toggleDisplay();
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
		if (results.value >> 16 == PRE_DATA) // IR code received from a Apple remote
		{
			IRcode = (results.value >> 8) & 0xff;
			IRcode = (IRcode << 1) & 0xff;
		}
		else // IR code received from another remote, so set to 0 to abort any further operation
		{
			IRcode = 0
		}		
	}
}

#pragma endregion IR_METHODS

#pragma region IR_REMOTE_KEY_EVENTS

void handleKeyUp()
{
	switch (controller.GUI_State)
	{
		case SabreController::HomeScreen:
		setVolume(INCREASE);
		break;
		case SabreController::InputSettingsMenu:
		if (controller.EditMode())
		{
			controller.printNextChar();	// change selected character of the input name
		}
		else
		{
			controller.printSelectedInputSettings(PREVIOUS, controller.SettingsCode());
		}
		break;
		case SabreController::MainMenu:
		if (!controller.EditMode())
		{
			controller.printSelectedMainMenuSetting(PREVIOUS);
		}
		else
		{
			if (controller.SelectedMenuSetting == SabreController::AdjustTime)
			{
				if (controller.CursorPosition == 1)
				{
					controller.setAndPrintHour(NEXT);
				}
				else
				{
					controller.setAndPrintMinute(NEXT);
				}
			}
			else
			{
				if (controller.CursorPosition == 1)
				{
					controller.setAndPrintDay(NEXT);
				}
				else if (controller.CursorPosition == 2)
				{
					controller.setAndPrintMonth(NEXT);
				}
				else
				{
					controller.setAndPrintYear(NEXT);
				}
			}
		}
		break;
	}
	if (!controller.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

void handleKeyDown()
{
	switch (controller.GUI_State)
	{
		case SabreController::HomeScreen:
		setVolume(DECREASE);
		break;
		case SabreController::InputSettingsMenu:
		if (controller.EditMode())
		{
			controller.PrintPreviousChar(); // change selected character of the input name
		}
		else
		{
			controller.printSelectedInputSettings(NEXT, controller.SettingsCode());
		}
		break;
		case SabreController::MainMenu:
		if (!controller.EditMode())
		{
			controller.printSelectedMainMenuSetting(NEXT);
		}
		else
		{
			if (controller.SelectedMenuSetting == SabreController::AdjustTime)
			{
				if (controller.CursorPosition == 1)
				{
					controller.setAndPrintHour(PREVIOUS);
				}
				else
				{
					controller.setAndPrintMinute(PREVIOUS);
				}
			}
			else
			{
				if (controller.CursorPosition == 1)
				{
					controller.setAndPrintDay(PREVIOUS);
				}
				else if (controller.CursorPosition == 2)
				{
					controller.setAndPrintMonth(PREVIOUS);
				}
				else
				{
					controller.setAndPrintYear(PREVIOUS);
				}
			}
		}
		break;
	}
	if (!controller.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

void handleKeyLeft()
{
	switch (controller.GUI_State)
	{
		case SabreController::MainMenu:
		changeMainMenuSettings(PREVIOUS);
		SetLastKeyByMainMenu();
		break;
		case SabreController::InputSettingsMenu:
		changeInputSetting(PREVIOUS);
		lastKey = 0;
		break;
		default:
		setInput(PREVIOUS);
		lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
		break;
	}
	if (!controller.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

void handleKeyRight()
{
	switch (controller.GUI_State)
	{
		case SabreController::MainMenu:
		changeMainMenuSettings(NEXT);
		SetLastKeyByMainMenu();
		break;
		case SabreController::InputSettingsMenu:
		changeInputSetting(NEXT);
		lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
		break;
		default:
		setInput(NEXT);
		lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
		break;
	}
	if (!controller.display())
	{
		toggleDisplay();
	}
	AutoMillis = millis();
}

void handleKeyCenter()
{	
	switch (controller.GUI_State)
	{
		case SabreController::InputSettingsMenu:
		SaveInputSettings();
		controller.printHomeScreen(controller.SelectedInput, controller.sabreDAC.Attenuation);
		lastKey = 0;	// reset the last keyCode value to prevent unwanted operation
		break;
		case SabreController::MainMenu:
		// save changed settings from the main menu and go to the input settings menu
		SaveMenuSettings();		
		controller.printInputSettingsMenu(controller.SelectedInput);
		lastKey = 0;	// reset the last keyCode value to prevent unwanted operation
		break;
		default:
		CenterKeyCount++; // count successive received key codes of the center key
		// Enter the input setting menu only after the user pressed the center key for CenterKeyDuration time
		if (CenterKeyCount * remoteDelay > CenterKeyDuration)
		{
			CenterKeyCount = 0;	// reset counter			
			controller.printInputSettingsMenu(controller.SelectedInput);
			lastKey = 0;	// reset the last keyCode value to prevent unwanted operation
		}
		break;		
	}
	AutoMillis = millis();
}

void handleKeyMenu()
{
	switch (controller.GUI_State)
	{
		case SabreController::MainMenu:
		// Save settings and return to the HomeScreen
		SaveMenuSettings();
		controller.printHomeScreen(controller.SelectedInput, controller.sabreDAC.Attenuation);
		controller.SelectedMenuSetting = 0;	// reset to default
		break;
		case SabreController::InputSettingsMenu:
		SaveInputSettings();
		controller.printMainMenu();
		break;
		default:
		// Enter the Main Menu
		controller.printMainMenu();
		break;
	}
	lastKey = 0;	// reset the last keyCode value to prevent unwanted operation
	AutoMillis = millis();
}

void handlePlayPause() // mute
{
	if (controller.sabreDAC.Mute)
	{		
		controller.sabreDAC.unMuteDACS();
		if (controller.GUI_State == SabreController::HomeScreen)
		{
			controller.printLargeAttenuation(controller.sabreDAC.Attenuation, LargeAttnuStartPos);
		}	
	}
	else
	{
		controller.sabreDAC.muteDACS();
		if (controller.GUI_State == SabreController::HomeScreen)
		{
			controller.printLargeMuteSymbol(LargeAttnuStartPos);
		}
	}	
	lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
	if (!controller.display())
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
		if (controller.sabreDAC.Attenuation > 0)
		{
			controller.sabreDAC.setAttenuation(controller.sabreDAC.Attenuation -= 1);
		}
	}
	else
	{
		if (controller.sabreDAC.Attenuation < 99)
		{
			controller.sabreDAC.setAttenuation(controller.sabreDAC.Attenuation += 1);
		}
	}
	controller.printLargeAttenuation(controller.sabreDAC.Attenuation, LargeAttnuStartPos);
}

void setInput(byte value)
{
	Helper::SetPointerValue(value, &controller.SelectedInput, NUMBER_OF_INPUTS -1, 0);
	controller.selectInput(controller.SelectedInput);	
	controller.printInputName(4, 2);
	controller.printLargeInput(controller.SelectedInput, 0);
	controller.writeSelectedInput();
}

void changeInputSetting(int value)
{
	switch (controller.SelectedInputSetting)
	{
		case SabreController::InputName:
		controller.setInputNameCursor(value);
		lastKey = 0;
		break;
		case SabreController::FirFilter:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].FIR_FILTER, controller.sabreDAC.SlowRolloff, 0);
		controller.sabreDAC.setFIRrolloffSpeed(controller.Config[controller.SelectedInput].FIR_FILTER);		
		controller.printFIRsetting(1, 1);
		break;
		case SabreController::IIRBandwidth:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].IIR_BANDWIDTH, controller.sabreDAC.use70K, 0);
		controller.sabreDAC.setIIRbandwidth(controller.Config[controller.SelectedInput].IIR_BANDWIDTH);		
		controller.printIIRsetting(1, 1);
		break;
		case SabreController::NotchDelay:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].NOTCH_DELAY, controller.sabreDAC.MCLK64, 0);
		controller.sabreDAC.setNotchDelay(controller.Config[controller.SelectedInput].NOTCH_DELAY);		
		controller.printNotchSetting(1, 1);
		break;
		case SabreController::Quantizer:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].QUANTIZER, controller.sabreDAC.use9BitsPseudo, 0);
		controller.sabreDAC.setQuantizer(controller.Config[controller.SelectedInput].QUANTIZER);		
		controller.printQuantizerSetting(1, 1);
		break;
		case SabreController::DpllBandwidth:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].DPLL_BANDWIDTH, controller.sabreDAC.DPLL_Best, 0);
		controller.sabreDAC.setDPLLbandwidth(controller.Config[controller.SelectedInput].DPLL_BANDWIDTH);		
		controller.printDPLLbandwidthSetting(1, 1);
		break;
		case SabreController::DpllBw128:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].DPLL_BW_128X, controller.sabreDAC.MultiplyDPLLBandwidthBy128, 0);
		controller.sabreDAC.setDPLLBandwidth128x(controller.Config[controller.SelectedInput].DPLL_BW_128X);		
		controller.printDPLLmultiplierSetting(1, 1);
		break;
		case SabreController::OverSamplingFilter:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].OSF_FILTER, controller.sabreDAC.bypassOSFfilter, 0);
		controller.sabreDAC.setOSFfilter(controller.Config[controller.SelectedInput].OSF_FILTER);		
		controller.printOSFfilterSetting(1, 1);
		break;
		case SabreController::InputFormat:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].SPDIF_ENABLE, controller.sabreDAC.useSPDIF, 0);
		controller.sabreDAC.setSPDIFenable(controller.Config[controller.SelectedInput].SPDIF_ENABLE);		
		controller.printSPDIFenableSetting(1, 1);
		break;
		case SabreController::SerialDataMode:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].SERIAL_DATA_MODE, controller.sabreDAC.RJ, 0);
		controller.sabreDAC.setSerialDataMode(controller.Config[controller.SelectedInput].SERIAL_DATA_MODE);		
		controller.printSerialDataModeSetting(1, 1);
		break;
		case SabreController::SpdifSource:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].SPDIF_SOURCE, controller.sabreDAC.Data8, 0);
		controller.sabreDAC.setSPDIFsource(controller.Config[controller.SelectedInput].SPDIF_SOURCE);		
		controller.printSPDIFsourceSetting(1, 1);
		break;
		case SabreController::BitMode:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].BIT_MODE, controller.sabreDAC.BitMode32, 0);
		controller.sabreDAC.setBitMode(controller.Config[controller.SelectedInput].BIT_MODE);		
		controller.printBitmodeSetting(1, 1);
		break;
		case SabreController::AutoDeemphasis:
		Helper::SetPointerValue(value, &controller.Config[controller.SelectedInput].DE_EMPHASIS_SELECT, controller.sabreDAC.f48kHz, 0);
		controller.sabreDAC.setDeEmphasisSelect(controller.Config[controller.SelectedInput].DE_EMPHASIS_SELECT);		
		controller.printDeemphFilterSetting(1, 1);
		break;
	}
}

void SaveInputSettings()
{
	if (controller.EditMode())
	{
		// stop edit mode and timer
		controller.stopTimer();		
	}
	controller.writeInputConfiguration();	// write settings to the EEPROM
	controller.SelectedInputSetting = 0;	// reset the Setting variable so the next time the menu starts at the first setting
}


#pragma endregion VOLUME_INPUT_SETTING_METHODS

#pragma region MAIN_MENU_SETTING_METHODS

void changeMainMenuSettings(uint8_t value)
{
	switch (controller.SelectedMenuSetting)
	{
		case SabreController::DisplayAutoOff:
		Helper::SetPointerValue(value, &controller.MainMenuSettings.displayAutoOff, 1, 0);
		controller.printDisplayAutoOffSetting(2, 1);
		break;
		case SabreController::DisplayAutoOffTime:
		Helper::SetPointerValue(value, &controller.MainMenuSettings.displayAutoOffTime, MAX_TIME, 0);
		controller.printDisplayAutoOffTimeSetting(2, 1);
		break;
		case SabreController::DefaultAttnu:
		Helper::SetPointerValue(value, &controller.MainMenuSettings.defaultAttenuation, MAX_ATTNU, 0);
		controller.printDefaultAttnuSetting(2, 1);
		break;
		case SabreController::ShowAutoClock:
		Helper::SetPointerValue(value, &controller.MainMenuSettings.showAutoClock, 1, 0);
		controller.printShowAutoClockSetting(2, 1);
		break;
		case SabreController::ShowAutoClockTime:
		Helper::SetPointerValue(value, &controller.MainMenuSettings.showAutoClockTime, MAX_TIME, 0);
		controller.printAutoClockTimeSetting(2, 1);
		break;
		case SabreController::AdjustTime:
		controller.setTimeCursor(value);
		break;
		case SabreController::AdjustDate:
		controller.setDateCursor(value);
		break;
		default:
		// do nothing
		break;
	}
}

// sets the lastKey variable to 0 if needed
void SetLastKeyByMainMenu()
{
	if (controller.SelectedMenuSetting != controller.DisplayAutoOffTime && controller.SelectedMenuSetting != controller.DefaultAttnu) // && gui.SelectedMenuSetting != gui.ShowAutoClockTime)
	{
		lastKey = 0;			// reset the last keyCode value to prevent any unwanted operation
	}
}

void SaveMenuSettings()
{
	if (controller.EditMode())
	{
		controller.stopTimer();
		controller.saveDateTime();
	}
	controller.writeMainMenuSettings();
	controller.SelectedMenuSetting = 0;
}

#pragma endregion MAIN_MENU_SETTING_METHODS

#pragma region TIMER_METHODS



// timer for blinking the cursor
ISR(TIMER1_COMPA_vect)	//timer1 interrupt 1Hz toggles the cursor
{	
	if (controller.GUI_State == SabreController::InputSettingsMenu)
	{
		controller.setInputNameCursor(0); // set cursor back to correct position
		if (Toggle)
		{
			controller.OLED.write(0x5F);
		}
		else
		{
			controller.printSelectedChar();
		}
	}
	else if (controller.GUI_State == SabreController::MainMenu)
	{
		if (Toggle)
		{
			if (controller.SelectedMenuSetting == SabreController::AdjustTime)
			{
				controller.setTimeCursor(255);
			}
			else
			{
				controller.setDateCursor(255);
			}
		}
		else
		{
			if (controller.SelectedMenuSetting == SabreController::AdjustTime)
			{
				controller.setTimeCursor(0);
			}
			else
			{
				controller.setDateCursor(0);
			}
		}
	}
	Toggle = !Toggle;	
}

#pragma endregion TIMER_METHODS


