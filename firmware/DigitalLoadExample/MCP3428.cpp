/****************************************************************************
 /*
 Distributed with a free-will license.
 Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
 MCP3428
 This code is designed to work with the MCP3428_I2CADC I2C Mini Module available from ControlEverything.com.
 https://www.controleverything.com/content/Analog-Digital-Converters?sku=MCP3428_I2CADC#tabs-0-product_tabset-2
 */
/****************************************************************************/

#include <Wire.h>
#include "MCP3428.h"

/**************************************************************************/
/*
        Instantiates a new MCP3428 class with appropriate properties
*/
/***************************************************************************/
MCP3428::MCP3428(uint8_t devAddress)
{
    Wire.begin();
    devAddr = 1101<<3;
    devAddr |= devAddress;
}

MCP3428::~MCP3428()
{
}

/***************************************************************************/
/*
        Verify the I2C connection and Sets up the Hardware
*/
/***************************************************************************/
bool MCP3428::testConnection()
{
    Wire.beginTransmission(devAddr);
    return (Wire.endTransmission() == 0);
}

/**************************************************************************/
/*
        Set the Configuration register with , Sample Rate, Conversion Mode and PGA Gain
        channel: This determines an ADC reading from the specified channel [1, 2, 3, 4]
        resolution: This determines the resolution (Sample Rate) for the device [12, 14, 16]
        mode: This determines the current operational status of the device (0: ONE-SHOT, 1:CONTINUOUS)
        PGA: This configures the programmable gain amplifier [1, 2, 4, 8]
*/
/**************************************************************************/
void MCP3428::SetConfiguration(uint8_t channel, uint8_t resolution, bool mode, uint8_t PGA)
{
    GAIN = PGA;

    if(resolution!=12 && resolution!=14 && resolution!=16)
    {
        SPS = 12;
    }
    else
    {
        SPS = resolution;
    }

    MODE = mode;
    config = 0;
    config = config<<2;
    // Setting the Channel
    config |= (channel-1);
    config = config<<1;
    // Setting the Conversion Mode
    config |= mode;
    config = config<<2;
    // Setting the Resolution (Sample Rate)
    config |= int((SPS-12)/2);
    config = config<<2;
    // Setting the PGA Gain
    //config|=int(log(PGA)/log(2));
    if(PGA == 2)
    {
        config |= 0x01;
    }
    else if(PGA == 4)
    {
        config |= 0x02;
    }
    else if(PGA == 8)
    {
        config |= 0x03;
    }
    
    // Start a conversion using configuration settings
    Wire.beginTransmission(devAddr);
    // 128: This bit is the data ready flag
    // One-Shot Conversion mode
    // Initiate a new conversion
    Wire.write((config |= 128));
    Wire.endTransmission();
}

/**************************************************************************/
/*
    Check the adc conversion
*/
/**************************************************************************/
bool MCP3428::CheckConversion()
{
    uint8_t i = 0;
    no_of_bytes = 3;
    Wire.requestFrom(devAddr, no_of_bytes);

    while(Wire.available())
    {   data[i++] = Wire.read();

        testvar = data[no_of_bytes-1] >> 7;
    }
    return testvar;
}

/**************************************************************************/
/*
        Reads the conversion results, measuring the voltage
        for an ADC reading from the specified channel
        Generates a signed value since the difference can be either
        positive or negative
        The resolution makes the ouptut in 12/14/16-bit
*/
/**************************************************************************/
int16_t MCP3428::readADC()
{

    raw_adc = 0;

    while(CheckConversion() == 1);

    switch (SPS)
    {
  
        case 12:
                raw_adc = data[0];
                raw_adc &= 0b00001111;
                raw_adc = raw_adc << 8;
                raw_adc |= data[1];

                if(raw_adc > 2047)
                {
                    raw_adc = raw_adc - 4096;
                }
        
                // raw_adc = raw_adc*1000/GAIN;
        
                break;
    
        case 14:
                raw_adc = data[0];
                raw_adc &= 0b00111111;
                raw_adc = raw_adc << 8;
                raw_adc |= data[1];

                if(raw_adc > 8191)
                {
                    raw_adc = raw_adc - 16384;
                }
        
                // raw_adc = raw_adc*250/GAIN;
       
                break;
    
        case 16:

                raw_adc = data[0];
                raw_adc = raw_adc << 8;
                raw_adc |= data[1];
                
                /*
                if(raw_adc > 32767)
                {
                    raw_adc = raw_adc - 65536;
                }
                */

                // raw_adc = raw_adc*62.5/GAIN;
      
                break;
    }
    return raw_adc;
}
