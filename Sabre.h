#pragma once
/*
* Sabre.h
*
* Created: 19-12-2013 23:05:30
* Author: CE-Designs
*/


#ifndef __SABRE_H__
#define __SABRE_H__

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include "Wire.h"
#include "EEPROM.h"

#define INPUT_NAME_SIZE 10
#define NUMBER_OF_INPUTS 6
#define SR_LENGTH 8

#define EEPROM_SELECTED_INPUT 400
#define EEPROM_DEF_ATTNU 401
#define EEPROM_FIRST_RUN 402

#define FIRST_RUN 512 		// this is just a flag for indicating that it's not the first run

#define DEFAULT_ATTNU 49  	// 49 decibel attenuation by default
	
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

class Sabre
{
	//variables
	public:

	bool dualMono;
	
	char SampleRateString[SR_LENGTH];
	
	enum SPDIF_Enable
	{
		useI2SorDSD, useSPDIF
	};
	
	enum DPLL
	{
		DPLL_NoBandwidth, DPLL_Lowest, DPLL_Low, DPLL_MedLow, DPLL_Medium,
		DPLL_MedHigh, DPLL_High, DPLL_Highest, DPLL_Best
	};
	
	enum BitModes
	{
		BitMode16, BitMode20, BitMode24, BitMode32
	};
	
	enum serialDataModes
	{
		I2S, LJ, RJ
	};
	
	enum JitterReductionEnable
	{
		UseJitterReduction, BypassJitterReduction
	};
	
	enum DeemphasisFilter
	{
		UseDeemphasisFilter, BypassDeemphasisFilter
	};
	
	enum DeEmphasisDelect
	{
		AutoDeemphasis, f32kHz, f44_1kHz, f48kHz
	};
	
	enum NotchDelay
	{
		NoNotch, MCLK4, MCLK8, MCLK16, MCLK32, MCLK64
	};
	
	enum DacPolarity
	{
		InPhase, AntiPhase, TpaPhaseDualMonoOnly
	};
	
	enum SourceOfDAC
	{
		DAC1, DAC2, DAC3, DAC4, DAC5, DAC6, DAC7, DAC8
	};

	enum DifferentialMode
	{
		trueDiff, pseudoDiff
	};
	
	enum IIR_Bandwidth
	{
		use50K, normalIIR, use60K, use70K
	};
	
	enum FIR_ROLLOFF_SPEED
	{
		FastRolloff, SlowRolloff
	};
	
	enum Quantizer
	{
		use6BitsTrue, use7BitsTrue, use7BitsPseudo,
		use8BitsTrue, use8BitsPseudo, use9BitsPseudo
	};
	
	enum MonoChannelSelect
	{
		useLeftChannelInAllMonoMode, useRightChannelInAllMonoMode
	};
	
	enum OSFfilter
	{
		useOSFfiler, bypassOSFfilter
	};
	
	enum AutoDeemph
	{
		useAutoDeemph, bypassAutoDeemph
	};
	
	enum spdifAutoDetect
	{
		autoDetectSPDIF, manuallySelectSPDIF
	};
	
	enum FIRlength
	{
		use28Coefficients, use27Coefficients
	};
	
	enum FinPhaseFlip
	{
		invertPhase, DoNoInvertPhase
	};
	
	enum OutputMode
	{
		NormalMode, AllMonoMode
	};
	
	enum SPDIFsource
	{
		Data1, Data2, Data3, Data4, Data5, Data6, Data7, Data8
	};
	
	enum dpll_bw_defaults
	{
		UseBestDPLLSettings, AllowAllDPLLSettings
	};
	
	enum dpll_bw_128x
	{
		UseDPLLBandwidthSetting, MultiplyDPLLBandwidthBy128
	};
	
	uint8_t Attenuation;	// hold the current attenuation (volume)
	uint8_t SelectedInput;	// holds the current selected input
	uint8_t DefaultAttenuation;	// Default attenuation at start (volume)
	
	struct config_t	// Holds the configuration of all inputs
	{
		// NON-DAC RELATED INPUT SETTINGS
		char INPUT_NAME[INPUT_NAME_SIZE];
		uint8_t INPUT_ENABLED;
		// DAC SETTINGS FROM THE INPUT
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
		uint8_t	BIT_MODE;			// use16bit, use20bit, use24bit or use 32bit
		uint8_t JITTER_REDUCTION;
		uint8_t DEEMPH_FILTER;
		uint8_t DE_EMPHASIS_SELECT;
		uint8_t OSF_FILTER;
		uint8_t AUTO_DEEMPH;
	} Config[NUMBER_OF_INPUTS + 1];
	
