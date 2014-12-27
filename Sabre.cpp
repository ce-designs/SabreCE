/*
* Sabre.cpp
*
* Created: 19-12-2013 23:05:29
* Author: CE-Designs
*/

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "Sabre.h"
#include "Wire.h"
#include "EEPROM.h"
#include "EEPROM_Anything.h"
#include "Helper.h"

// default constructor
Sabre::Sabre()
{
	WIRE.begin();
} //Sabre


//PUBLIC FUNCTIONS////////////////////////////////////////////////////////

void Sabre::begin(bool DualMono, uint8_t F_crystal)
{
	
	this->Reg17.All_Mono = DualMono;		// Set flag, so all DACs are muted as early as possible
	this->sReg10 = 0xCE;					// use default = 8'b11001110
	muteDACS();								// mute DAC's as early as possible
	this->Fcrystal = F_crystal;				// store the crystal frequency (MHz)
	// Set all register defaults
	setRegisterDefaults();
	// Set output mode
	if (this->Reg17.All_Mono)
	{
		setOutputMode(AllMonoMode);
	}
	else
	{
		setOutputMode(NormalMode);
	}
	
	this->SelectedInput = EEPROM.read(EEPROM_SELECTED_INPUT);
	this->DefaultAttenuation = EEPROM.read(EEPROM_DEF_ATTNU);
	this->Attenuation = DefaultAttenuation;
	
	if (Helper::firstRun(EEPROM_SABRE_FIRST_RUN))
	{
		resetInputNames();
		applyDefaultSettings();
		for (uint8_t i = 0; i < NUMBER_OF_INPUTS; i++)
		{
			writeInputConfiguration(i);
		}
		writeDefaultAttenuation();
		EEPROM.write(EEPROM_SABRE_FIRST_RUN, FIRST_RUN); // write flag to EEPROM
	}
	else
	{
		// read all input settings from the EEPROM
		readInputConfiguration();
	}
	
	// set the volume
	setVolume(this->Attenuation);
	// apply the settings for the last known selected input
	applyInputConfiguration(this->SelectedInput);
	unMuteDACS();
}

void Sabre::setRegisterDefaults()
{
	sReg8 = 0x68;						// Use either I2S or DSD input
	sReg11 = 0x85;						// Lowest DPLL bandwidth and De-emphasis delect is 44.1kHz (8'b10000101)
	sReg13 = 0x00;						// DAC polarity set to In-Phase (8'b00000000)
	sReg12 = 0x20;						// Apply dither control, External input, no remap
	sReg14 = 0x0B;						// DAC3/4/7/8 Source, 50k IIR Bandwidth, fast FIR Rolloff (8'b10001011)
	sReg15 = 0x00;						// 6 Bits quantizer (8'b00000000)
	sReg16 = 0x00;						// Do not ramp volume down upon automute condition (8'b00000000)
	sReg17 = 0x1C;						// Use left channel in all_mono mode, use OSF filter, normal dpll_lock_rst_reg, use auto_deemph, spdif_autodetect, Fir_length=28, no fin_phase_flip, normal 8 channel mode (8'b00011100)
	sReg18 = 0x01;						// SPDIF to input #1
	sReg19 = 0x00;						// DACB polarity is set to Anti-Phase
	sReg25 = 0x00;						// Allow all settings (8'b00000000)
}

void Sabre::applyDefaultSettings()
{
	for (byte i = 0; i < NUMBER_OF_INPUTS; i++)
	{
		Config[i].AUTO_DEEMPH = useAutoDeemph;
		Config[i].BIT_MODE = BitMode32;
		Config[i].DE_EMPHASIS_SELECT = AutoDeemphasis;
		Config[i].DEEMPH_FILTER = BypassDeemphasisFilter;
		Config[i].DIFFERENTIAL_MODE = trueDiff;
		Config[i].DPLL_BANDWIDTH = DPLL_Best;
		Config[i].DPLL_BW_128X = UseDPLLBandwidthSetting;
		Config[i].DPLL_BW_DEFAULTS = UseBestDPLLSettings;
		Config[i].FIR_FILTER = FastRolloff;
		Config[i].IIR_BANDWIDTH = use50K;
		Config[i].INPUT_ENABLED = 0x01;
		Config[i].JITTER_REDUCTION = UseJitterReduction;
		Config[i].NOTCH_DELAY = NoNotch;
		Config[i].OSF_FILTER = useOSFfiler;
		Config[i].QUANTIZER = use6BitsTrue;
		Config[i].SERIAL_DATA_MODE = I2S;
		Config[i].SPDIF_ENABLE = useI2SorDSD;
		Config[i].SPDIF_SOURCE = Data1;
		//
		DefaultAttenuation = DEFAULT_ATTNU;
		Attenuation = DefaultAttenuation;
	}
}

