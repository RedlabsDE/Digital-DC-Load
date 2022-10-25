#include "MCP47x6.h"

const byte MCP47x6_defaultaddr = 0x60;


MCP47x6base::MCP47x6base() {
  i2caddr = MCP47x6_defaultaddr;
  command = 0;
  commandneeded = false;
  writemode = eepromwritenot;
  vref = supplyunbuff;
  pwrdwn = powerdownnot;
}

MCP47x6base::MCP47x6base(uint8_t addr): i2caddr(addr) {
  i2caddr = addr;
  command = 0;
  commandneeded = false;
  writemode = eepromwritenot;
  vref = supplyunbuff;
  pwrdwn = powerdownnot;
}

boolean MCP47x6base::devicepresent(void) {
  Wire.beginTransmission(i2caddr);
  return (Wire.endTransmission() == 0);
}

void MCP47x6base::setGain(const boolean set2xgain) {
  command = (command & ~0x01);
  if (set2xgain) {
    command = command | 0x01;
  }
  commandneeded = true;
};

void MCP47x6base::setReference(const voltagereference_t refmode) {
                        //76543210
  command = (command & ~0b00011000);

  switch (refmode) {
    case supplyunbuff : {
                             //76543210
        command = (command | 0b00000000);
        break;
      }
    case refpinunbuff : {
                             //76543210
        command = (command | 0b00010000);
        break;
      }
    case refpinbuff : {
                             //76543210
        command = (command | 0b00011000);
        break;
      }
  }
  commandneeded = true;
}

byte MCP47x6base::getBits() 
{
    return bits;
}


void MCP47x6base::setEEPROMwritemode(const eeprommode_t write2eeprom)
{
  writemode = write2eeprom;
}

boolean MCP47x6base::setVOut(const int avalue) {
  Wire.beginTransmission(i2caddr);

  if (commandneeded) {
    // just in case these bits are set...
                          //76543210 
    command = (command & ~0b11100000);
    switch (writemode) {
      case eepromwritenot: {
                               //76543210
          command = (command | 0b01000000);
          break;
        }
      case eepromwriteonce: {
                               //76543210
          command = (command | 0b01100000);
          writemode=eepromwritenot;
          break;
        }
      case eepromwritealways: {
                               //76543210
          command = (command | 0b01100000);
          break;
        }
    }
    Wire.write((uint8_t) (command));

    // as shown in "figure 6-2"
    setOutPutBytesCmd(avalue);
    commandneeded = false;
  } else {
    // as shown in "figure 6-1"
    setOutPutBytesDev(avalue);
  }
  return (Wire.endTransmission() == 0);
}

// as shown in "figure 6-1"
const void MCP4706::setOutPutBytesDev(const int avalue) {
  Wire.write((uint8_t) (0));
  Wire.write((uint8_t) ((avalue) & 0xff));
}

// as shown in "figure 6-2"
const void MCP4706::setOutPutBytesCmd(const int avalue) {
  Wire.write((uint8_t) ((avalue) & 0xff));
  Wire.write((uint8_t) (0));
}

// as shown in "figure 6-1"
const void MCP4716::setOutPutBytesDev(const int avalue) {
  Wire.write((uint8_t) ((avalue >> 6) & 0x0f));
  Wire.write((uint8_t) ((avalue << 2) & 0xff));
}

// as shown in "figure 6-2"
const void MCP4716::setOutPutBytesCmd(const int avalue) {
  Wire.write((uint8_t) ((avalue >> 2) & 0xff));
  Wire.write((uint8_t) ((avalue << 6) & 0xff));
}

// as shown in "figure 6-1"
const void MCP4726::setOutPutBytesDev(const int avalue) {
  Wire.write((uint8_t) ((avalue >> 8) & 0x0f));
  Wire.write((uint8_t) (avalue & 0xff));
}

// as shown in "figure 6-2"
const void MCP4726::setOutPutBytesCmd(const int avalue) {
  Wire.write((uint8_t) ((avalue >> 4) & 0xff));
  Wire.write((uint8_t) ((avalue << 4) & 0xff));
}