	struct Register17
	{
		bool Use_Left_Channel;
		bool OSF_Bypass;
		bool Auto_Deemph;
		bool SPDIF_Autodetect;
		bool Fin_Phase_Flip;
		bool All_Mono;
		uint8_t Fir_Lentgh;
	}Reg17;

	struct Register27
	{
		bool DSD_Mode;
		bool SPDIF_Valid;
		bool SPDIF_Enabled;
		bool Lock;
	}Status;
		
	
	unsigned long getSampleRate();	// Retrieve DPLL_NUM, calculate sample rate and file the SampleRateString char array
	//void getSampleRateString();	// hold a 8 character string containing the sample rate
	//void setSampleRateString();	// return a 8 character string containing the sample rate
	
	protected:
	
	private:

	unsigned long Fcrystal;	// Clock Frequency (MHz)
	
	uint8_t	sReg8;		// settings for register 8
	uint8_t sReg9;		// settings for register 9
	uint8_t sReg10;		// settings for register 10
	uint8_t sReg11;		// settings for register 11
	uint8_t sReg12;		// settings for register 12
	uint8_t sReg13;		// settings for register 13
	uint8_t sReg14;		// settings for register 14
	uint8_t sReg15;		// settings for register 15
	uint8_t sReg16;		// settings for register 16
	uint8_t sReg17;		// settings for register 17
	uint8_t sReg18;		// settings for register 18
	uint8_t sReg19;		// settings for register 19
	uint8_t sReg25;		// settings for register 25
	uint8_t sReg27;		// value of status register 27

	byte readRegister(uint8_t value);

	unsigned long SampleRate;	// holds the calculated sample rate
	unsigned long getDPLL_NUM();
	unsigned long calculateSampleRate(unsigned long dpllNumm);

	bool firstRun();	// for checking if it is the first run of the controller
	

	//functions
	public:
	
	Sabre();
	
	virtual void begin(bool DualMono, uint8_t F_crystal);
	
	void setVolume(uint8_t value);
	void setSPDIFenable(uint8_t value);
	void setAutomuteTime(uint8_t value);
	void setBitMode(uint8_t value);
	void setSerialDataMode(uint8_t value);
	void setJitterReductionEnable(uint8_t value);
	void setDeemphasisFilter(uint8_t value);	
	void setDPLLbandwidth(uint8_t value);
	void setDeEmphasisSelect(uint8_t value);
	void setNotchDelay(uint8_t value);
	void setDacPolarity(uint8_t value);
	void setSourceOfDACs(uint8_t dac8, uint8_t dac7, uint8_t dac4, uint8_t dac3);
	void setIIRbandwidth(uint8_t value);
	void setFIRrolloffSpeed(uint8_t value);
	void setQuantizer(uint8_t value);
	void setMonoChSelect(uint8_t value);
	void setOSFfilter(uint8_t value);
	void setAuto_deemphasis(uint8_t value);
	void setSPDIFAutoDetect(uint8_t value);
	void setFIRLength(uint8_t value);
	void setFinPhaseFlip(uint8_t value);
	void setOutputMode(uint8_t value);
	void setSPDIFsource(uint8_t value);
	void setDaCBpolarity(uint8_t value);
	void setDPLLbandwidthDefaults(uint8_t value);
	void setDPLLBandwidth128x(uint8_t value);
	
	void getStatus();
	
	void muteDACS();
	void unMuteDACS();	
	
	void writeInputConfiguration(uint8_t input);
	void writeSelectedInput();
	void writeDefaultAttenuation();
	
	void selectInput(uint8_t input);
		

	protected:
	private:
	
	void writeSabreReg(uint8_t regAddr, uint8_t value);
	void writeReg(uint8_t dacAddr, uint8_t regAddr, uint8_t value);
	void setSourceOfDAC8(uint8_t value);
	void setSourceOfDAC7(uint8_t value);
	void setSourceOfDAC4(uint8_t value);
	void setSourceOfDAC3(uint8_t value);
	void setDifferentialMode(uint8_t value);
		
	void readInputConfiguration();
	void applyInputConfiguration(uint8_t input);
	
	void setRegisterDefaults(); 
	void useDefaultSettings();
	
	
	//Sabre( const Sabre &c );
	//Sabre& operator=( const Sabre &c );
	
}; //Sabre

#endif //__SABRE_H__