void Sabre::resetInputNames()
{
	for (byte i = 0; i < NUMBER_OF_INPUTS; i++)
	{
		Config[i].INPUT_NAME[0] = 'I';
		Config[i].INPUT_NAME[1] = 'N';
		Config[i].INPUT_NAME[2] = 'P';
		Config[i].INPUT_NAME[3] = 'U';
		Config[i].INPUT_NAME[4] = 'T';
		Config[i].INPUT_NAME[5] = ' ';
		Config[i].INPUT_NAME[6] = (i+49);
		for (uint8_t x = 7; x < INPUT_NAME_SIZE; x++)
		{
			Config[i].INPUT_NAME[x] = ' ';
		}
	}
}

void Sabre::setVolume(uint8_t value)
{
	writeSabreReg(REG0, value);			// set volume of DAC0
	writeSabreReg(REG1, value);			// set volume of DAC1
	writeSabreReg(REG2, value);			// set volume of DAC2
	writeSabreReg(REG3, value);			// set volume of DAC3
	writeSabreReg(REG4, value);			// set volume of DAC4
	writeSabreReg(REG5, value);			// set volume of DAC5
	writeSabreReg(REG6, value);			// set volume of DAC6
	writeSabreReg(REG7, value);			// set volume of DAC7
	Attenuation = value;
}

void Sabre::setSPDIFenable(uint8_t value)
{
	switch (value)
	{
		case useSPDIF:
		bitSet(sReg8, 7);					// Use SPDIF input (8b'11101000)
		break;
		default:
		bitClear(sReg8, 7);					// Use I2S or DSD input (8b'01101000)
		break;
	}
	writeSabreReg(REG8, sReg8);
}

void Sabre::setAutomuteTime(uint8_t value)
{
	writeSabreReg(REG9, value);			// time in seconds = 2096896/(value*DATA_CLK)S
}


void Sabre::setBitMode(uint8_t value)
{
	switch (value)
	{
		case BitMode16:					// Set to 16 bit (Serial Data Mode)
		bitClear(sReg10, 6);
		bitSet(sReg10, 7);
		break;
		case BitMode20:					// Set to 20 bit (Serial Data Mode)
		bitSet(sReg10, 6);
		bitClear(sReg10, 7);
		break;
		case BitMode24:					// Set to 24 bit (Serial Data Mode)
		bitClear(sReg10,6);
		bitClear(sReg10,7);
		break;
		default:					// (Default) Set to 32 bit
		bitSet(sReg10,6);
		bitSet(sReg10,7);
		break;
	}
	writeSabreReg(REG10, sReg10);		// write setting to register
}

void Sabre::setSerialDataMode(uint8_t value)
{
	switch (value)
	{
		break;
		case LJ:						// Reg 10: Set to left justified data mode
		bitSet(sReg10, 4);
		bitClear(sReg10, 5);
		break;
		case RJ:						// Reg 10: Set to left justified data mode
		bitClear(sReg10, 4);
		bitSet(sReg10, 5);
		break;
		default:						// Reg 10: (Default) // Set to I2S data mode
		bitClear(sReg10, 4);
		bitClear(sReg10, 5);
	}
	writeSabreReg(REG10, sReg10);		// Reg 10: write setting to register
}


