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

#pragma region HEADER_FILE_INCLUSIONS

#include "Wire.h"
#include "EEPROM.h"

#pragma endregion HEADER_FILE_INCLUSIONS

#pragma region #DEFINE_DIRECTIVES

#define INPUT_NAME_SIZE 8
#define NUMBER_OF_INPUTS 6
#define SR_LENGTH 8

//// EEPROM LOCATIONS FOR STORING DATA ///

#define EEPROM_SELECTED_INPUT 400
#define EEPROM_DEF_ATTNU 401
#define EEPROM_FIRST_RUN 402

//// END OF EEPROM LOCATIONS ///

#define FIRST_RUN 0x03 		// this is just a flag for indicating that it's not the first run
#define DEFAULT_ATTNU 49  	// 49 decibel attenuation by default

// WIRE FOR I2C		
#define WIRE Wire

//REGISTERS///////////////////////////////////////////////////////////////
#define REG0 0X00	// Volume of DAC0
#define REG1 0X01	// Volume of DAC1
#define REG2 0X02	// Volume of DAC2
#define REG3 0X03	// Volume of DAC3
#define REG4 0X04	// Volume of DAC4
#define REG5 0X05	// Volume of DAC5
#define REG6 0X06	// Volume of DAC6
#define REG7 0X07	// Volume of DAC7
#define REG8 0x08	// Automute_lev
#define REG9 0x09	// Automute_time
#define REG10 0x0A	// Mode Control 1
#define REG11 0x0B	// Mode Control 2
#define REG12 0x0C	// Mode Control 3: Dither control, Notch delay
#define REG13 0x0D	// DAC Polarity
#define REG14 0x0E	// DAC3/4/7/8 Source, Differential Mode, IRR Bandwidth, FIR Rolloff
#define REG15 0x0F	// Mode Control 4: Quantizer settings
#define REG16 0x10	// Automute Loopback
#define REG17 0x11	// Mode Control 5
#define REG18 0x12	// SPDIF Source
#define REG19 0x13	// DACB Polarity
#define REG23 0x17	// >Master Trim (MSB's)
#define REG22 0x16	// >Master Trim
#define REG21 0x15	// >Master Trim
#define REG20 0x14	// >Master Trim (LSB's)
#define REG24 0x18	// Phase Shift
#define REG25 0x19	// DPLL Mode Control
#define REG27 0x1B	// Status
#define REG31 0x1F	// >DPLL_NUM (MSB's)
#define REG30 0x1E	// >DPLL_NUM
#define REG29 0x1D	// >DPLL_NUM
#define REG28 0x1C	// >DPLL_NUM (LSB's)
//END REGISTERS///////////////////////////////////////////////////////////
		
#pragma endregion #DEFINE_DIRECTIVES

class Sabre
{
	public:
		
#pragma region REGISTER ENUMS
	
	enum SPDIF_ENABLE
	{
		useI2SorDSD, useSPDIF
	};

	enum DPLL_BANDWIDTH
	{
		DPLL_NoBandwidth, DPLL_Lowest, DPLL_Low, DPLL_MedLow, DPLL_Medium,
		DPLL_MedHigh, DPLL_High, DPLL_Highest, DPLL_Best
	};

	enum BIT_MODE
	{
		BitMode16, BitMode20, BitMode24, BitMode32
	};

	enum SERIAL_DATA_MODE
	{
		I2S, LJ, RJ
	};

	enum JITTER_REDUCTION_ENABLE
	{
		UseJitterReduction, BypassJitterReduction
	};

	enum DEEMPHASIS_FILTER
	{
		UseDeemphasisFilter, BypassDeemphasisFilter
	};

	enum DEEMPHASIS_SELECT
	{
		AutoDeemphasis, f32kHz, f44_1kHz, f48kHz
	};

	enum NOTCH_DELAY
	{
		NoNotch, MCLK4, MCLK8, MCLK16, MCLK32, MCLK64
	};

	enum DAC_POLARITY
	{
		InPhase, AntiPhase, TpaPhaseDualMonoOnly
	};

	enum SOURCE_OF_DAC
	{
		DAC1, DAC2, DAC3, DAC4, DAC5, DAC6, DAC7, DAC8
	};

