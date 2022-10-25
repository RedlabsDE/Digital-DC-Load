#include "RL021_DigitalLoad.h"


/************************************************************************************************************************************************/
/*  Constructor
/************************************************************************************************************************************************/
/** Default Constructor - for debug only
 * 
 *  @param /
 *	@return /
 */
RL021_DigitalLoad::RL021_DigitalLoad()
{
  /*
    MOCK_MCP47x6base mockDAC;
    deviceDAC = &mockDAC;

    MOCK_MCP3428 mockADC;
    deviceADC = &mockADC;
    */
    
  
    SetDefaultCalibration();
}

/** Constructor 
 *  Set DAC and ADC device
 * 
 *  @param /
 *	@return /
 */
RL021_DigitalLoad::RL021_DigitalLoad(MCP47x6base * newDeviceDAC, MCP3428 * newDeviceADC):deviceDAC(newDeviceDAC), deviceADC(newDeviceADC)
{
    SetDefaultCalibration();
}


/************************************************************************************************************************************************/
/* Private - Calibration                                                                                                                           
/************************************************************************************************************************************************/
/*
 * slope = (V2-V1)/(ADC2-ADC1)
 * offset = V1 - slope*ADC1
 * 
 */


/** Set Default Calibratio Data 
 *  Values are determined by theoretical calculation of resistor settings 
 * 
 *  @param /
 *	@return /
 */
void RL021_DigitalLoad::SetDefaultCalibration()
{
    /// Load DAC default calibration values
    calibrationData.slope_dac[RANGE_DAC_LOW] = 1000.0/4095; /// 1A range
    calibrationData.offset_dac[RANGE_DAC_LOW] = 2; //dummy value - to be measured
    calibrationData.slope_dac[RANGE_DAC_HIGH] = 400.0/165;//10000.0/4095; /// 10A range
    calibrationData.offset_dac[RANGE_DAC_HIGH] = -11; //dummy value - to be measured
    
    /// Load ADC default calibration values (16-bit signed)

    /// JP2 - closed
    calibrationData.slope_adc[RANGE_DAC_HIGH][ADC_CH_CURRENT] = 400.0/1318;//10000.0/32767; /// 10A range
    calibrationData.offset_adc[RANGE_DAC_HIGH][ADC_CH_CURRENT] = 0;
    /// JP2 - open
    calibrationData.slope_adc[RANGE_DAC_LOW][ADC_CH_CURRENT] = 1000.0/32767; /// 1A range
    calibrationData.offset_adc[RANGE_DAC_LOW][ADC_CH_CURRENT] = 0;
    
    /// JP3 - open
    calibrationData.slope_adc[RANGE_ADC_HIGH][ADC_CH_VLOAD] = 10000.0/13262; // 24000.0/32767; //24V range
    calibrationData.offset_adc[RANGE_ADC_HIGH][ADC_CH_VLOAD] = 4;
    /// JP3 - closed
    calibrationData.slope_adc[RANGE_ADC_LOW][ADC_CH_VLOAD] = 4000.0/32767; // 4V range
    calibrationData.offset_adc[RANGE_ADC_LOW][ADC_CH_VLOAD] = 5;
    
    /// JP4 - open
    calibrationData.slope_adc[RANGE_ADC_HIGH][ADC_CH_VEXT] = 24000.0/32767; //24V range
    calibrationData.offset_adc[RANGE_ADC_HIGH][ADC_CH_VEXT] = 2;
    /// JP4 - closed
    calibrationData.slope_adc[RANGE_ADC_LOW][ADC_CH_VEXT] = 4000.0/32767; // 4V range
    calibrationData.offset_adc[RANGE_ADC_LOW][ADC_CH_VEXT] = 2;
    
    
    /// Set default jumper settings
    highRangeSelected_current = false; //jumper JP2 (true: closed, false: open)
    lowRangeSelected_Vload = false; //jumper JP3 (true: closed, false: open)
    lowRangeSelected_Vext = false; //jumper JP4 (true: closed, false: open)
 
}


/************************************************************************************************************************************************/
/* Private - DAC calculation                                                                                                                          
/************************************************************************************************************************************************/
/** Calculate 12-bit DAC register value for desired current  
 * 
 *  @param uint16_t current_mA - 
 *	@return uint16_t - 12-bit DAC value [0-4095] 
 */