void Sabre::setJitterReductionEnable(uint8_t value)
{
	switch (value)
	{
		case BypassJitterReduction:		// Reg 10: Bypass jitter reduction
		bitClear(sReg10, 2);
		break;
		default:
		bitSet(sReg10, 2);				// Reg 10: enable jitter reduction
		break;
	}
	writeSabreReg(REG10, sReg10);		// Reg 10: Use or bypass JITTER_DEDUCTION
}


void Sabre::setDeemphasisFilter(uint8_t value)
{
	switch (value)
	{
		case UseDeemphasisFilter:		// Reg 10: Use Deemphasis Filter
		bitClear(sReg10, 1);
		break;
		default:						// Reg 10: Bypass Deemphasis Filter
		bitSet(sReg10, 1);
		break;
	}
	writeSabreReg(REG10, sReg10);		// REg 10: Write setting to register
}

void Sabre::unMuteDACS()
{
	bitClear(sReg10, 0);				// Clear bit zero of reg 10: unmute DACs (1'b1)
	writeSabreReg(REG10, sReg10);		// Unmute DACs
	this->Mute = false;
}

void Sabre::muteDACS()
{
	bitSet(sReg10, 0);					// Set bit zero for reg 10: Mute DACs (1'b1)
	writeSabreReg(REG10, sReg10);		// Mute DACs.
	this->Mute = true;
}

void Sabre::setDPLLbandwidth(uint8_t value)
{
	if (value == DPLL_Best)
	{
		setDPLLbandwidthDefaults(UseBestDPLLSettings);
	}
	else
	{
		setDPLLbandwidthDefaults(AllowAllDPLLSettings);
	}
	switch (value)
	{
		case DPLL_NoBandwidth:			// Reg 11: Prepare byte for no bandwidth setting
		bitClear(sReg11, 2);
		bitClear(sReg11, 3);
		bitClear(sReg11, 4);
		break;
		case DPLL_Lowest:				// Reg 11: Prepare byte for lowest bandwidth setting
		bitSet(sReg11, 2);
		bitClear(sReg11, 3);
		bitClear(sReg11, 4);
		break;
		case DPLL_Low:					// Reg 11: Prepare byte for low bandwidth setting
		bitClear(sReg11, 2);
		bitSet(sReg11, 3);
		bitClear(sReg11, 4);
		break;
		case DPLL_MedLow:				// Reg 11: Prepare byte for medium-low bandwidth setting
		bitSet(sReg11, 2);
		bitSet(sReg11, 3);
		bitClear(sReg11, 4);
		break;
		case DPLL_Medium:				// Reg 11: Prepare byte for medium bandwidth setting
		bitClear(sReg11, 2);
		bitClear(sReg11, 3);
		bitSet(sReg11, 4);
		break;
		case DPLL_MedHigh:				// Reg 11: Prepare byte for medium-high bandwidth setting
		bitSet(sReg11, 2);
		bitClear(sReg11, 3);
		bitSet(sReg11, 4);
		break;
		case DPLL_High:					// Reg 11: Prepare byte for high bandwidth setting
		bitClear(sReg11, 2);
		bitSet(sReg11, 3);
		bitSet(sReg11, 4);
		break;
		case DPLL_Highest:				// Reg 11: Prepare byte for highest bandwidth setting
		bitSet(sReg11, 2);
		bitSet(sReg11, 3);
		bitSet(sReg11, 4);
		break;
		default:						// Reg 11: (default) Prepare byte for Best DPLL bandwidth setting
		bitSet(sReg11, 2);
		bitClear(sReg11, 3);
		bitClear(sReg11, 4);
		break;
	}
	writeSabreReg(REG11, sReg11);		// Reg 11: Set DPLL bandwidth
}

