/**
* \file    DigitalLoadExample.ino
* \brief    Example Control of Digital Constant Current Source
* \brief    Required hardware: PCB RL-021/xx, Microcontroller (Arduino) with I2C communication  
* \brief    Required drivers: MCP47x6.h, MCP3428.h
* 
* \brief    basic functions: 
*               -Set constant load current and read back all measured channels
                -Control via serial commands
                -Output waveforms to load
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



/*
 * Arduino Nano
 * 
 * I2C - SDA: A4
 * I2C - SCL: A5
 */

#include "printf.h"

#include "RL021_DigitalLoad.h"

////////////////////////////////////////////////////////////////////////////////////
#ifndef MOCK_ADC_DAC
  /// Create DAC Object with default I2C adress 0x60
  MCP47x6base * DAC_mcp47x6 = new MCP4726;
  //MOCK_MCP47x6base DAC_mcp47x6;
  
  /// Create ADC Object with default I2C adress 0x68
  MCP3428 ADC_mcp3428(0); /// A2, A1, A0 bits (000, 0x68)
  //MOCK_MCP3428 ADC_mcp3428; /// debug
  
  /// Create DigitalLoad Object
  //RL021_DigitalLoad myLoad(DAC_mcp47x6, &ADC_mcp3428);
  
  /// Create DigitalLoad Object with mocked ADC/DAC
  RL021_DigitalLoad myLoad(DAC_mcp47x6,&ADC_mcp3428);
////////////////////////////////////////////////////////////////////////////////////
#else

  //Use simulation of ADC and DAC if required hardware is not present
  MCP47x6base DAC_mcp47x6;
  MCP3428 ADC_mcp3428;
  RL021_DigitalLoad myLoad(&DAC_mcp47x6,&ADC_mcp3428);

#endif
////////////////////////////////////////////////////////////////////////////////////
int countTicks = 10000;
uint16_t currentToSet;

/// Increment current stepwise by 1 LSB of DAC (via serial command '+'/'-')
void calibrateCurrent();

/// Print raw ADC data for all 4 channels
void calibrateVoltage();

/// Send all ADC measured values via serial port
void sendInfo();
void sendInfoProtocol();
void sendRawInfoProtocol();

// send 'r' to switch to raw data
// send 't' to switch to mA/mV data
bool sendRawInfo = false; //(false): sendInfoProtocol(), (true):sendRawInfoProtocol()

/// Dummy output functions (call frequently to get waveform)
void Sawtooth();
void Triangle();
void Square(uint16_t currentLevel);

void setup() {
  
  while (!Serial);
  Serial.begin(115200);
  printf_begin();

  //Serial.println("<RL 021/00 Digital Load>");
/*
  Serial.print("check for DAC ... ");
    if (!DAC_mcp47x6->devicepresent()) 
    {
      Serial.println("DAC Device not found");
    }
    else
    {
      Serial.println("DAC Device OK");
    }

    /// Use external voltage reference (2.048V onboard) 
    DAC_mcp47x6->setReference(MCP47x6base::refpinbuff);

    Serial.print("check for ADC ... ");
    if (!ADC_mcp3428.testConnection()) 
    {
      Serial.println("ADC Device not found");
    }
    else
    {
      Serial.println("ADC Device OK");
    }
    */

    DAC_mcp47x6->setReference(MCP47x6base::refpinbuff);


    /// Write board jumper settings (like set on PCB)
    myLoad.SetJumperSetting(JP2_CURRENT,Jumper_Closed);
    myLoad.SetJumperSetting(JP3_VLOAD,Jumper_Open);
    myLoad.SetJumperSetting(JP4_VEXT,Jumper_Open);  

    /// Write calibration data, otherwise default calibration is used
    ///...
    
    currentToSet = 0;
    myLoad.SetCurrent_mA(currentToSet);

    //Serial.println("<start loop>");

}

