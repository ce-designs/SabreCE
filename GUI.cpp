/*
* GUI.cpp
*
* Created: 28-12-2013 17:02:16
* Author: CE-Designs
*/

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "GUI.h"
#include "CharacterOLED.h"
#include "Sabre.h"
#include "RTClib.h"
#include "EEPROM_Anything.h"
#include "Helper.h"

// default constructor
GUI::GUI(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	// OLED_V1 = older, OLED_V2 = newer. If 2 doesn't work try 1 ;)
	OLED = CharacterOLED(OLED_V2, rs, rw, enable, d4 , d5, d6, d7);
	
} //GUI

void GUI::begin()
{
	OLED.begin(20, 4);	// initialize the 20x4 OLED
	rtc.begin();
	
	if (!rtc.isrunning())
	{
		rtc.adjust(DateTime(2014, 1, 1, 0, 0)); // set the date to 1 January 2014 and time to 12:00 hrs
	}
	
	if (Helper::firstRun(EEPROM_GUI_FIRST_RUN))
	{
		applyDefaultSettings();
		writeMainMenuSettings();
		EEPROM.write(EEPROM_GUI_FIRST_RUN, FIRST_RUN); // write flag to EEPROM
	}
	else
	{
		readMainMenuSettings();			// read the main menu settings from the EEPROM
	}
	
	this->GUI_State = HomeScreen;
	this->SelectedInputSetting = InputName;
	this->SelectedMenuSetting = DisplayAutoOff;
	this->CursorPosition = 0;
	this->TimerEnabled = false;
}

// read all main menu settings from the EEPROM
void GUI::readMainMenuSettings()
{
	EEPROM_readAnything(EEPROM_MENU_SETTINGS, this->MainMenuSettings);
}

// write all changed main menu settings to the EEPROM
void GUI::writeMainMenuSettings()
{
	EEPROM_writeAnything(EEPROM_MENU_SETTINGS, this->MainMenuSettings);
}

// DEFAULT MAIN MENU SETTINGS
void GUI::applyDefaultSettings()
{
	MainMenuSettings.displayAutoOff = 0;		// = false, thus no Display Auto Off
	MainMenuSettings.displayAutoOffTime = 4;	// = 4 seconds
	MainMenuSettings.showAutoClock = 0;			// = false, thus no Auto clock
	MainMenuSettings.showAutoClockTime = 4;		// = 4 seconds
	MainMenuSettings.defaultAttnu = 50;			// = 50 dB attenuation
}

void GUI::setSabreDac( Sabre *dac )
{
	sabreDAC = *dac;
}

void GUI::printLargeAttenuation(uint8_t Attenuation, uint8_t col)
{
	OLED.setCursor(col, 0);
	OLED.print("Vol -dB");
	if (Attenuation < 10)
	{
		OLED.printLargeNumber(0, col, 1);
		OLED.printLargeNumber(Attenuation, col + 4, 1);
	}
	else
	{
		OLED.printLargeNumber(Attenuation, col, 1);
	}
}

void GUI::printLargeMuteSymbol(uint8_t col)
{
	OLED.setCursor(col, 0);
	OLED.print("Mute   ");
	uint8_t x = 1;
	
	for (uint8_t i = 0; i < 21; i++)
	{
		if (i % 7 == 0)			// define cursor position
		{
			OLED.setCursor(col, x++);
		}
		if (i < 7 || i == 10 || i > 13)
		{
			OLED.write(0x20);	// print blank
		}
		else
		{
			OLED.write(0x03);	// print block
		}
	}
}

void GUI::printLargeInput(uint8_t selectedInput, uint8_t col)
{
	OLED.printLargeNumber((selectedInput % NUMBER_OF_INPUTS) + 1, col, 1);
}

void GUI::printInputName(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	for (uint8_t i = 0; i < INPUT_NAME_SIZE; i++)
	{
		OLED.write(sabreDAC.Config[sabreDAC.SelectedInput % NUMBER_OF_INPUTS].INPUT_NAME[i]);
	}
}