void Sabre::setDeEmphasisSelect(uint8_t value)
{
	switch (value)
	{
		case f32kHz:								// Reg 11: De-emphasis select 32 kHz
		setAuto_deemphasis(bypassAutoDeemph);		// disable auto deemphasis filter
		setDeemphasisFilter(UseDeemphasisFilter);	// Use Deemphasis Filter
		bitClear(sReg11, 0);
		bitClear(sReg11, 1);
		break;
		case f48kHz:								// Reg 11: De-emphasis select 48 kHz
		setAuto_deemphasis(bypassAutoDeemph);		// disable auto deemphasis filter
		setDeemphasisFilter(UseDeemphasisFilter);	// Use Deemphasis Filter
		bitClear(sReg11, 0);
		bitSet(sReg11, 1);
		break;
		case f44_1kHz:								// Reg 11: De-emphasis select 44.1 kHz
		setAuto_deemphasis(bypassAutoDeemph);		// disable auto deemphasis filter
		setDeemphasisFilter(UseDeemphasisFilter);	// Use Deemphasis Filter
		bitSet(sReg11, 0);
		bitClear(sReg11, 1);
		break;
		default:									// (default) Auto De-emphasis select
		bitSet(sReg11, 0);
		bitClear(sReg11, 1);
		writeSabreReg(REG11, sReg11);					// Reg 11: Set De-emphasis select
		setDeemphasisFilter(BypassDeemphasisFilter);	// set to default
		setAuto_deemphasis(useAutoDeemph);				// enable auto deemphasis filter
		break;
	}
	if (value != AutoDeemphasis)
	{
		writeSabreReg(REG11, sReg11);		// Reg 11: Set De-emphasis select
	}
}

void Sabre::setNotchDelay(uint8_t value)
{
	switch (value)
	{
		case MCLK4:						// Reg 12: set notch delay to: MCLK/4
		sReg12 = 0x21;
		break;
		case MCLK8:						// Reg 12: set notch delay to: MCLK/8
		sReg12 = 0x23;
		break;
		case MCLK16:					// Reg 12: set notch delay to: MCLK/16
		sReg12 = 0x27;
		break;
		case MCLK32:					// Reg 12: set notch delay to: MCLK/32
		sReg12 = 0x2F;
		break;
		case MCLK64:					// Reg 12: set notch delay to: MCLK/64
		sReg12 = 0x3F;
		break;
		default:						// Reg 12: set notch delay to: No notch
		sReg12 = 0x20;
		break;
	}
	writeSabreReg(REG12, sReg12);		// Reg 12: Write notch-delay setting to register
}

void Sabre::setDacPolarity(uint8_t value)
{
	if (value == TpaPhaseDualMonoOnly && dualMono)
	{
		writeReg(0x48, REG13, 0x22);	// MONO LEFT DACx: odd dacs=in-phase; even dacs=anti-phase
		writeReg(0x49, REG13, 0x22);	// MONO RIGHT DACx: odd dacs=anti-phase; even dacs=in-phase
		return;
	}
	switch (value)
	{
		case AntiPhase:
		sReg13 = 0xFF;					// Reg 13: Set polarity of all DACs to Anti-Phase
		break;
		default:
		sReg13 = 0x00;					// Reg 13: (Default) Set polarity of all DACs to In-Phase
		break;
	}
	writeSabreReg(REG13, sReg13);		//Reg 13: Write settings to register
}

void Sabre::setSourceOfDACs(uint8_t dac8source, uint8_t dac7source, uint8_t dac4source, uint8_t dac3source)
{
	setSourceOfDAC8(dac8source);		// Reg 14: prepare the register for DAC8
	setSourceOfDAC7(dac7source);		// Reg 14: prepare the register for DAC7
	setSourceOfDAC4(dac4source);		// Reg 14: prepare the register for DAC4
	setSourceOfDAC3(dac3source);		// Reg 14: prepare the register for DAC3
	writeSabreReg(REG14, sReg14);		// Reg 14: Write source settings to register
}

void Sabre::setIIRbandwidth(uint8_t value)
{
	switch (value)
	{
		case normalIIR:					// Reg 14: Set IRR bandwidth to normal (47K) (PCM)
		bitClear(sReg14, 1);
		bitClear(sReg14, 2);
		break;
		case use60K:					// Reg 14: Set IRR bandwidth to 60K (DSD)
		bitClear(sReg14, 1);
		bitSet(sReg14, 2);
		break;
		case use70K:					// Reg 14: Set IRR bandwidth to 70K (DSD)
		bitSet(sReg14, 1);
		bitSet(sReg14, 2);
		break;
		default:						// Reg 14: (Default) Set IRR bandwidth to 50K (DSD)
		bitSet(sReg14,1);
		bitClear(sReg14,2);
	}
	writeSabreReg(REG14, sReg14);		// Reg 14: Write IIR bandwidth setting to register
}