void loop() 
{  

  //Sawtooth(200);
  //Triangle(150);
  //Square(133);
  //myLoad.SetCurrent_mA(currentToSet);

  /*
  Serial.print("c: ");
  Serial.print(currentToSet);
  Serial.println();
  */
  
  //calibrateCurrent();

  //calibrateVoltage();

  //Character received via UART
  if ( Serial.available() )
  {
    handleSerialCommand();
  }
  
  //tick counter
  countTicks++;
  if (countTicks > 100) //1s
  {
    countTicks = 0;
    //sendInfo();
    

    if(sendRawInfo)
    {
      sendRawInfoProtocol();
    }
    else
    {
      sendInfoProtocol();
    }
  }
  
  delay(10);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read and Interprete Serial command from PC
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
/*
Single character commands:
'+' Increment DAC +1
'-' Decrement DAC -1
'0' Set DAC to 0
'8' Increment DAC +100
'2' Decrement DAC -100
'r' set sendRawInfo TRUE
't' set sendRawInfo FALSE

Multi character commands:
'sa' Read ASCII digits (1-9999) 'e' set load current in mA
'sf' Read ASCII digits (1-9999) 'e' set raw DAC value

'<' Ignore following characters until '>' received



*/
void handleSerialCommand()
{
  uint16_t serialNumber = 0;
                            // E, Z, H, T, ZT
  static uint8_t number[5] = {0,0,0,0,0};
  static bool readInDigit = false;
  static bool firstCharacter = false;
  static uint8_t serialDigitType = 0;
  
  /// Read single character via serial port
  char c = (Serial.read());

  /// Check for multi character command start sign
  if(c == 's' && readInDigit == false)
  {
    readInDigit = true;
    firstCharacter = true;  
  }
  else if(firstCharacter == true)
  {
    serialDigitType = c;
    readInDigit = true;
    firstCharacter = false;
  }
  /// Check for multi character commands data
  if(readInDigit)
  {
      if(c >= 48 && c <= 57)//ASCII '0'-'9'
      {
        uint8_t newDigit;  
        newDigit = c - 48; //ASCII '0'-'9' to integer
        number[4] = number[3]; //ZT
        number[3] = number[2]; //T
        number[2] = number[1]; //H
        number[1] = number[0]; //Z
        number[0] = newDigit; //E      
      }    
  }

  /// Single character commands
  if(readInDigit == false)
  {
      switch (c)
      {
        case '+': //increment DAC small step
            DAC_IncrementRaw(1, 0, 0);
          break;
        case '-': //decrement DAC small step
            DAC_IncrementRaw(0, 0, 0);
          break;
        case '0': //set DAC to 0
            DAC_IncrementRaw(0, 0, 1);
          break;
        case '8': ///increment DAC big step
            DAC_IncrementRaw(1, 1, 0);
          break;
        case '2': //decrement DAC bif step
            DAC_IncrementRaw(0, 1, 0);
          break;
        case 'r':
              sendRawInfo = true;
          break;
        case 't':
              sendRawInfo = false;
          break;
        case '5':
          
          break;
        case '6':
          
          break;
        case '7':
          
          break;
        case '9':
          break;
        default:
          Serial.println("single command unknown");
          break;
      }
  }

  /// Check for multi character command end sign
  if(c == 'e' && readInDigit == true)
  {
    serialNumber = number[0] + number[1]*10 + number[2]*100 + number[3]*1000 + number[4]*10000;
    number[4] = 0;
    number[3] = 0;
    number[2] = 0;
    number[1] = 0;
    number[0] = 0;

    if(serialDigitType == 'a')
    {
      //set read in number to DAC
      myLoad.SetCurrent_mA(serialNumber);   
      Serial.print("<");
      Serial.print("Set Load Current [mA]: ");
      Serial.print(serialNumber);
      Serial.print(">");
      Serial.println();
    }
    else if (serialDigitType == 'f')
    {
      myLoad.SetRawDac(serialNumber);
      Serial.print("<");
      Serial.print("raw DAC set: ");
      Serial.print(serialNumber);
      Serial.print(">");
      Serial.println();
    }


    readInDigit = false;
  }


}


/** Quick & Dirty function to increment/decrement DAC counts
 *
 *  @param bool increment - (1): Increment,   (0): Decrement
 *  @param bool bigStep -   (1): step = 100   (0): step = 1
 *  @param bool reset -     (1): set DAC to 0 (0): Increment/Decrement
 *  @return /
 */
void DAC_IncrementRaw(bool increment, bool bigStep, bool reset)
{
  static uint16_t dacCounts = 0;
  
  if(reset)
  {
    dacCounts = 0;
  }
  else
  {
    if(increment)
    {
      dacCounts += 1 + bigStep*99;     
    }
    else
    {
        dacCounts -= 1 + bigStep*99;
    }
  }

  // Check DAC range
  if(dacCounts > 4095)
  {
    if(increment)
    {
      dacCounts = 4095;
    }
    else
    {
      dacCounts = 0;
    }
  } 
  
  //DAC_mcp47x6->setVOut(dacCounts);
  myLoad.SetRawDac(dacCounts);
  /*
  Serial.print("DAC counts: ");
  Serial.print(dacCounts);
  Serial.println();
  */
}

///////////////////////////////////////////////////////////////////////////
// Set raw DAC value via serial commands
/*
 * '0'  set DAC count to 0
 * '+'  increment DAC counts by 1 (set to 0 if > 4095)
 * '-'  decrement DAC counts by 1 (set to 4095 if < 0)
 * '8'  increment DAC counts by 100
 */
void calibrateCurrent()
{
  static uint16_t dacCounts = 0;
  
  //Character received via UART
  if ( Serial.available() )
  {
      char c = (Serial.read());
    
          switch (c)
          {
            case '0':
              dacCounts = 0;
              break;
            case '+':
              dacCounts++;
              break;
            case '-':
              if(dacCounts)
              {
                dacCounts--;
              }
              else
              {
                 dacCounts = 4095;             
              }
              break;
             case '8':
              dacCounts += 100;
              break;
             default:
             break;
          }
      
      if(dacCounts > 4095)
      {
        dacCounts = 0;
      }   

      myLoad.SetRawDac(dacCounts);
      Serial.print("DAC counts: ");
      Serial.print(dacCounts);
      Serial.println();
  }
}


///////////////////////////////////////////////////////////////////////////
// Measure all ADC channels and print raw values
void calibrateVoltage()
{
  int16_t rawAdc = 0;

  rawAdc = myLoad.GetRawAdc(ADC_CH_CURRENT);
  Serial.print("ADC counts current: ");
  Serial.print(rawAdc);
  Serial.println();

  rawAdc = myLoad.GetRawAdc(ADC_CH_VLOAD);
  Serial.print("ADC counts Vload: ");
  Serial.print(rawAdc);
  Serial.println();

  rawAdc = myLoad.GetRawAdc(ADC_CH_VEXT);
  Serial.print("ADC counts Vext: ");
  Serial.print(rawAdc);
  Serial.println();

  rawAdc = myLoad.GetRawAdc(ADC_CH_NTC);
  Serial.print("ADC counts NTC: ");
  Serial.print(rawAdc);
  Serial.println();  

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quick&Dirty DAC Waveforms - call frequently to get the waveform
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Increment until currentLevel is reached, start again at 0
void Sawtooth(uint16_t currentLevel)
{
  currentToSet++;

  if(currentToSet > currentLevel)
  {
    currentToSet = 0;
  }  
}

///////////////////////////////////////////////////////////////////////////
// Increment until currentLevel is reached, decrement until 0 is reached, start again
void Triangle(uint16_t currentLevel)
{
  static bool increment = true;

  if(increment)
  {
    currentToSet++;
  }
  else
  {
    currentToSet--;
  }
  
  if(currentToSet > currentLevel)
  {
    increment = false;
  }  
  
  if(currentToSet == 0)
  {
    increment = true;
  }
}

///////////////////////////////////////////////////////////////////////////
// Toggle between 0 and currentLevel, time is hard coded to duration
void Square(uint16_t currentLevel)
{
  static bool out = true;
  static uint8_t duration = 50;

  duration--;

  if(duration == 0)
  {
    duration = 50;

    if(out)
    {
      currentToSet = currentLevel;
    }
    else
    {
      currentToSet = 0;      
    }
    out = !out;  
  }  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print Info
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
/// Send values in SI units
/*
 * 's'a'...'e'  load current in mA
 * 's'b'...'e'  load voltage in mV
 * 's'c'...'e'  extern voltage in mV
 * 's'd'...'e'  NTC temp in °Cx10
 */
void sendInfoProtocol()
{
  Serial.print("sa");
  Serial.print(myLoad.GetCurrent_mA());
  Serial.print("e");
  Serial.println();

  Serial.print("sb");
  Serial.print(myLoad.GetVoltageLoad_mV());
  Serial.print("e");
  Serial.println();

  Serial.print("sc");
  Serial.print(myLoad.GetVoltageExt_mV());
  Serial.print("e");
  Serial.println();

  Serial.print("sd");
  Serial.print(myLoad.GetTemperature());
  Serial.print("e");
  Serial.println();   
}

///////////////////////////////////////////////////////////////////////////
/// Send raw ADC values
/*
 * 's'f'...'e'  load current channel
 * 's'g'...'e'  load voltage channel
 * 's'h'...'e'  extern voltage channel
 * 's'i'...'e'  NTC temp channel
 */
void sendRawInfoProtocol()
{
  //////////
  Serial.print("sf");
  Serial.print(myLoad.GetRawAdc(ADC_CH_CURRENT));
  Serial.print("e");
  Serial.println();

  Serial.print("sg");
  Serial.print(myLoad.GetRawAdc(ADC_CH_VLOAD));
  Serial.print("e");
  Serial.println();

  Serial.print("sh");
  Serial.print(myLoad.GetRawAdc(ADC_CH_VEXT));
  Serial.print("e");
  Serial.println();

  Serial.print("si");
  Serial.print(myLoad.GetRawAdc(ADC_CH_NTC));
  Serial.print("e");
  Serial.println();  
}


///////////////////////////////////////////////////////////////////////////
/// Send readable info to console
void sendInfo()
{
  Serial.print("ADC: current [mA]: ");
  Serial.print(myLoad.GetCurrent_mA());
  Serial.println();

  Serial.print("ADC: load voltage [mV]: ");
  Serial.print(myLoad.GetVoltageLoad_mV());
  Serial.println();

  Serial.print("ADC: ext voltage [mV]: ");
  Serial.print(myLoad.GetVoltageExt_mV());
  Serial.println();

  Serial.print("ADC: NTC temp [C x10]: ");
  Serial.print(myLoad.GetTemperature());
  Serial.println();
  
}