void GUI::printSampleRate(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	if (!sabreDAC.Status.Lock)
	{
		OLED.print("No Lock  ");
	}
	else
	{
		if (Status.DSD_Mode)
		{
			if(sabreDAC.getSampleRate() > 6143000)
			{
				OLED.print("6.1 MHz  ");
			}
			else
			{
				if(sabreDAC.getSampleRate() > 5644000)
				{
					OLED.print( "5.6 MHz  ");
				}
				else
				{
					if(sabreDAC.getSampleRate() > 3071000)
					{
						OLED.print("3.0 MHz  ");
					}
					else
					{
						if(sabreDAC.getSampleRate() > 2822000)
						{
							OLED.print("2.8 MHz  ");
						}
						else
						{
							OLED.print("Unknown  ");
						}
					}
				}
			}
		}
		else
		{                      // If not DSD_Mode then it is either I2S or SPDIF
			if(sabreDAC.getSampleRate() > 383900)
			{
				OLED.print("384 KHz  ");
			}
			else
			{
				if(sabreDAC.getSampleRate() > 352700)
				{
					OLED.print("352.8 KHz");
				}
				else
				{
					if(sabreDAC.getSampleRate() > 191900)
					{
						OLED.print("192 KHz  ");
					}
					else
					{
						if(sabreDAC.getSampleRate() > 176300)
						{
							OLED.print("176.4 KHz");
						}
						else
						{
							if(sabreDAC.getSampleRate() > 95900)
							{
								OLED.print("96 KHz   ");
							}
							else
							{
								if(sabreDAC.getSampleRate() > 88100)
								{
									OLED.print("88 KHz   ");
								}
								else
								{
									if(sabreDAC.getSampleRate() > 47900)
									{
										OLED.print("48 KHz   ");
									}
									else if (sabreDAC.getSampleRate() > 43900)
									{
										OLED.print("44.1 KHz ");
									}
									else
									{
										OLED.print("Unknown  ");
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void GUI::printInputFormat(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	if (!sabreDAC.Status.Lock)
	{
		OLED.print("     ");
	}
	else if (sabreDAC.Status.DSD_Mode)
	{
		OLED.print("DSD  ");
	}
	else if (sabreDAC.Status.SPDIF_Valid)
	{
		OLED.print("SPDIF");
	}
	else
	{
		OLED.print("PCM  ");
	}
}

void GUI::printHomeScreen(uint8_t selectedInput, uint8_t attenuation)
{
	OLED.defineCustomChar0();					// restore custom character 0
	OLED.defineCustomChar1();					// restore custom character 1
	OLED.defineCustomChar2();					// restore custom character 2
	OLED.clear();
	
	switch (GUI_Substate)
	{
		case NoVolumeNumbersHS:					// Home Screen: No volume numbers
		printInputName(0, 0);
		printLargeInput(selectedInput, 16);
		printSampleRate(0, 2);
		printInputFormat(0, 3);
		break;
		case NoInputNumberHS:					// Home Screen: No large input number
		printInputName(0, 0);
		printSampleRate(0, 2);
		printInputFormat(0, 3);
		if (sabreDAC.Mute)
		{
			printLargeMuteSymbol(13);
		}
		else
		{
			printLargeAttenuation(attenuation, 13);
		}
		break;
		default:								// Home Screen: (default) use large input and volume numbers
		printInputName(4, 2);
		printLargeInput(selectedInput, 0);
		if (sabreDAC.Mute)
		{
			printLargeMuteSymbol(13);
		}
		else
		{
			printLargeAttenuation(attenuation, 13);
		}
		printSampleRate(0, 0);
		printInputFormat(4, 3);
		break;
	}
	NoDisplay = false;
	this->GUI_State = HomeScreen;
}



void GUI::printInputSettingsMenu(uint8_t selectedInput)
{
	PrepareForMenuPrinting();					// defines new custom chars and clears the display
	String input = "INPUT";
	printTitleBar(input + (selectedInput + 1));	// print Input Settings Menu: Print title bar with the selected input
	
	
	printSelectedInputSettings(255, this->SettingsCode());		// print all activated input settings
	
	printPointer();
	
	OLED.setCursor(19,3);
	OLED.write(1);
	
	NoDisplay = false;
	this->GUI_State = InputSettingsMenu;
}

void GUI::printSelectedInputSettings(uint8_t value, int code)
{
	
	Helper::SetPointerValue(value, &this->SelectedInputSetting, INPUT_SETTINGS_COUNT - 1, 0);
	

	uint8_t row = 1;
	uint8_t col = 1;
	
	for (uint8_t i = this->SelectedInputSetting; i < INPUT_SETTINGS_COUNT; i++)
	{
		if (bitRead(code, i))
		{
			switch (i)
			{
				case InputName:
				printInputNameSetting(col, row);
				break;
				case FirFilter:
				printFIRsetting(col, row);
				break;
				case IIRBandwidth:
				printIIRsetting(col, row);
				break;
				case NotchDelay:
				printNotchSetting(col, row);
				break;
				case Quantizer:
				printQuantizerSetting(col, row);
				break;
				case DpllBandwidth:
				printDPLLbandwidthSetting(col, row);
				break;
				case DpllBw128:
				printDPLLmultiplierSetting(col, row);
				break;
				case OverSamplingFilter:
				printOSFfilterSetting(col, row);
				break;
				case InputFormat:
				printSPDIFenableSetting(col, row);
				break;
				case SerialDataMode:
				printSerialDataModeSetting(col, row);
				break;
				case SpdifSource:
				printSPDIFsourceSetting(col, row);
				break;
				case BitMode:
				printBitmodeSetting(col, row);
				break;
				case AutoDeemphasis:
				printDeemphFilterSetting(col, row);
				break;
				default:
				printEmptyRow(row);
				break;
			}
			row++;
			if (row > 3) break;	// stop printing
		}
	}
	for (uint8_t i = row; i < 4; i++)
	{
		printEmptyRow(i);
	}
	
}

void GUI::PrepareForMenuPrinting()
{
	OLED.defineUpwardsArrowChar(UPWARDS_ARROW);
	OLED.defineDownwardsArrowChar(DOWNWARDS_ARROW);
	OLED.defineLockedChar(LOCK_SYMBOL);
	OLED.clear();
}

void GUI::printEmptyRow(uint8_t row)
{
	OLED.setCursor(0, row);
	for (uint8_t i = 0; i < 20; i++)
	{
		OLED.write(0x20);
	}
}

void GUI::printLockSymbol(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.write(2);				// print lock symbol
}

void GUI::printTitleBar(String title)
{
	uint8_t m = 0;
	uint8_t i  = 20 - title.length();
	if (i % 2)
	{
		m = 1;
	}
	OLED.setCursor(0, 0);
	for (uint8_t x = 0; x < (i / 2); x++)			// Print Title Bar: Print the set of first characters '_'
	{
		OLED.write(0x5F);
	}
	OLED.print(title);								// Print Title Bar: Print the title
	for (uint8_t x = 0; x < (i / 2) + m; x++)		// Print Title Bar: Print the last set of characters '_'
	{
		OLED.write(0x5F);
	}
}

void GUI::printInputNameSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("INP: ");
	printInputName(col + 5, row);
	// fill row with blanks
	for (uint8_t i = 0; i < 4; i++)
	{
		OLED.write(0x20);	// print blank
	}
}

void GUI::printFIRsetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("FIR: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].FIR_FILTER)
	{
		case SlowRolloff:
		OLED.print("Slow Rolloff");
		break;
		default:
		OLED.print("Fast Rolloff");
		break;
	}
}

void GUI::printIIRsetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("IIR: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].IIR_BANDWIDTH)
	{
		case normalIIR:
		OLED.print("47K (PCM)   ");
		break;
		case use60K:
		OLED.print("60K (DSD)   ");
		break;
		case use70K:
		OLED.print("70K (DSD)   ");
		break;
		default:
		OLED.print("50K (DSD)   ");
		break;
	}
}

void GUI::printNotchSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("NCH: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].NOTCH_DELAY)
	{
		case MCLK4:
		OLED.print("MCLK/4      ");
		break;
		case MCLK8:
		OLED.print("MCLK/8      ");
		break;
		case MCLK16:
		OLED.print("MCLK/16     ");
		break;
		case MCLK32:
		OLED.print("MCLK/32     ");
		break;
		case MCLK64:
		OLED.print("MCLK/64     ");
		break;
		default:
		OLED.print("No Notch    ");
		break;
	}
}

void GUI::printQuantizerSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("QTZ: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].QUANTIZER)
	{
		case use7BitsTrue:
		OLED.print("7Bit true   ");
		break;
		case use7BitsPseudo:
		OLED.print("7Bit pseudo ");
		break;
		case use8BitsTrue:
		OLED.print("8Bit true   ");
		break;
		case use8BitsPseudo:
		OLED.print("8Bit pseudo ");
		break;
		case use9BitsPseudo:
		OLED.print("9Bit pseudo ");
		break;
		default:
		OLED.print("6Bit true   ");
		break;
	}
}

void GUI::printDPLLbandwidthSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("PLL: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].DPLL_BANDWIDTH)
	{
		case DPLL_NoBandwidth:
		OLED.print("No DPLL bw  ");
		break;
		case DPLL_Lowest:
		OLED.print("Lowest      ");
		break;
		case DPLL_Low:
		OLED.print("Low         ");
		break;
		case DPLL_MedLow:
		OLED.print("Medium-Low  ");
		break;
		case DPLL_Medium:
		OLED.print("Medium      ");
		break;
		case DPLL_MedHigh:
		OLED.print("Medium-High ");
		break;
		case DPLL_High:
		OLED.print("High        ");
		break;
		case DPLL_Highest:
		OLED.print("Highest     ");
		break;
		default:
		OLED.print("Best        ");
		break;
	}
}

void GUI::printDPLLmultiplierSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("PLM: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].DPLL_BW_128X)
	{
		case MultiplyDPLLBandwidthBy128:
		OLED.print("DPLL 128x   ");
		break;
		default:
		OLED.print("DPLL bw     ");
		break;
	}
}

void GUI::printOSFfilterSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("OSF: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].OSF_FILTER)
	{
		case bypassOSFfilter:
		OLED.print("Bypass OSF  ");
		break;
		default:
		OLED.print("OSF Filter  ");
		break;
	}
}

void GUI::printSPDIFenableSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("INF: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].SPDIF_ENABLE)
	{
		case useSPDIF:
		OLED.print("S/PDIF      ");
		break;
		default:
		OLED.print("I2S/DSD     ");
		break;
	}
}

void GUI::printSerialDataModeSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("SDM: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].SERIAL_DATA_MODE)
	{
		case LJ:
		OLED.print("LJ           ");
		break;
		case RJ:
		OLED.print("RJ           ");
		break;
		default:
		OLED.print("I2S          ");
		break;
	}
}

void GUI::printSPDIFsourceSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("SRC: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].SPDIF_SOURCE)
	{
		case Data2:
		OLED.print("Data 2      ");
		break;
		case Data3:
		OLED.print("Data 3      ");
		break;
		case Data4:
		OLED.print("Data 4      ");
		break;
		case Data5:
		OLED.print("Data 5      ");
		break;
		case Data6:
		OLED.print("Data 6      ");
		break;
		case Data7:
		OLED.print("Data 7      ");
		break;
		case Data8:
		OLED.print("Data 8      ");
		break;
		default:
		//if (sabreDAC.Config[sabreDAC.SelectedInput].SPDIF_ENABLE == useI2SorDSD)
		//{
		//printLockSymbol(col + 4, row);
		//}
		OLED.print("Data 1      ");
		break;
	}
}

void GUI::printBitmodeSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("BTM: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].BIT_MODE)
	{
		case BitMode16:
		OLED.print("16 Bit      ");
		break;
		case BitMode20:
		OLED.print("20 Bit      ");
		break;
		case BitMode24:
		OLED.print("24 Bit      ");
		break;
		default:
		OLED.print("32 Bit      ");
		break;
	}
}

void GUI::printDeemphFilterSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("DMP: ");
	switch (sabreDAC.Config[sabreDAC.SelectedInput].DE_EMPHASIS_SELECT)
	{
		case f32kHz:
		OLED.print("32 kHz      ");
		break;
		case f48kHz:
		OLED.print("48 kHz      ");
		break;
		case f44_1kHz:
		OLED.print("44.1 kHz    ");
		break;
		default:
		OLED.print("Auto Deemph ");
		break;
	}
}

void GUI::setInputNameCursor( uint8_t value )
{
	cli();
	
	if (this->CursorPosition != 0 && value != 0)
	{
		printSelectedChar();
	}
	Helper::SetPointerValue(value, &CursorPosition, INPUT_NAME_SIZE, 0);
	OLED.setCursor(5 + this->CursorPosition, 1);
	if (this->CursorPosition > 0 && this->TimerEnabled == false)
	{
		this->TimerEnabled = true;
		printLockSymbol(0, 1);
		
		//set timer1 interrupt at 1Hz
		startTimer();
	}
	else if (this->CursorPosition == 0 && this->TimerEnabled)
	{
		OLED.write(0x20);
		printPointer();
		stopTimer(); // stop timer and
	}
	sei();
}


void GUI::printSelectedChar()
{
	cli();
	OLED.setCursor(5 + this->CursorPosition, 1);
	OLED.write(sabreDAC.Config[sabreDAC.SelectedInput % NUMBER_OF_INPUTS].INPUT_NAME[this->CursorPosition -1]);
	sei();
}

bool GUI::EditMode()
{
	return this->CursorPosition > 0;
}

void GUI::printNextChar()
{
	cli();
	OLED.setCursor(5 + this->CursorPosition, 1);
	uint8_t i = sabreDAC.Config[sabreDAC.SelectedInput % NUMBER_OF_INPUTS].INPUT_NAME[this->CursorPosition -1] + 1;
	if (i == 0x80) i = 0x20;
	sabreDAC.Config[sabreDAC.SelectedInput % NUMBER_OF_INPUTS].INPUT_NAME[this->CursorPosition -1] = i;
	OLED.write(i);
	sei();
}

void GUI::PrintPreviousChar()
{
	cli();
	OLED.setCursor(5 + this->CursorPosition, 1);
	uint8_t i = sabreDAC.Config[sabreDAC.SelectedInput % NUMBER_OF_INPUTS].INPUT_NAME[this->CursorPosition -1] - 1;
	if (i == 0x1F) i = 0x7F;
	sabreDAC.Config[sabreDAC.SelectedInput % NUMBER_OF_INPUTS].INPUT_NAME[this->CursorPosition -1] = i;
	OLED.write(i);
	sei();
}

void GUI::stopTimer()
{
	cli();
	TCCR1A = 0;	// set TCCR1A register to 0
	TCCR1B = 0;	// same for TCCR1B
	sei();
	this->TimerEnabled = false;
}

void GUI::printMainMenu()
{
	PrepareForMenuPrinting();					// defines new custom chars and clears the display
	OLED.setCursor(0, 0);
	printTitleBar("MENU");
	printSelectedMainMenuSetting(this->SelectedMenuSetting);
	printPointer();
	NoDisplay = false;
	this->GUI_State = MainMenu;
}

void GUI::printSelectedMainMenuSetting(uint8_t value)
{
	Helper::SetPointerValue(value, &this->SelectedMenuSetting, DefaultAttnu, 0);	// set the correct value for the setting variable
	
	uint8_t row = 1;
	uint8_t col = 2;
	
	switch (this->SelectedMenuSetting)
	{
		case DisplayAutoOff:
		printDisplayAutoOffSetting(col, row);
		break;
		case DisplayAutoOffTime:
		printDisplayAutoOffTimeSetting(col, row);
		break;
		case DefaultAttnu:
		printDefaultAttnuSetting(col, row);
		break;
		case ShowAutoClock:
		printShowAutoClockSetting(col, row);
		break;
		case ShowAutoClockTime:
		printAutoClockTimeSetting(col, row);
		break;
		case AdjustTime:
		printAdjustTimeSetting(col, row);
		break;
		case AdjustDate:
		printAdjustDateSetting(col, row);
		break;
		default:
		// do nothing
		break;
	}
}

void GUI::printDisplayAutoOffSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("Auto Display Off: ");
	printEnabledSetting(MainMenuSettings.displayAutoOff, col + 2, row + 1);
}

void GUI::printDisplayAutoOffTimeSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("Display Off Time:");
	OLED.setCursor(col + 2, row + 1);
	OLED.print(MainMenuSettings.displayAutoOffTime);
	OLED.print(" sec     ");
}

void GUI::printShowAutoClockSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("Auto Clock:      ");
	printEnabledSetting(MainMenuSettings.showAutoClock, col + 2, row + 1);
}

void GUI::printDefaultAttnuSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("Default volume:  ");
	OLED.setCursor(col + 2, row + 1);
	OLED.write(0x2D); // minus
	OLED.print(MainMenuSettings.defaultAttnu);
	OLED.print(" dB      ");
}

void GUI::printAutoClockTimeSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("Clock Time:      ");
	OLED.setCursor(col + 2, row + 1);
	OLED.print(MainMenuSettings.showAutoClockTime);
	OLED.print(" sec     ");
}

void GUI::printAdjustTimeSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("Adjust Time:     ");
	OLED.setCursor(col + 2, row + 1);
	DateTime now = rtc.now();
	Hour = now.hour();
	Minute = now.minute();
	printHour();
	OLED.write(0x3A);
	printMinute();
	// fill row with blanks
	for (uint8_t i = 0; i < 5; i++)
	{
		OLED.write(0x20);
	}
	
}

void GUI::printAdjustDateSetting(uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	OLED.print("Adjust Date:     ");
	OLED.setCursor(col + 2, row + 1);
	DateTime now = rtc.now();
	Day = now.day();
	Month = now.month();
	Year = now.year();
	OLED.print(Day);
	OLED.write(0x2D);
	OLED.print(Month);
	OLED.write(0x2D);
	OLED.print(Year);
}

void GUI::printPointer()
{
	OLED.setCursor(0, 1);
	OLED.write(0x7E);							// Print arrow to point out the selected menu or setting
}

void GUI::printEnabledSetting(bool enabled, uint8_t col, uint8_t row)
{
	OLED.setCursor(col, row);
	if (enabled)
	{
		OLED.print("Enabled   ");
	}
	else
	{
		OLED.print("Disabled  ");
	}
}

// toggles between the display states On/Off
void GUI::toggleDisplay()
{
	
	if (this->NoDisplay)
	{
		switch (GUI_State)
		{
			case HomeScreen:
			printHomeScreen(this->sabreDAC.SelectedInput, this->sabreDAC.Attenuation);
			break;
			case InputSettingsMenu:
			printInputSettingsMenu(sabreDAC.SelectedInput);
			break;
			case MainMenu:
			printMainMenu();
			break;
			default:
			/* Your code here */
			break;
		}
		this->NoDisplay = false;
	}
	else
	{
		OLED.clear();
		this->NoDisplay = true;
	}
}


// prints the time in large numbers
void GUI::printLargeTime()
{
	DateTime now = rtc.now();
	OLED.printLargeNumber(now.hour(), 1, 1);
	OLED.setCursor(9,1);
	OLED.write(0x2E);
	OLED.setCursor(9,3);
	OLED.write(0X2E);
	OLED.printLargeNumber(now.minute(), 11, 1);
}

void GUI::setTimeCursor( uint8_t value )
{
	cli();
	
	if (this->CursorPosition != 0 && value != 0)
	{
		if (CursorPosition == 1)
		{
			printHour();
		}
		else
		{
			printMinute();
		}
	}
	
	Helper::SetPointerValue(value, &CursorPosition, 2, 0);
	
	if (this->CursorPosition > 0 && this->TimerEnabled == false)
	{
		this->TimerEnabled = true;
		printLockSymbol(0, 1);
		
		//set timer1 interrupt at 1Hz
		startTimer();
	}
	else if (this->CursorPosition == 0 && this->TimerEnabled)
	{
		printPointer();
		stopTimer(); // stop timer
		saveDateTime();
	}

	if (CursorPosition == 1)
	{
		OLED.setCursor(4, 2);
		if (value == 0)
		{
			OLED.write(0x5F);
			OLED.write(0x5F);
		}
	}
	else if (CursorPosition == 2)
	{
		OLED.setCursor(7, 2);
		if (value == 0)
		{
			OLED.write(0x5F);
			OLED.write(0x5F);
		}
	}
	
	sei();
}

//set timer1 interrupt at 1Hz
void GUI::startTimer()
{
	//set timer1 interrupt at 1Hz
	TCCR1A = 0;// set entire TCCR1A register to 0
	TCCR1B = 0;// same for TCCR1B
	TCNT1  = 0;//initialize counter value to 0
	// set compare match register for 1hz increments
	OCR1A =  7812; // = (16*10^6) / (0.5*1024) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS12 and CS10 bits for 1024 prescaler
	TCCR1B |= (1 << CS12) | (1 << CS10);
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
}

void GUI::setAndPrintHour( uint8_t value )
{
	cli();
	Helper::SetPointerValue(value, &Hour, 23, 0);
	printHour();
	sei();
}

void GUI::setAndPrintMinute( uint8_t value )
{
	cli();
	Helper::SetPointerValue(value, &Minute, 59, 0);
	printMinute();
	sei();
}

void GUI::printHour()
{
	OLED.setCursor(4, 2);
	if (Hour < 10)
	{
		OLED.write(0x30);
	}
	OLED.print(Hour);
}

void GUI::printMinute()
{
	OLED.setCursor(7, 2);
	if (Minute < 10)
	{
		OLED.write(0x30);
	}
	OLED.print(Minute);
}

void GUI::setDateCursor( uint8_t value )
{
	cli();
	
	if (this->CursorPosition != 0 && value != 0)
	{
		if (CursorPosition == 1)
		{
			printDay();
		}
		else if (CursorPosition == 2)
		{
			printMonth();
		}
		else
		{
			printYear();
		}
	}
	
	Helper::SetPointerValue(value, &CursorPosition, 3, 0);
	
	if (this->CursorPosition > 0 && this->TimerEnabled == false)
	{
		this->TimerEnabled = true;
		printLockSymbol(0, 1);
		
		//set timer1 interrupt at 1Hz
		startTimer();
	}
	else if (this->CursorPosition == 0 && this->TimerEnabled)
	{
		OLED.write(0x20);
		printPointer();
		stopTimer(); // stop timer
		saveDateTime();
	}

	if (CursorPosition == 1)
	{
		OLED.setCursor(4, 2);
		if (value == 0)
		{
			OLED.write(0x5F);
			OLED.write(0x5F);
		}
	}
	else if (CursorPosition == 2)
	{
		OLED.setCursor(7, 2);
		if (value == 0)
		{
			OLED.write(0x5F);
			OLED.write(0x5F);
		}
	}
	else if (CursorPosition == 3)
	{
		OLED.setCursor(10, 2);
		if (value == 0)
		{
			for (uint8_t i = 0; i < 4; i++)
			{
				OLED.write(0x5F);
			}
		}
	}
	
	sei();
}


void GUI::setAndPrintDay( uint8_t value )
{
	cli();
	// get the maximal number of days in the month
	uint8_t maxValue = DateTime::daysInMonth(Month - 1);
	//// correct it when the year is a leap year
	if (Month == 2)
	{
		if (DateTime::isLeapYear(Year))
		{
			maxValue += 1;
		}
	}
	Helper::SetPointerValue(value, &Day, maxValue, 1);
	
	printDay();
	sei();
}

void GUI::printDay()
{
	OLED.setCursor(4, 2);
	if (Day < 10)
	{
		OLED.write(0x30);
	}
	OLED.print(Day);
}

void GUI::setAndPrintMonth( uint8_t value )
{
	cli();
	Helper::SetPointerValue(value, &Month, 12, 1);
	printMonth();
	sei();
}

void GUI::printMonth()
{
	OLED.setCursor(7, 2);
	if (Month < 10)
	{
		OLED.write(0x30);
	}
	OLED.print(Month);
}

void GUI::setAndPrintYear( uint8_t value )
{
	cli();
	uint8_t tempYear = Year - 2000;
	Helper::SetPointerValue(value, &tempYear, 99, 1);
	Year = tempYear + 2000;
	printYear();
	sei();
}

void GUI::printYear()
{
	OLED.setCursor(10, 2);
	OLED.print(Year);
}

void GUI::saveDateTime()
{
	rtc.adjust(DateTime(Year, Month, Day, Hour, Minute, 0)); // set the date and time
	
}