void Sabre::setFIRrolloffSpeed(uint8_t value)
{
	switch (value)
	{
		case SlowRolloff:				// Reg 14: Set FIR rolloff speed to: Slow Rolloff
		bitClear(sReg14, 0);
		break;
		default:
		bitSet(sReg14, 0);				// Reg 14: (Default) Set FIR rolloff speed to: Fast Rolloff
		break;
	}
	writeSabreReg(REG14, sReg14);		// Reg 14: Write FIR rolloff speed setting to register
}

void Sabre::setQuantizer(uint8_t value)
{
	switch (value)
	{
		case use7BitsTrue:
		setDifferentialMode(trueDiff);	// use true differential mode
		sReg15 = 0x55;					// 7-bit quantizer
		break;
		case use7BitsPseudo:
		setDifferentialMode(pseudoDiff);// use pseudo differential mode
		sReg15 = 0x55;					// 7-bit quantizer
		break;
		case use8BitsTrue:
		setDifferentialMode(trueDiff);	// use true differential mode
		sReg15 = 0xAA;					// 8-bit quantizer
		break;
		case use8BitsPseudo:
		setDifferentialMode(pseudoDiff);// use pseudo differential mode
		sReg15 = 0xAA;					// 8-bit quantizer
		break;
		case use9BitsPseudo:
		setDifferentialMode(pseudoDiff);// only pseudo differential mode with the 9 bits quantizer
		sReg15 = 0xFF;					// 9-bit quantizer
		break;
		default:
		setDifferentialMode(trueDiff);	// (default) only true differential mode with the 6 bits quantizer
		sReg15 = 0x00;					// (default) 6-bit quantizer
		break;
	}
	writeSabreReg(REG15, sReg15);		// Reg 15: Write quantizer settings to register
}

void Sabre::setMonoChSelect(uint8_t value)
{
	if (value > useRightChannelInAllMonoMode)
	{
		value %= (useRightChannelInAllMonoMode + 1);
	}
	switch (value)
	{
		case useRightChannelInAllMonoMode:
		bitSet(sReg17, 7);
		Reg17.Use_Left_Channel = false;
		break;
		default:
		bitClear(sReg17, 7);
		Reg17.Use_Left_Channel = true;
		break;
	}
	writeSabreReg(REG17, sReg17);		// Reg 17: Write register for all_mono_mode
}


void Sabre::setOSFfilter(uint8_t value)
{
	switch (value)
	{
		case bypassOSFfilter:
		bitSet(sReg17, 6);				// Reg 17: Don't use the OSF filter
		Reg17.OSF_Bypass = true;
		break;
		default:
		bitClear(sReg17, 6);			// Reg 17: (Default) Use the OSF filter
		Reg17.OSF_Bypass = false;
		break;
	}
	writeSabreReg(REG17, sReg17);		// Reg 17: Write setting to register
}

void Sabre::setAuto_deemphasis(uint8_t value)
{
	switch (value)
	{
		case bypassAutoDeemph:
		bitClear(sReg17, 4);				// Reg 17: De-emphasis filter is not automatically applied
		Reg17.Auto_Deemph = false;
		break;
		default:
		bitSet(sReg17, 4);					// Reg 17: (Default) De-emphasis filter is automatically applied with the correct (SPDIF) frequencies
		Reg17.Auto_Deemph = true;
		break;
	}
	writeSabreReg(REG17, sReg17);			// Reg 17: Write setting to register
}

void Sabre::setSPDIFAutoDetect(uint8_t value)
{
	switch (value)
	{
		case manuallySelectSPDIF:
		bitClear(sReg17, 3);			// Reg 17: Must manually detect SPDIF input
		Reg17.SPDIF_Autodetect = false;
		break;
		default:
		bitSet(sReg17, 3);				// Reg 17: Automatically detect SPDIF
		Reg17.SPDIF_Autodetect = true;
		break;
	}
	writeSabreReg(REG17, sReg17);		// Reg 17: write setting to register
}

