// MOCK Classes for DAC and ADC, used for code test without required hardware (ADC and DAC is simulated)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MOCK DAC
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  class MCP47x6base
  {
  public:
    MCP47x6base()
    {
      
    }
  
    void setVOut(uint16_t dacValue)
    {
      //printf("setVOut: %i\n",dacValue);
    }
    
  };

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MOCK ADC
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  class MCP3428
  {
    
  private:    
    uint8_t data[3]; /// read values
    int16_t raw_adc;
    uint8_t selectedChannel;
    
  public:
    MCP3428()
    {
      
    }

    void SetConfiguration(uint8_t channel, uint8_t resolution, bool mode, uint8_t PGA)
    {
      selectedChannel = channel;
      //printf("MCP3428 set channel: %i\n",selectedChannel);
    }
    
    bool CheckConversion()
    {
      // set dummy values   
      //data[0] = selectedChannel; //highest byte
      //data[1] = selectedChannel * 2;     


     switch(selectedChannel)
     {
      case 1: //ADC_CH_CURRENT 
        data[0] = 0xFF; //highest byte
        data[1] = 0xFF;     
      break;
      case 2: //ADC_CH_VLOAD
        data[0] = 0; //highest byte
        data[1] = 0x00;     
      break;
      case 3: //ADC_CH_VEXT
        data[0] = 0x07; //highest byte
        data[1] = 0xFF;     
      break;
      case 4: //ADC_CH_NTC
        data[0] = 0x08; //highest byte
        data[1] = 0x01;     
      break;
      default:
        data[0] = 0; //highest byte
        data[1] = 0;     
      break;
      
     }      
      return false;
    }
   
    int16_t readADC()  
    {
      while(CheckConversion()==1)
      {
        
      }

      //16 bit
      
      raw_adc = 0;
      raw_adc = data[0];
      raw_adc = raw_adc << 8;
      raw_adc |= data[1];

      //12bit
      /*
      raw_adc = data[0];
      raw_adc &= 0b00001111;
      raw_adc = raw_adc << 8;
      raw_adc |= data[1];
      
      if(raw_adc > 2047)
      {
        raw_adc = raw_adc - 4096;
      }
      */
      
     
      return raw_adc;
    }

  };
