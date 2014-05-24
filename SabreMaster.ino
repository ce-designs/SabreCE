/*
* SabreMaster
*
* Created: 19-12-2013 23:05:30
* Author: CE-Designs
*/

#include "Sabre.h"
#include "GUI.h"
#include "Wire.h"
#include "EEPROM.h"
#include "CharacterOLED.h"
#include "IRremote.h"

#define INCREASE 0x00
#define DECREASE 0x01

Sabre dac;	// dac

GUI gui(25, 26, 27, 28, 29, 30, 31);	// define the pins for the OLED

IRrecv	irrecv(10);				// set the IR receiver pin
decode_results results;			// object for storing the IR decode results

const byte remoteDelay = 80;	// Delay in milliseconds after each key press of the remote

byte IRcode;
byte lastKey;

unsigned long SR_Millis = 0;

enum GUI_states
{
	HomeScreen, MainMenu, InputSettingsMenu, DacSettingsMenu, DateTimeMenu, DisplayMenu
};

void setup()
{
	//Serial.begin(9600);		// for debugging
	Wire.begin();			// join the I2C bus
	dac.begin(false, 100);	// true = dual mono mode | false = stereo mode, Clock frequency (MHz)
	
	//////////////////////////////////////////////////////////////////////////
	dac.Status.DSD_Mode=true;
	dac.Status.Lock = true;	
	
	//dac.Attenuation = 49;
	//dac.DefaultAttenuation = 49;
	//dac.writeDefaultAttenuation();
	
	//dac.setBitMode(dac.BitMode32);			// or dac.BitMode16 | dac.BitMode20 | dac.BitMode24
	//dac.setSerialDataMode(dac.I2S);			// or dac.LJ | dac.RJ
	//dac.setDPLLbandwidth(dac.DPLL_Best);	//
	//dac.setDeEmphasisSelect(dac.f44_1kHz);	// or dac.f32kHz | dac.f48kHz
	
	//////////////////////////////////////////////////////////////////////////
	
	irrecv.enableIRIn();		// Start the receiver
	
	gui.sabreDAC = dac;
	gui.start();		// Start the user interface + initialize the display
	gui.printHomeScreen(dac.SelectedInput, 99 - dac.Attenuation);
}




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
			delay(remoteDelay * 2);
			lastKey = KEY_LEFT;
			break;
			case KEY_RIGHT:
			handleKeyRight();
			delay(remoteDelay * 2);
			lastKey = KEY_RIGHT;
			break;
			case KEY_CENTER:
			//handleKeyCenter();
			lastKey = KEY_CENTER;
			break;
			case KEY_MENU:
			handleKeyMenu();
			lastKey = KEY_MENU;
			break;
			case KEY_PLAY_PAUSE:			
			lastKey = KEY_PLAY_PAUSE;
			break;
			default:
			// do nothing
			break;
		}
		delay(remoteDelay);
		IRcode = 0;
	}
	
	// print sample rate every 500 ms and only when the home screen is showing
	if (millis() - SR_Millis >= 500 && gui.GUI_State == HomeScreen)
	{
		gui.printSampleRate(5, 2);
		SR_Millis = millis();
	}
	
} // end of Main Loop


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


void handleKeyUp()
{
	switch (gui.GUI_State)
	{
		case HomeScreen:
		setVolume(INCREASE);
		break;
		case MainMenu:
		gui.PrintSelectedInputSettings((gui.Setting -= 1) % SETTINGS_COUNT);
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
		case MainMenu:
		gui.PrintSelectedInputSettings((gui.Setting += 1) % SETTINGS_COUNT);		
		break;
	}
}

void handleKeyLeft()
{
	switch (gui.GUI_State)
	{
		case HomeScreen:
		// CORRECTION FOR INPUT SELECTION ERROR
		if (dac.SelectedInput == 0)
		{
			dac.SelectedInput -= (255 % NUMBER_OF_INPUTS);
			handleKeyLeft();
		}
		// END CORRECTION		
		setInput(dac.SelectedInput -= 1);		
		break;
		case MainMenu:
		changeSetting(-1);
		break;
	}
}

