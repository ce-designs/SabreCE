/* 
* Helper.h
*
* Created: 27-11-2014 21:30:40
* Author: CE-Designs
*/


#ifndef __HELPER_H__
#define __HELPER_H__

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// FLAGS
#define NEXT 0x01
#define PREVIOUS 0x02

class Helper
{
//variables
public:
protected:
private:

//functions
public:
	
	static void SetPointerValue(const uint8_t value, uint8_t *pointerValue, const uint8_t maxValue, const uint8_t minValue);
	//static bool firstRun(int ee); 	// for checking if it is the first run of the controller
	
protected:
private:

}; //Helper

#endif //__HELPER_H__