void Sabre::setFIRLength(uint8_t value)
{
	switch (value)
	{
		case use27Coefficients:		// Reg 17: 2nd stage FIR filter is 27 coefficients in length
		bitClear(sReg17, 2);
		break;
		default:
		bitSet(sReg17, 2);			// Reg 17: 2nd stage FIR filter is 28 coefficients in length
		break;
	}
	writeSabreReg(REG17, sReg17);	// Reg 17: Write setting to register
	Reg17.Fir_Lentgh = value;
}

void Sabre::setFinPhaseFlip(uint8_t value)
{
	switch (value)
	{
		case invertPhase:
		bitSet(sReg17, 1);				// Reg 17: Invert the phase to the DPLL
		Reg17.Fin_Phase_Flip = true;
		break;
		default:
		bitClear(sReg17, 1);			// Reg 17: Do not invert the phase to the DPLL
		Reg17.Fin_Phase_Flip = false;
		break;
	}
	writeSabreReg(REG17, sReg17);
	
}

void Sabre::setOutputMode(uint8_t value)
{
	switch (value)
	{
		case AllMonoMode:				// Reg 17: All 8 DACs are source from one source for true mono
		bitSet(sReg17, 0);
		Reg17.All_Mono = true;
		break;
		default:						// Reg 17: Normal 8 channel mode
		bitClear(sReg17, 0);
		Reg17.All_Mono = false;
		break;
	}
	writeSabreReg(REG17, sReg17);		// Reg 17: Write output mode setting to register
}

void Sabre::setSPDIFsource(uint8_t value)
{
	switch (value)
	{
		case Data2:
		sReg18 = 0x02;					// Set SPDIF to input #2
		break;
		case Data3:
		sReg18 = 0x04;					// Set SPDIF to input #3
		break;
		case Data4:
		sReg18 = 0x08;					// Set SPDIF to input #4
		break;
		case Data5:
		sReg18 = 0x10;					// Set SPDIF to input #5
		break;
		case Data6:
		sReg18 = 0x20;					// Set SPDIF to input #6
		break;
		case Data7:
		sReg18 = 0x40;					// Set SPDIF to input #7
		break;
		case Data8:
		sReg18 = 0x80;					// Set SPDIF to input #8
		break;
		default:
		sReg18 = 0x01;					// (default) Set SPDIF to input #1
		break;
	}
	writeSabreReg(REG18, sReg18);		// Reg 18: Write SPDIF input setting
}

void Sabre::setDaCBpolarity(uint8_t value)
{
	switch (value)
	{
		case InPhase:
		sReg19 = 0xFF;					// Reg 19: Set polarity of all DACBs to In-Phase
		break;
		default:
		sReg19 = 0x00;					// Reg 19: Set polarity of all DACBs to Anti-Phase
		break;
	}
	writeSabreReg(REG19, sReg19);		// Reg 19: Write settings to register
}

void Sabre::setDPLLbandwidthDefaults(uint8_t value)
{
	switch (value)
	{
		case AllowAllDPLLSettings:		// Reg 25: set DPLL mode control to allow all settings
		bitClear(sReg25, 1);
		break;
		default:						// Reg 25: set DPLL mode control use the best DPLL settings
		bitSet(sReg25, 1);
		break;
	}
	writeSabreReg(REG25, sReg25);
	Config[SelectedInput % NUMBER_OF_INPUTS].DPLL_BW_DEFAULTS = value;
}

void Sabre::setDPLLBandwidth128x(uint8_t value)
{
	switch (value)
	{
		case MultiplyDPLLBandwidthBy128:// Reg 25: Multiply DPLL bandwidth setting by 128
		bitSet(sReg25, 0);
		break;
		default:
		bitClear(sReg25, 0);			// Reg 25: Use DPLL Bandwidth Setting
		break;
	}
	writeSabreReg(REG25, sReg25);		// Reg 25: Write setting to register
}

