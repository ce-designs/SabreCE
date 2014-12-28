/* 
* Helper.cpp
*
* Created: 27-11-2014 21:30:40
* Author: CE-Designs
*/


#include "Helper.h"
#include "EEPROM.h"

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


void Helper::SetPointerValue( const uint8_t value, uint8_t *pointerValue, const uint8_t maxValue, const uint8_t minValue)
{
	if (value == NEXT)
	{
		if (*pointerValue >= maxValue)
		{
			*pointerValue = minValue;
		}
		else
		{
			*pointerValue += 1; // set to next setting, cursor position or whatever
		}
	}
	else if (value == PREVIOUS)
	{
		if (*pointerValue == minValue)
		{
			*pointerValue = maxValue;
		}
		else
		{
			*pointerValue -= 1; // set to previous setting, cursor position or whatever
		}
	}
}
