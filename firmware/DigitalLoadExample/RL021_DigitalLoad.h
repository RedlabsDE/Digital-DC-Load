/**
* \file    RL021_DigitalLoad.h
* \brief    Control of Digital Constant Current Source (DAC + NFET) with feedback (ADC)
* \brief    Required hardware: PCB RL-021/xx, I2C communication (via MCU / USB bridge / ...)    
* \brief    Required drivers: MCP47x6.h, MCP3428.h
* 
* \brief    basic functions: 
*               set constant load current [mA] 
*               measure actual load current [mA]
*               measure actual load voltage [mV]
*               measure actual external voltage [mV]
*               measure actual NTC temperature [°C x10]
* 
*               set calibration values for used PCB
* 
* \author  Julian Schindler
*
* \par     Editor 
*
* \date    20.09.2020 first implementation
*
* \todo
* \version V0.1
*/

#ifndef _RL021_DigitalLoad_H_
#define _RL021_DigitalLoad_H_

#include <stdio.h>
#include <stdint.h>

// Use the following define for code tests without required hardware (ADC and DAC is simulated)
//#define MOCK_ADC_DAC

//////////////////////////////////////////////////////
#ifndef MOCK_ADC_DAC

	/// I2C
	//#include <Wire.h>

	/// DAC
	/// https://github.com/holgerlembke/MCP47x6
	#include "MCP47x6.h"

	/// ADC
	/// This code is designed to work with the MCP3428_I2CADC I2C Mini Module available from ControlEverything.com.
	/// https://www.controleverything.com/content/Analog-Digital-Converters?sku=MCP3428_I2CADC#tabs-0-product_tabset-2
	#include "MCP3428.h"


#else

  #include "MOCK-DAC-ADC.h"
  
#endif
//////////////////////////////////////////////////////


/************************************************************************/
/* Enums                                                                */
/************************************************************************/
typedef enum
{
    JP2_CURRENT,
    JP3_VLOAD,
    JP4_VEXT
    
} E_JUMPER;

typedef enum
{
    Jumper_Open,
    Jumper_Closed
    
} E_JUMPER_STATE;


/// ADC channel order
typedef enum
{
    ADC_CH_CURRENT,
    ADC_CH_VLOAD,
    ADC_CH_VEXT,
    ADC_CH_NTC,
    ADC_CH_LAST
} E_ADC_CHANNEL;

/// highRangeSelected_current
typedef enum
{
    RANGE_DAC_LOW,
    RANGE_DAC_HIGH
    
} E_DAC_RANGE;

/// lowRangeSelected_Vload
/// lowRangeSelected_Vext
typedef enum
{
    RANGE_ADC_HIGH,
    RANGE_ADC_LOW
    
} E_ADC_RANGE;

/************************************************************************/
/* Structs                                                              */
/************************************************************************/
typedef struct
{
    /// ADC - voltage / current measurement calibration
    /// [highrange, lowrange][]
    float slope_adc[2][ADC_CH_LAST];  
    float offset_adc[2][ADC_CH_LAST];

    /// DAC - current set calibration    
    /// [lowrange, highrange]
    float slope_dac[2]; 
    float offset_dac[2];

} S_RL021_Calibration;





/************************************************************************/
/* Class                                                                */
/************************************************************************/
class RL021_DigitalLoad {
    
 public:    
 //private:
 // private:
    ///////////////////////////////////////////////////////////////
    /// Range selection Jumper settings (true: closed, false: open)
    
    /// Current Range (JP2)
    bool highRangeSelected_current;
    /// Vload Range (JP3)
    bool lowRangeSelected_Vload;
    /// Vext Range (JP4)
    bool lowRangeSelected_Vext;

  /// NTC R/T characteristic for NTC Type: ("B57421V2103", R(25°C)=10kOhm, B_25/100=4000K
  float NTC_TABLE_B57421V2103[42] = {96.158, 66.892, 47.127, 33.606, 24.243, //-55 -> -35
                                 17.681, 13.032, 9.702, 7.2923, 5.5314, //- 30 -> -10
                                4.2325, 3.2657, 2.54, 1.9907, 1.5716, //-5 -> 15
                                1.2494, 1.0000, 0.80552, 0.65288,0.53229, // 20 -> 40
                                0.43645, 0.35981, 0.29819, 0.24837,0.20787, // 45 -> 65
                                0.17479, 0.14763, 0.12523, 0.10667, 0.091227, // 70 -> 90 
                                0.078319, 0.067488, 0.058363, 0.050647, 0.044098, //95 -> 115
                                0.03852, 0.033752, 0.029663, 0.026146, 0.023111, //120 -> 140
                                0.020484, 0.018203}; // 145 -> 150
    
    ///////////////////////////////////////////////////////////////
    /// Values for transfer function ( DAC -> Current )
    S_RL021_Calibration calibrationData;
    
    /// Use default calibration data
    void SetDefaultCalibration();
    
    ///////////////////////////////////////////////////////////////
    /// Pointer to used DAC Device (MCP47x6)
    MCP47x6base * deviceDAC;
    
    /// Pointer to used ADC Device (MCP3428)
    MCP3428 * deviceADC;
       
    ///////////////////////////////////////////////////////////////
    /// Calculate raw DAC register value from desired current 
    uint16_t CalculateDAC(uint16_t current_mA);
    
    /// Calculate Load Current from ADC raw data
    uint16_t CalculateCurrent(uint16_t adcValue);
    
    /// Calculate Load/External Voltage from ADC raw data
    uint16_t CalculateVoltage(uint16_t adcValue, E_ADC_CHANNEL channel);

    /// Calculate Temperature from ADC raw data
    int16_t CalculateTemperature(uint16_t adcValue);
    
///public:
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Default constructor (use default calibrationData)
    RL021_DigitalLoad();

    /// Constructor with DAC and ADC device
    RL021_DigitalLoad(MCP47x6base * newDeviceDAC, MCP3428 * newDeviceADC);
    
    ///////////////////////////////////////////////////////////////
    /// Setter for private calibrationData - use to set complete calibrationData
    void SetCalibrationData(S_RL021_Calibration newCalibrationData);
    
    /// Setter for private calibrationData - use to only set single calibrationData
    void SetCalibration_DAC_slope(float calValue, E_DAC_RANGE range);
    void SetCalibration_DAC_offset(float calValue, E_DAC_RANGE range);
    
    void SetCalibration_ADC_slope(float calValue, E_ADC_CHANNEL channel, E_ADC_RANGE range);
    void SetCalibration_ADC_offset(float calValue, E_ADC_CHANNEL channel, E_ADC_RANGE range);
    
    ///////////////////////////////////////////////////////////////
    /// Set actual jumper state like set on PCB
    void SetJumperSetting(E_JUMPER jumper,bool closed);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set DAC output for desired current
    void SetCurrent_mA(uint16_t current_mA);
    ///////////////////////////////////////////////////////////////
    /// DAC - set raw DAC data (interface method to DAC driver)
    void SetRawDac(uint16_t dacValue);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ADC - get raw ADC data from selected channel (interface method to ADC driver)
    uint16_t GetRawAdc(E_ADC_CHANNEL channel);
    
    /// Get measured current from ADC
    uint16_t GetCurrent_mA();
    
    /// Get measured load voltage from ADC
    uint16_t GetVoltageLoad_mV();

    /// Get measured external voltage from ADC
    uint16_t GetVoltageExt_mV();
    
    /// Get NTC temperature in °C x10
    int16_t GetTemperature();
    
};



#endif /* _RL021_DigitalLoad_H_ */