uint16_t RL021_DigitalLoad::CalculateDAC(uint16_t current_mA)
{
    uint16_t dacValue = 0;
    float slope = 0;
    float offset = 0;
    
    /// Get calibration data for actual jumper settings
    slope = calibrationData.slope_dac[highRangeSelected_current];
    offset = calibrationData.offset_dac[highRangeSelected_current];

    /// 
    if(current_mA < offset)
    {
        current_mA = offset;
    }

    /// get DAC value via transfer function    
    dacValue = (current_mA - offset)/slope;
    
    /// Limit to 12-bit max value
    if(dacValue > 4095)
    {
        dacValue = 4095;
    }
  
  return dacValue;
}

/************************************************************************************************************************************************/
/* Private - ADC calculation                                                                                                                         
/************************************************************************************************************************************************/
/// Calculate Load/External Voltage from ADC raw data
uint16_t RL021_DigitalLoad::CalculateVoltage(uint16_t adcValue, E_ADC_CHANNEL channel)
{
    uint16_t voltage_mV; 
    float slope = 0;
    float offset = 0;
    
    if(channel == ADC_CH_VLOAD)
    {
        slope = calibrationData.slope_adc[lowRangeSelected_Vload][ADC_CH_VLOAD];  
        offset = calibrationData.offset_adc[lowRangeSelected_Vload][ADC_CH_VLOAD];  
    }
    else if(channel == ADC_CH_VEXT)
    {
        slope = calibrationData.slope_adc[lowRangeSelected_Vext][ADC_CH_VEXT];  
        offset = calibrationData.offset_adc[lowRangeSelected_Vext][ADC_CH_VEXT];  
    }
    else
    {
        
    }
    
    voltage_mV = slope*adcValue - offset;

    return voltage_mV;
}


/// Calculate Load Current from ADC raw data
uint16_t RL021_DigitalLoad::CalculateCurrent(uint16_t adcValue)
{
    uint16_t current_mA; 
    float slope = 0;
    float offset = 0;
    
    slope = calibrationData.slope_adc[highRangeSelected_current][ADC_CH_CURRENT];
    offset = calibrationData.offset_adc[highRangeSelected_current][ADC_CH_CURRENT];
    
    current_mA = slope*adcValue - offset;
    
    return current_mA;
}


/// Calculate Temperature from 16-bit ADC raw data (0-32767)
int16_t RL021_DigitalLoad::CalculateTemperature(uint16_t adcValue)
{
    float Rf = 10000.0; //pullup in kOhm
    float REF = 32767.0; // 16-bit signed ADC TOP value (ADC result with no ADC connected / pull-up shorted)
    
    /// Calculate Resistance of NTC
    float RT = (adcValue / (REF-adcValue)) * Rf;
    /// Calculate R_T / R_25 (10k at 25°C)
    float RT_R25 = RT / 10000.0;
    
    /// Calculation for ideal pull-up (10.0 kOhm)
    //float RT_R25 = ADC / (REF-ADC);

    /// Look through NTC look-up table     
    uint8_t entry = 0;
    for(uint8_t i=0;i<41;i++) //42 entrys
    {
        if(RT_R25 < NTC_TABLE_B57421V2103[i] && RT_R25 > NTC_TABLE_B57421V2103[i+1])
        {
            entry = i;
        }
    }
    
    // temperature in 5 °C steps:
    // int16_t temperaturex10C = -55+(entry*5) * 10;
    
    //linear interpolation
    float faktor = (NTC_TABLE_B57421V2103[entry] - RT_R25) / (NTC_TABLE_B57421V2103[entry] - NTC_TABLE_B57421V2103[entry+1]);   
    float temperature = -55+(entry*5) + faktor*5;

    /// Check valid range
    if(temperature > 150)
    {
        temperature = 150;
    }
    else if(temperature < -55)
    {
        temperature = -55;
    }
    
    int16_t temperaturex10C = (int16_t)(temperature * 10);

    
    return temperaturex10C;    
}

/************************************************************************************************************************************************/
/* Private - ADC / DAC driver interface                                                                                                                         
/************************************************************************************************************************************************/

uint16_t RL021_DigitalLoad::GetRawAdc(E_ADC_CHANNEL channel)
{
    /// Configure ADC: Channel, 16-bit, oneShot, gain=1
    deviceADC->SetConfiguration(channel+1, 16, 0, 1);

    /// read raw ADC value
    /// long readADC();
    int16_t rawAdcRead = deviceADC->readADC();
    uint16_t rawAdc = 0;

    if(rawAdcRead > 0)
    {
      rawAdc = rawAdcRead;
    }
    else
    {
      rawAdc = 0;
    }
    
    
    //printf(" raw adc: %i\n",rawAdc);
    return rawAdc;
    
    //return 32767/2; //debug return
}


