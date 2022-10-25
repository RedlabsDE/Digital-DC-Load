/**************************************************************************/
/*
   an Arduino library for the Microchip mcp4706, mcp4716 and mcp4726 8/10/12-bit DAC

   Motivation
     ++ make a lib that really works
     ++ exercise in
          +++ object oriented programming / C++
          +++ nice capsulation of hardware
          +++ structured software layout
          +++ not necessarily tricky, shortest, smartest

   Working-With-Hardware-Verification:
      mcp4716    2017-10-24
      mcp4706/26 2017-11-19

   needed work:
    -- ide 1.9.++ - support? integral documentation? doxygen?

    Chip-Info    
    
    Sot 23-6
          +-----+
    V_out + O   + V_Ref
          +     +
    V_SS- +     + SCL
          +     +
    V_DD+ +     + SDA
          +-----+

    Chip Marking:
        i2c    0x60   0x61   0x62   0x63
       4706    DBxx / DCxx / DDxx / DExx
       4716    DFxx / DGxx / DHxx / DJxx
       4726    DKxx / DLxx / DMxx / DPxx

       
   holger lembke, lembke@gmail.com, https://github.com/holgerlembke/MCP47x6
*/
/**************************************************************************/

#ifndef _MCP47x6_H_
#define _MCP47x6_H_

#include <Arduino.h>
#include <Wire.h>


// base class, dont use directly (you can't anyway)
class MCP47x6base {
  public:
    enum eeprommode_t { eepromwritenot, eepromwriteonce, eepromwritealways };
    enum voltagereference_t { supplyunbuff, refpinunbuff, refpinbuff };
    // defined but not used.
    enum powerdownmode_t { powerdownnot, powerdown1k, powerdown100k, powerdown500k };

    // query the device (not really. Just look if someone is at i2c adress)
    boolean devicepresent(void);

    // by default, changes are only written to the volatile DAC register, so lost on power off/on
    void setEEPROMwritemode(const eeprommode_t write2eeprom);

    // set gaint to 2x or 1x (Table 4-4)
    void setGain(const boolean set2xgain);
    // Voltage Reference (Table 4-4)
    void setReference(const voltagereference_t refmode);
    // returns the ADC-Bit-Width
    byte getBits();

    // the real deal.
    boolean setVOut(const int avalue);
  protected:
    MCP47x6base();
    MCP47x6base(uint8_t addr);

    // abstracts
    virtual const void setOutPutBytesDev(const int avalue) = 0;
    virtual const void setOutPutBytesCmd(const int avalue) = 0;

    uint8_t i2caddr;
    byte bits = 0;
  private:
    byte command;
    boolean commandneeded;
    eeprommode_t writemode;
    voltagereference_t vref;
    powerdownmode_t pwrdwn;
};


// use this:
class MCP4706: public MCP47x6base {
  public:
    MCP4706() {
      bits = 8;
    };
    MCP4706(uint8_t addr) {
      bits = 8;
    };
  protected:
    virtual const void setOutPutBytesDev(const int avalue);
    virtual const void setOutPutBytesCmd(const int avalue);
};

// or that:
class MCP4716: public MCP47x6base {
  public:
    MCP4716() {
      bits = 10;
    };
    MCP4716(uint8_t addr) {
      bits = 10;
    };
  protected:
    virtual const void setOutPutBytesDev(const int avalue);
    virtual const void setOutPutBytesCmd(const int avalue);
};

// or even better:
class MCP4726: public MCP47x6base {
  public:
    MCP4726() {
      bits = 12;
    };
    MCP4726(uint8_t addr) {
      bits = 12;
    };
  protected:
    virtual const void setOutPutBytesDev(const int avalue);
    virtual const void setOutPutBytesCmd(const int avalue);
};

#endif /* _MCP47x6_H_ */