void Sabre::getStatus()
{
	sReg27 = readRegister(REG27);		// Reg27: Read register 27
	if(sReg27&B00001000)				// Reg 27: DSD Mode?
	{
		Status.DSD_Mode = true;
	}
	else
	{
		Status.DSD_Mode = false;
	}
	if(sReg27&B00000100)				// Reg 27: SPDIF valid?
	{
		Status.SPDIF_Valid = true;
	}
	else
	{
		Status.SPDIF_Valid = false;
	}
	if(sReg27&B00000010)				// Reg 27: SPDIF enabled?
	{
		Status.SPDIF_Enabled = true;
	}
	else
	{
		Status.SPDIF_Enabled = false;
	}
	if(sReg27&B00000001)				// Reg 27: Lock?
	{
		Status.Lock = true;
	}
	else
	{
		Status.Lock = false;
	}
}

void Sabre::setSampleRate()
{
	this->SampleRate = calculateSampleRate(getDPLL_NUM());	// calculate sample rate
};

void Sabre::writeInputConfiguration()
{
	writeInputConfiguration(this->SelectedInput);
}

void Sabre::writeInputConfiguration(uint8_t input)
{
	int location = (input) * sizeof(Config[input]);
	EEPROM_writeAnything(location, Config[input]);	// write the input configuration to the EEPROM
}

void Sabre::writeSelectedInput()
{
	EEPROM.write(EEPROM_SELECTED_INPUT , this->SelectedInput);
}

void Sabre::writeDefaultAttenuation()
{
	EEPROM.write(EEPROM_DEF_ATTNU , this->DefaultAttenuation);
}

void Sabre::selectInput(uint8_t value)
{
	applyInputConfiguration(this->SelectedInput);		// Apply the stored configuration for the input.
}
//END PUBLIC FUNCTIONS////////////////////////////////////////////////////


//PRIVATE FUNCTIONS///////////////////////////////////////////////////////
void Sabre::writeSabreReg(uint8_t regAddr, uint8_t value)
{
	writeReg(0x48, regAddr, value);	// Write to DAC with address 0x90 (0x48 for arduino);
	if (Reg17.All_Mono)
	{
		writeReg(0x49, regAddr, value);	// Write to DAC with address 0x92 (0x49 for arduino);
	}
}

void Sabre::writeReg(uint8_t dacAddr, uint8_t regAddr, uint8_t value)
{
	WIRE.beginTransmission(dacAddr);// Begin transmission to Sabre device address
	WIRE.write(regAddr);			// Specifying the address of register
	WIRE.write(value);				// Writing the value into the register
	WIRE.endTransmission();			// End transmission
}

byte Sabre::readRegister(uint8_t value)
{
	WIRE.beginTransmission(0x48);	// Hard coded the Sabre device address
	WIRE.write(value);				// Queues the address of the register
	WIRE.endTransmission();			// Sends the address of the register
	WIRE.requestFrom(0x48, 1);		// Request one byte from address
	if (WIRE.available())			// Wire.available indicates if data is available
	{
		return WIRE.read();			// Wire.read() reads the data on the wire
	}
	else
	{
		return 0;                   // If no data in the wire, then return 0 to indicate error
	}
}

volatile unsigned long Sabre::getDPLL_NUM()
{
	volatile unsigned long DPLL_NUM = 0;
	DPLL_NUM|=readRegister(REG31);
	DPLL_NUM<<=8;
	DPLL_NUM|=readRegister(REG30);
	DPLL_NUM<<=8;
	DPLL_NUM|=readRegister(REG29);
	DPLL_NUM<<=8;
	DPLL_NUM|=readRegister(REG28);
	return DPLL_NUM;
}