/// DAC - set raw DAC data (interface method to DAC driver)
void RL021_DigitalLoad::SetRawDac(uint16_t dacValue)
{
    //printf(" raw DAC: %i\n",dacValue);
    deviceDAC->setVOut(dacValue);
}

/************************************************************************************************************************************************/
/* Public - Calibration / Settings                                                                                                                           
/************************************************************************************************************************************************/
/** Set Calibration data for correct transfer function calculation
 * 
 *  @param S_RL021_Calibration newCalibrationData - 
 *	@return /
 */
void RL021_DigitalLoad::SetCalibrationData(S_RL021_Calibration newCalibrationData)
{
    calibrationData = newCalibrationData;
}

void RL021_DigitalLoad::SetCalibration_DAC_slope(float calValue, E_DAC_RANGE range)
{
    
}
void RL021_DigitalLoad::SetCalibration_DAC_offset(float calValue, E_DAC_RANGE range)
{
    
}

void RL021_DigitalLoad::SetCalibration_ADC_slope(float calValue, E_ADC_CHANNEL channel, E_ADC_RANGE range)
{
    
}
void RL021_DigitalLoad::SetCalibration_ADC_offset(float calValue, E_ADC_CHANNEL channel, E_ADC_RANGE range)
{
    
}



/** Set private jumper settings according to actual jumper state on PCB
 * 
 *  @param E_JUMPER jumper - jumper to set
 *  @param bool closed - (true): jumper closed
 *                      (false): jumper opened
 *	@return /
 */
void RL021_DigitalLoad::SetJumperSetting(E_JUMPER jumper,bool closed)
{
    if(jumper == JP2_CURRENT)
    {
       highRangeSelected_current = closed;
    }
    else if(jumper == JP3_VLOAD)
    {
        lowRangeSelected_Vload = closed;
    }
    else if(jumper == JP4_VEXT)
    {
        lowRangeSelected_Vext = closed;
    }
}

/************************************************************************************************************************************************/
/* Public - set                                                                                                                           
/************************************************************************************************************************************************/
/** Write calculated 12-bit DAC register value (for desired current) to DAC  
 * 
 *  @param uint16_t current_mA - 
 *	@return /
 */
void RL021_DigitalLoad::SetCurrent_mA(uint16_t current_mA)
{
    /// Calculate DAC value
    uint16_t dacValue = CalculateDAC(current_mA);
    
    /// Write value to DAC
    SetRawDac(dacValue);
}


/************************************************************************************************************************************************/
/* Public - get                                                                                                                           
/************************************************************************************************************************************************/

/// Get measured current from ADC
uint16_t RL021_DigitalLoad::GetCurrent_mA()
{
    uint16_t current_mA;
    
    /// Get adc raw data from Vload channel 
    uint16_t rawAdc = GetRawAdc(ADC_CH_CURRENT);
    
    /// Calculate Load Current from ADC raw data
    current_mA =  CalculateCurrent(rawAdc);
    
    return current_mA;
}

/// Get measured load voltage from ADC
uint16_t RL021_DigitalLoad::GetVoltageLoad_mV()
{
    uint16_t voltage_mV;    
    
    /// Get adc raw data from Vload channel 
    uint16_t rawAdc = GetRawAdc(ADC_CH_VLOAD);
    
    /// Get calculated voltage
    voltage_mV = CalculateVoltage(rawAdc, ADC_CH_VLOAD);
    
    return voltage_mV; 
}

/// Get measured external voltage from ADC
uint16_t RL021_DigitalLoad::GetVoltageExt_mV()
{
    uint16_t voltage_mV;    
    
    /// Get adc raw data from Vext channel 
    uint16_t rawAdc = GetRawAdc(ADC_CH_VEXT);
    
    /// Get calculated voltage
    voltage_mV = CalculateVoltage(rawAdc, ADC_CH_VEXT);
    
    return voltage_mV;
}

/// Get NTC temperature in °C x10
int16_t RL021_DigitalLoad::GetTemperature()
{
    int16_t tempCelsiusX10 = 0;
    
    /// Get adc raw data from NTC channel 
    uint16_t rawAdc = GetRawAdc(ADC_CH_NTC);
    
    tempCelsiusX10 = CalculateTemperature(rawAdc);
    
    return tempCelsiusX10;
    
}