void handleKeyRight()
{
	switch (gui.GUI_State)
	{
		case HomeScreen:
		if (dac.SelectedInput == 255)
		{
			dac.SelectedInput += (255 % NUMBER_OF_INPUTS);
			handleKeyRight();
		}
		setInput(dac.SelectedInput += 1);
		break;
		case MainMenu:
		changeSetting(+1);
		break;
	}
}

void handleKeyMenu()
{
	switch (gui.GUI_State)
	{
		case HomeScreen:
		gui.sabreDAC = dac;
		gui.GUI_State += 1;
		gui.printInputSettingsMenu(dac.SelectedInput);
		break;
		case MainMenu:
		dac.writeInputConfiguration(dac.SelectedInput); // write settings to the EEPROM
		gui.GUI_State -= 1;
		gui.printHomeScreen(dac.SelectedInput, 99 - dac.Attenuation);
		break;
	}	
}

// increases or decreases the volume
void setVolume(byte value)
{
	if (value == INCREASE)
	{
		if (dac.Attenuation < 99)
		{
			dac.setVolume(dac.Attenuation += 1);			
		}
	}
	else
	{
		if (dac.Attenuation > 0)
		{
			dac.setVolume(dac.Attenuation -= 1);
		}
	}
	gui.printLargeAttenuation(99 - dac.Attenuation, 13);
}

void setInput(byte value)
{	
	dac.selectInput(dac.SelectedInput);	
	gui.sabreDAC = dac;
	gui.printInputName(0, 0);
	gui.printLargeInput(dac.SelectedInput, 0);	
	dac.writeSelectedInput();
}

void changeSetting(int value)
{	
	switch (gui.Setting % SETTINGS_COUNT)
	{
		case 0:
		/* Your code here */
		break;
		case 1:
		dac.setFIRrolloffSpeed((dac.Config[dac.SelectedInput].FIR_FILTER += value) % 2);
		gui.sabreDAC = dac;
		gui.printFIRsetting(1, 1);
		break;
		case 2:
		dac.setIIRbandwidth((dac.Config[dac.SelectedInput].IIR_BANDWIDTH += value) % 4);
		gui.sabreDAC = dac;
		gui.printIIRsetting(1, 1);
		break;
		case 3:
		dac.setNotchDelay((dac.Config[dac.SelectedInput].NOTCH_DELAY += value) % 6);
		gui.sabreDAC = dac;
		gui.printNotchSetting(1, 1);
		break;
		case 4:
		dac.setQuantizer((dac.Config[dac.SelectedInput].QUANTIZER += value) % 6);
		gui.sabreDAC = dac;
		gui.printQuantizerSetting(1, 1);
		break;
		case 5:
		dac.setDPLLbandwidth((dac.Config[dac.SelectedInput].DPLL_BANDWIDTH += value) % 9);
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
		dac.setSerialDataMode((dac.Config[dac.SelectedInput].SERIAL_DATA_MODE += value) % 3);
		gui.sabreDAC = dac;
		gui.printSerialDataModeSetting(1, 1);
		break;
		case 10:
		dac.setSPDIFsource((dac.Config[dac.SelectedInput].SPDIF_SOURCE += value) % 8);
		gui.sabreDAC = dac;
		gui.printSPDIFsourceSetting(1, 1);
		break;
		case 11:
		dac.setBitMode((dac.Config[dac.SelectedInput].BIT_MODE += value) % 4);
		gui.sabreDAC = dac;
		gui.printBitmodeSetting(1, 1);
		break;
		case 12:
		dac.setDeEmphasisSelect((dac.Config[dac.SelectedInput].DE_EMPHASIS_SELECT += value) % 4);
		gui.sabreDAC = dac;
		gui.printDeemphFilterSetting(1, 1);
		break;
	}
	
	
}
