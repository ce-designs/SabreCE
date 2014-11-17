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

#pragma endregion HEADER_FILE_INCLUSIONS

#pragma region GLOBAL_VARIABLES

Sabre dac;							// dac

GUI gui(25, 26, 27, 28, 29, 30, 31);// define the pins for the OLED

IRrecv	irrecv(10);					// set the IR receiver pin
decode_results results;				// object for storing the IR decode results

byte IRcode;						// for storing the current IR code
byte lastKey;						// for remembering the last IR Remote key

byte CenterKeyCount = 0;			// for determining how long the user pressed the center key

unsigned long StatusMillis = 0;		// for recording the time of the last status update
byte statusUpdateCount = 0;			// 

bool Toggle;						// used by timer2, see TIMER_METHODS

#pragma endregion GLOBAL_VARIABLES

#pragma region CONSTANTS

const byte remoteDelay = 140;	// Delay in milliseconds after each key press of the remote

const unsigned int CenterKeyDuration = 1000;	//  Time in milliseconds that the center key needs to be pressed before it invokes a method

const unsigned int StatusUpdateInterval = 500;	// the interval between each status update

const byte LargeAttnuStartPos = 13;	// the start position for printing the large attenuation 

#pragma endregion CONSTANTS

#pragma region ENUMS

// must be the exact same as in the GUI header file!
enum GUI_states
{
	HomeScreen, InputSettingsMenu, MainMenu
};

#pragma endregion ENUMS

#pragma region SETUP

void setup()
{
	Serial.begin(9600);		// for debugging
	
	Wire.begin();			// join the I2C bus
	dac.begin(false, 100);	// true = dual mono mode | false = stereo mode, Clock frequency (MHz)
	
	irrecv.enableIRIn();		// Start the receiver
	
	gui.start();		// Start the user interface + initialize the display
	gui.sabreDAC = dac;
	gui.printHomeScreen(dac.SelectedInput, 99 - dac.Attenuation);
}

#pragma endregion SETUP

#pragma region MAIN_LOOP

// Main Loop
void loop()
{
	// IR routines
	if (irrecv.decode(&results)) {
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
			handleKeyUp();
			lastKey = KEY_UP;
			break;
			case KEY_DOWN:
			handleKeyDown();
			lastKey = KEY_DOWN;
			break;
			case KEY_LEFT:
			handleKeyLeft();
			delay(remoteDelay * 2); // add some extra delay
			lastKey = KEY_LEFT;
			break;
			case KEY_RIGHT:
			handleKeyRight();
			delay(remoteDelay * 2); // add some extra delay
			lastKey = KEY_RIGHT;
			break;
			case KEY_CENTER:
			handleKeyCenter();
			lastKey = KEY_CENTER;
			break;
			case KEY_MENU:
			handleKeyMenu();
			lastKey = KEY_MENU;
			break;
			case KEY_PLAY_PAUSE:
			handlePlayPause();
			lastKey = KEY_PLAY_PAUSE;
			break;
			default:
			// do nothing
			break;
		}
		delay(remoteDelay);
		IRcode = 0;
	}
	
	
	// print status every StatusUpdateInterval (ms) and only when the home screen is showing
	if (gui.GUI_State == HomeScreen && millis() - StatusMillis >= StatusUpdateInterval)
	{
		dac.getStatus();
		dac.getSampleRate();
		gui.sabreDAC = dac;
		gui.printSampleRate(0, 0);
		gui.printInputFormat(4, 3);
		statusUpdateCount++;
		if (statusUpdateCount * StatusUpdateInterval > CenterKeyDuration)
		{
			statusUpdateCount = 0;	// reset counter
			CenterKeyCount = 0;		// reset counter
		}
		StatusMillis = millis();		
	}
	
} // end of Main Loop

#pragma endregion MAIN_LOOP

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
		case HomeScreen:
		setVolume(INCREASE);
		break;
		case InputSettingsMenu:
		if (gui.InputNameEditMode())
		{
			gui.printNextChar();	// change selected character of the input name
		}
		else
		{
			gui.PrintSelectedInputSettings(PREVIOUS);
		}		
		break;
	}
}

void handleKeyDown()
{
	switch (gui.GUI_State)
	{
		case HomeScreen:
		setVolume(DECREASE);
		break;
		case InputSettingsMenu:
		if (gui.InputNameEditMode())
		{
			gui.PrintPreviousChar(); // change selected character of the input name						
		}
		else
		{
			gui.PrintSelectedInputSettings(NEXT);
		}		
		break;
	}
}

void handleKeyLeft()
{
	switch (gui.GUI_State)
	{
		case HomeScreen:
		setInput(-1);
		break;
		case InputSettingsMenu:		
		changeInputSetting(-1);
		break;
	}
}

void handleKeyRight()
{
	switch (gui.GUI_State)
	{
		case HomeScreen:
		setInput(+1);
		break;
		case InputSettingsMenu:
		changeInputSetting(+1);
		break;
	}
}

void handleKeyCenter()
{	
	CenterKeyCount++; // count successive received key codes of the center key
	// Enter the input setting menu only after the user pressed the center key for CenterKeyDuration time
	if (CenterKeyCount * remoteDelay > CenterKeyDuration)
	{
		CenterKeyCount = 0;	// reset counter
		gui.sabreDAC = dac;		
		gui.printInputSettingsMenu(dac.SelectedInput);
	}
		
	
}