	enum DIFFERENTIAL_MODE
	{
		trueDiff, pseudoDiff
	};

	enum IIR_BANDWIDTH
	{
		normalIIR, use50K, use60K, use70K
	};

	enum FIR_ROLLOFF_SPEED
	{
		FastRolloff, SlowRolloff
	};

	enum QUANTIZER
	{
		use6BitsTrue, use7BitsTrue, use7BitsPseudo,
		use8BitsTrue, use8BitsPseudo, use9BitsPseudo
	};

	enum MONO_CHANNEL_SELECT
	{
		useLeftChannelInAllMonoMode, useRightChannelInAllMonoMode
	};

	enum OVERSAMPLING_FILTER
	{
		useOSFfiler, bypassOSFfilter
	};

	enum AUTO_DEEMPHASIS
	{
		useAutoDeemph, bypassAutoDeemph
	};

	enum SPDIF_AUTO_DETECT
	{
		autoDetectSPDIF, manuallySelectSPDIF
	};

	enum FIR_LENGTH
	{
		use28Coefficients, use27Coefficients
	};

	enum FIN_PHASE_FLIP
	{
		invertPhase, DoNotInvertPhase
	};

	enum OUTPUT_MODE
	{
		NormalMode, AllMonoMode
	};

	enum SPDIF_SOURCE
	{
		Data1, Data2, Data3, Data4, Data5, Data6, Data7, Data8
	};

	enum DPLL_BW_DEFAULT
	{
		UseBestDPLLSettings, AllowAllDPLLSettings
	};

	enum DPLL_BW_128X
	{
		UseDPLLBandwidthSetting, MultiplyDPLLBandwidthBy128
	};
	
#pragma endregion REGISTER ENUMS
	
#pragma region PUBLIC VARIABLES

	bool dualMono;
	uint8_t Attenuation;			// hold the current attenuation (volume)
	uint8_t SelectedInput;			// holds the current selected input
	uint8_t DefaultAttenuation;		// Default attenuation at start (volume)
	bool Mute;						// holds the current mute state
	unsigned long getSampleRate();	// Retrieve DPLL_NUM, calculate sample rate and file the SampleRateString char array
	unsigned long SampleRate;		// holds the calculated sample rate
	
#pragma endregion PUBLIC VARIABLES
	
#pragma region STRUCTS

	struct config_t	// Holds the configuration of each of the configured inputs
	{
		// NON DAC-REGISTER RELATED INPUT SETTINGS
		char INPUT_NAME[INPUT_NAME_SIZE];
		uint8_t INPUT_ENABLED;
		// DAC REGISTER SETTINGS OF THE INPUT
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
		uint8_t	BIT_MODE;			
		uint8_t JITTER_REDUCTION;
		uint8_t DEEMPH_FILTER;
		uint8_t DE_EMPHASIS_SELECT;
		uint8_t OSF_FILTER;
		uint8_t AUTO_DEEMPH;
		// END OF DAC SETTINGS
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
			
#pragma endregion STRUCTS
		
	protected:
	
	private:

#pragma region PRIVATE VARIABLES

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
	
	volatile unsigned long getDPLL_NUM();
	unsigned long calculateSampleRate(volatile unsigned long dpllNumm);
	
	bool firstRun();	// for checking if it is the first run of the controller
	bool manual;		// for indicating: manual settings selection or use default settings
	
#pragma endregion PRIVATE VARIABLES
	
	//functions
	public:
	
#pragma region CONTRUCTOR

	Sabre();
	
#pragma endregion CONTRUCTOR
	
#pragma region PUBLIC METHODS
	
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
	
	void applyDefaultSettings();
	
	void getStatus();
	
	void muteDACS();
	void unMuteDACS();	
	
	void writeInputConfiguration();
	void writeInputConfiguration(uint8_t input);
	void writeSelectedInput();
	void writeDefaultAttenuation();
	
	void selectInput(uint8_t input);
	
#pragma endregion PUBLIC METHODS
		

	protected:
	private:
	
#pragma region PRIVATE METHODS
	
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
	void resetInputNames();
	
#pragma endregion PRIVATE METHODS
	
	
}; //Sabre

#endif //__SABRE_H__



