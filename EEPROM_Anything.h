/* 
* EEPROM_Anything.h
*
* Created: 22-11-2014 19:43:57
* Author: CE-Designs
*
* Originated from: http://playground.arduino.cc/Code/EEPROMWriteAnything 
*/


#ifndef __EEPROM_ANYTHING_H__
#define __EEPROM_ANYTHING_H__

#include "EEPROM.h"

// EEPROM Locations 0 to 499 are reserved for the Sabre class
// EEPROM Locations 450 to 454 are preserved for the GUI class

#pragma region EEPROM_ANYTHING_METHODS

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
	const byte* p = (const byte*)(const void*)&value;
	unsigned int i;
	byte currValue;
	
	for (i = 0; i < sizeof(value); i++)
	{
		currValue = EEPROM.read(ee);
		if (currValue != *p)
		{
			EEPROM.write(ee++, *p++);
		}
		else
		{
			ee++;
			*p++;
		}
	}
	return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
	byte* p = (byte*)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
	{
		*p++ = EEPROM.read(ee++);
	}
	return i;
}

#pragma endregion EEPROM_ANYTHING_METHODS

#endif //__EEPROM_ANYTHING_H__