void handleKeyMenu()
{
	switch (gui.GUI_State)
	{
		case HomeScreen:
		// Enter the Main Menu
		gui.printMainMenu();
		
		break;
		case InputSettingsMenu:
		// Save settings and return to the HomeScreen
		if (gui.InputNameEditMode())
		{
			// stop edit mode and timer
			gui.stopInputNameEditMode();
			// copy the edited input name from the gui class
			dac.Config[dac.SelectedInput] = gui.sabreDAC.Config[dac.SelectedInput];		
		}
		dac.writeInputConfiguration(); // write settings to the EEPROM
		gui.SelectedInputSetting = 0; // reset the Setting variable so the next time the menu starts at the first setting
		gui.printHomeScreen(dac.SelectedInput, dac.Attenuation);
		break;
		case MainMenu:
		// Save settings and return to the HomeScreen
		gui.printHomeScreen(dac.SelectedInput, dac.Attenuation);	
		break;
	}
}

void handlePlayPause()
{
	if (dac.Mute)
	{		
		dac.unMuteDACS();
		gui.printLargeAttenuation(dac.Attenuation, LargeAttnuStartPos);		
	}
	else
	{
		dac.muteDACS();
		gui.printLargeMuteSymbol(LargeAttnuStartPos);
	}
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
	dac.SelectedInput += value;
	// CORRECTION FOR INPUT SELECTION ERROR
	if (dac.SelectedInput == 255)
	{
		dac.SelectedInput = NUMBER_OF_INPUTS -1;
	}
	else if (dac.SelectedInput == NUMBER_OF_INPUTS)
	{
		dac.SelectedInput = 0;
	}
	// END CORRECTION
	
	dac.selectInput(dac.SelectedInput);
	gui.sabreDAC = dac;
	gui.printInputName(4, 2);
	gui.printLargeInput(dac.SelectedInput, 0);
	dac.writeSelectedInput();
}

void changeInputSetting(int value)
{
	switch (gui.SelectedInputSetting % SETTINGS_COUNT)
	{
		case 0:
		if (value == -1) value = PREVIOUS;		
		gui.SetCursorPosition(value);
		break;
		case 1:
		dac.setFIRrolloffSpeed((dac.Config[dac.SelectedInput].FIR_FILTER += value) % 2);
		gui.sabreDAC = dac;
		gui.printFIRsetting(1, 1);
		break;
		case 2:
		dac.setIIRbandwidth((dac.Config[dac.SelectedInput].IIR_BANDWIDTH += (value + 4)) % 4);
		gui.sabreDAC = dac;
		gui.printIIRsetting(1, 1);
		break;
		case 3:
		dac.setNotchDelay((dac.Config[dac.SelectedInput].NOTCH_DELAY += (value + 6)) % 6);
		gui.sabreDAC = dac;
		gui.printNotchSetting(1, 1);
		break;
		case 4:
		dac.setQuantizer((dac.Config[dac.SelectedInput].QUANTIZER += (value + 6)) % 6);
		gui.sabreDAC = dac;
		gui.printQuantizerSetting(1, 1);
		break;
		case 5:
		dac.setDPLLbandwidth((dac.Config[dac.SelectedInput].DPLL_BANDWIDTH += (value + 9)) % 9);
		gui.sabreDAC = dac;
		gui.printDPLLbandwidthSetting(1, 1);
		break;
		case 6:
		dac.setDPLLBandwidth128x((dac.Config[dac.SelectedInput].DPLL_BW_128X += value) % 2);
		gui.sabreDAC = dac;
		gui.printDPLLmultiplierSetting(1, 1);
		break;
		case 7:
		dac.setOSFfilter((dac.Config[dac.SelectedInput].OSF_FILTER += value) % 2);
		gui.sabreDAC = dac;
		gui.printOSFfilterSetting(1, 1);
		break;
		case 8:
		dac.setSPDIFenable((dac.Config[dac.SelectedInput].SPDIF_ENABLE += value) % 2);
		gui.sabreDAC = dac;
		gui.printSPDIFenableSetting(1, 1);
		break;
		case 9:
		dac.setSerialDataMode((dac.Config[dac.SelectedInput].SERIAL_DATA_MODE += (value + 3)) % 3);
		gui.sabreDAC = dac;
		gui.printSerialDataModeSetting(1, 1);
		break;
		case 10:
		dac.setSPDIFsource((dac.Config[dac.SelectedInput].SPDIF_SOURCE += (value + 8)) % 8);
		gui.sabreDAC = dac;
		gui.printSPDIFsourceSetting(1, 1);
		break;
		case 11:
		dac.setBitMode((dac.Config[dac.SelectedInput].BIT_MODE += (value + 4)) % 4);
		gui.sabreDAC = dac;
		gui.printBitmodeSetting(1, 1);
		break;
		case 12:
		dac.setDeEmphasisSelect((dac.Config[dac.SelectedInput].DE_EMPHASIS_SELECT += (value + 4)) % 4);
		gui.sabreDAC = dac;
		gui.printDeemphFilterSetting(1, 1);
		break;
	}
}

#pragma endregion VOLUME_INPUT_SETTING_METHODS

#pragma region TIMER_METHODS



// timer for blinking the cursor
ISR(TIMER1_COMPA_vect)	//timer1 interrupt 1Hz toggles the cursor
{	
	gui.SetCursorPosition(0); // set cursor back to correct position
	if (Toggle)
	{
		gui.OLED.write(0x5F);
	}
	else
	{
		gui.printSelectedChar();
	}
	Toggle = !Toggle;	
}

#pragma endregion TIMER_METHODS