unsigned long Sabre::calculateSampleRate(unsigned long DPLL_NUM)
{
	unsigned long i;
	if (Status.SPDIF_Valid)
	{
		i = Fcrystal / 5;
		DPLL_NUM *= i;
		DPLL_NUM /= 859;
	}
	else
	{
		if (Fcrystal==80)
		{
			DPLL_NUM/=3436;		// Calculate SR for I2S -80MHz part
		}
		else if (Fcrystal==100)
		{
			DPLL_NUM*=4;			// Calculate SR for I2S -100MHz part
			DPLL_NUM/=10995;		// Calculate SR for I2S -100MHz part
		}
		else
		{
			i = (64L * 4295L)  / Fcrystal;
			DPLL_NUM /= i;
		}
		
	}
	if (DPLL_NUM < 90000)		// Adjusting because in integer operation, the residual is truncated
	{
		DPLL_NUM+=1;
	}
	else
	{
		if (DPLL_NUM < 190000)
		{
			DPLL_NUM+=2;
		}
		else
		{
			if (DPLL_NUM < 350000)
			{
				DPLL_NUM+=3;
			}
			else
			{
				DPLL_NUM+=4;
			}
		}
	}
	if(this->Reg17.OSF_Bypass)	// When OSF is bypassed, the magnitude of DPLL is reduced by a factor of 64
	{
		DPLL_NUM*=64;
	}
	return DPLL_NUM;			// return the sample rate
}

void Sabre::setSourceOfDAC8(uint8_t value)
{
	switch (value)
	{
		case DAC6:
		bitSet(sReg14, 7);				// Reg 14: DAC6
		break;
		default:
		bitClear(sReg14, 7);			// Reg 14: (Default) DAC8
		break;
	}
}

void Sabre::setSourceOfDAC7(uint8_t value)
{
	switch (value)
	{
		case DAC5:
		bitSet(sReg14, 6);				// Reg 14: DAC5
		break;
		default:
		bitClear(sReg14, 6);			// Reg 14: (Default) DAC7
		break;
	}
}

void Sabre::setSourceOfDAC4(uint8_t value)
{
	switch (value)
	{
		case DAC2:					// Reg 14: DAC2
		bitSet(sReg14, 5);
		break;
		default:
		bitClear(sReg14,5);			// Reg 14: (Default) DAC4
		break;
	}
}

void Sabre::setSourceOfDAC3(uint8_t value)
{
	switch (value)
	{
		case DAC1:					// Reg 14: DAC1
		bitSet(sReg14, 4);
		break;
		default:
		bitClear(sReg14,4);			// Reg 14: (Default) DAC3
		break;
	}
}

void Sabre::setDifferentialMode(uint8_t value)
{
	switch (value)
	{
		case trueDiff:
		bitSet(sReg14, 3);
		break;
		case pseudoDiff:
		bitClear(sReg14, 3);
		break;
	}
	writeSabreReg(REG14, sReg14);		// Reg 14: Write differential mode setting to register
}



void Sabre::readInputConfiguration()
{
	int location = 0;
	for (uint8_t i = 0; i < NUMBER_OF_INPUTS; i++)
	{
		location += EEPROM_readAnything(location, this->Config[i]);
	}
}

void Sabre::applyInputConfiguration(uint8_t input)
{
	if (!Mute)
	{
		muteDACS();
	}
	
	setSPDIFenable(Config[input].SPDIF_ENABLE);
	setSerialDataMode(Config[input].SERIAL_DATA_MODE);
	setBitMode(Config[input].BIT_MODE);
	setSPDIFsource(Config[input].SPDIF_SOURCE);
	
	setDeemphasisFilter(Config[input].DEEMPH_FILTER);
	setDeEmphasisSelect(Config[input].DE_EMPHASIS_SELECT);
	setJitterReductionEnable(Config[input].JITTER_REDUCTION);
	setOSFfilter(Config[input].OSF_FILTER);
	
	setFIRrolloffSpeed(Config[input].FIR_FILTER);
	setIIRbandwidth(Config[input].IIR_BANDWIDTH);
	setQuantizer(Config[input].QUANTIZER);
	setDPLLbandwidth(Config[input].DPLL_BANDWIDTH);
	setDPLLBandwidth128x(Config[input].DPLL_BW_128X);
	setNotchDelay(Config[input].NOTCH_DELAY);
	
	if (Mute)
	{
		unMuteDACS();
	}
}


//END PRIVATE FUNCTIONS///////////////////////////////////////////////////
