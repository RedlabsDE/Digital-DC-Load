
/*
Uesed P5 elements:
Textfield  (set data)
Console (Println)

Slider (show measured data / set data)
Button

Tab (calibration / normal use)

Toggle (settings)



*/

//www.sojamo.de/libraries/controlp5
import controlP5.*;
ControlP5 cp5;

import processing.serial.*;
Serial myPort;      // The serial port


boolean showGraph = false;

int SLIDER_POS_X = 50;

int SLIDER_WIDTH = 200;
int SLIDER_HEIGHT = 30;

int SLIDER_DISTANCE = SLIDER_HEIGHT + 20;


void setup() 
{
  size(1000,400);
  noStroke();
  cp5 = new ControlP5(this);
  frameRate(20);  
  
  PFont pfont = createFont("Arial",20,true); // use true/false for smooth/no-smooth
  ControlFont font = new ControlFont(pfont,15);
  
  ///////////////////////////////////////////////////////////////////////////////
  /// Show Control
  cp5.addSlider("sliderCurrent")
     .setPosition(SLIDER_POS_X,50+0*SLIDER_DISTANCE)
     .setRange(0,2) //Range 2A (change by settings)
     .setCaptionLabel("[A]  Load Current") 
     .lock()
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(3) 
     .setFont(font)
     ;
     
  cp5.addSlider("sliderVoltLoad")
     .setPosition(SLIDER_POS_X,1*SLIDER_DISTANCE + 50)
     .setRange(0,24)//Range 24V (change by settings)
     .setCaptionLabel("[V]  Load Voltage") 
     .lock()
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(3) 
     .setFont(font)
     ;
     
  cp5.addSlider("sliderVoltExt")
     .setPosition(SLIDER_POS_X,2*SLIDER_DISTANCE + 50)
     .setRange(0,24)//Range 24V (change by settings)
     .setCaptionLabel("[V]  External Voltage") 
     .lock()
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(3) 
     .setFont(font)
     ;
     
  cp5.addSlider("sliderTemp")
     .setPosition(SLIDER_POS_X,3*SLIDER_DISTANCE + 50)
     .setRange(0,200)//Range 24V (change by settings)
     .setCaptionLabel("[°C]  NTC Temp") 
     .lock()
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(1) 
     .setFont(font)
     ;
     
   ///////////////////////////////////////////////////////////////////////////////
   // Show Calibration / raw data
  cp5.addSlider("sliderRawCurrent")
     .setPosition(SLIDER_POS_X,50+0*SLIDER_DISTANCE)
     .setRange(0,32767)
     .setCaptionLabel("ADC  Load Current") 
     .lock()
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(0) 
     .setFont(font)
     ;
     
  cp5.addSlider("sliderRawVoltLoad")
     .setPosition(SLIDER_POS_X,1*SLIDER_DISTANCE + 50)
     .setRange(0,32767)
     .setCaptionLabel("ADC  Load Voltage") 
     .lock()
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(0) 
     .setFont(font)
     ;
     
  cp5.addSlider("sliderRawVoltExt")
     .setPosition(SLIDER_POS_X,2*SLIDER_DISTANCE + 50)
     .setRange(0,32767)
     .setCaptionLabel("ADC  External Voltage") 
     .lock()
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(0) 
     .setFont(font)
     ;
     
  cp5.addSlider("sliderRawTemp")
     .setPosition(SLIDER_POS_X,3*SLIDER_DISTANCE + 50)
     .setRange(0,32767)
     .setCaptionLabel("ADC NTC") 
     .lock()
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(0) 
     .setFont(font)
     ;
  ///////////////////////////////////////////////////////////////////////////////
  /// SET 
  
  cp5.addSlider("sliderSetCurrent")
     .setPosition(SLIDER_POS_X + SLIDER_WIDTH + 200 ,0*SLIDER_DISTANCE + 50)
     .setRange(0,10000) //10A - change range by setting TODO
     .setCaptionLabel("[mA] Load") 
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(0) 
     .setScrollSensitivity(0.0025) //step=1
     .setFont(font)
     ; 
     
  cp5.addSlider("sliderSetRawDAC")
     .setPosition(SLIDER_POS_X + SLIDER_WIDTH + 200 ,0*SLIDER_DISTANCE + 50)
     .setRange(0,4095)
     .setCaptionLabel("DAC raw") 
     .setSize(SLIDER_WIDTH, SLIDER_HEIGHT) 
     .setDecimalPrecision(0) 
     .setScrollSensitivity(0.0025) //step=1
     .setFont(font)
     ; 

  ///////////////////////////////////////////////////////////////////////////////
  // create a new button
  cp5.addButton("BTNsetCurrent")
     .setValue(0)
     .setPosition(SLIDER_POS_X + SLIDER_WIDTH*3+100 ,0*SLIDER_DISTANCE + 50)
     .setSize(50,50)
     .setCaptionLabel("set") 
     .setFont(font)
     ;
     
  cp5.addButton("BTNsetDAC")
     .setValue(0)
     .setPosition(SLIDER_POS_X + SLIDER_WIDTH*3+100 ,0*SLIDER_DISTANCE + 50)
     .setSize(50,50)
     .setCaptionLabel("set") 
     .setFont(font)
     ;

     
  ///////////////////////////////////////////////////////////////////////////////
  /// init value
  cp5.getController("sliderCurrent").setValue(1.123); 
  
  ///////////////////////////////////////////////////////////////////////////////
  /// Tabs
  
  cp5.getTab("default")
     .activateEvent(true)
     .setLabel("Control")
     .setId(1)
     ;
  cp5.addTab("extra")
   .setColorBackground(color(0, 160, 100))
   .setColorLabel(color(255))
   .setColorActive(color(255,128,0))
   .setLabel("calibration")
   .setId(2)
   .activateEvent(true)
   ;
   
   
   cp5.getController("BTNsetDAC").moveTo("extra");
   cp5.getController("sliderSetRawDAC").moveTo("extra");
   
   cp5.getController("sliderRawCurrent").moveTo("extra");
   cp5.getController("sliderRawVoltLoad").moveTo("extra");
   cp5.getController("sliderRawVoltExt").moveTo("extra");
   cp5.getController("sliderRawTemp").moveTo("extra");
   
   
   

   
  
  ///////////////////////////////////////////////////////////////////////////////
  /// serial
  printArray(Serial.list());
  
  
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 115200);
  
  
}

int offsetX = 50;
int offsetY = 300;

int timeX = 0;
/*
void draw() 
{
  //background(100);
  stroke(255);
  
  point(50,200);
  
  
  float tY = cp5.getController("sliderTemp").getValue();
  line(offsetX+timeX, 400-tY, offsetX+timeX, 400);
}
*/


int width = 800;                                                   //use this variable to controll screen widtt
int height = 400;                                                  //use this variable to controll screen height 

int[] data1 = new int[width];                                      //data1, data2 and data3 store the graph information, and must be as long has the width of the screen (pixels)
int[] data2 = new int[width];                    
int[] data3 = new int[width];
int newDataPoint1 = 0;                                             //newDataPoint1, newDataPoint2 and newDataPoint3 are esentially buffers for incoming data
int newDataPoint2 = 0;
int newDataPoint3 = 0;
void draw()                                                        //the main routine (runs continuously until the program is ended)
{
  
  if(showGraph == true)
  {
    background(0,0,100);                                         //set the background to white. There are RGB color selectors online if you'd like to find a better looking color
    stroke(0,0,0);                                                   //set the stroke (line) color to black
    strokeWeight(2);                                                 //set the stroke width (weight) for the axes
    line(0,height/2,width,height/2);                                 //draw the x-axis line            
    line(width/4,0,width/4,height);                                  //draw the y-axis line
    
    float tY = cp5.getController("sliderTemp").getValue();
    float vY = cp5.getController("sliderCurrent").getValue();
    //****DATA HANDLING****//                                        //the following 3 variables are buffers for incoming informaion, set these variables to the data you would like to display
    //*********************//  
    newDataPoint1 = offsetY-(int)tY;                                          
    newDataPoint2 = offsetY-(int)(vY*1000);          
    newDataPoint3 = 0;                                               //because the third data buffer is set to a constant (0), you'll notice a blue line at the top of the window.
    //*********************//
    //*********************//
    
    for(int i = 0; i < width-1; i++)                                 //each interation of draw, shift data points one pixel to the left to simulate a continuously moving graph
    {
      data1[i] = data1[i+1];
      data2[i] = data2[i+1];
      data3[i] = data3[i+1];
    }
    
    data1[width-1] = newDataPoint1;                                 //introduce the bufffered data into the rightmost data slot
    data2[width-1] = newDataPoint2;
    data3[width-1] = newDataPoint3;
    
    strokeWeight(2);                                                //set the stroke width (weight) for the actual graph
    
    for(int i = width-1; i > 0; i--)                                
    {
      stroke(255,0,0);
      line(i,data1[i-1], i+1, data1[i]);
      stroke(0,255,0);
      line(i,data2[i-1], i+1, data2[i]);
      stroke(0,0,255);
      line(i,data3[i-1], i+1, data3[i]); 
    }
  
  }
  else
  {
    background(100);
  }
  
}    

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void serialEvent(Serial myPort) 
{
  //int inByte = myPort.read();  
  char inChar = myPort.readChar();
  print(inChar);
  handleSerialCommand(inChar);
  
}

                            // E, Z, H, T

//int number[];

//int[] number = new int[5];

int[] number = {0,0,0,0,0};

boolean ignoreSerial = false;
boolean readInDigit = false;
boolean firstCharacter = false;

char serialDigitType; //first character after 's' define which number is sent

void handleSerialCommand(char c)
{
   //int number[4] = {0,0,0,0};
   //bool readInDigit = false;
   
   
  /// Read single character via serial port
  //char c = (Serial.read());
  
  if(c == '<')
  {
    ignoreSerial = true;
  }
  
  if(c == '>')
  {
    ignoreSerial = false;
  }

  //ignoreSerial = true;

  if(!ignoreSerial)
  {
    //return;
  

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
  else if(readInDigit && firstCharacter == false)/// Check for multi character commands data
  {
      if(c >= 48 && c <= 57)//ASCII '0'-'9'
      {
        int newDigit;  
        newDigit = c - 48; //ASCII '0'-'9' to integer

        number[4] = number[3]; //T
        number[3] = number[2]; //T
        number[2] = number[1]; //H
        number[1] = number[0]; //Z
        number[0] = newDigit; //E      
      }    
  }
  
  /// Single character commands
  if(readInDigit == false)
  {

  }
  
  /// Check for multi character command end sign
  if(c == 'e' && readInDigit == true)
  {
    long serialNumber = number[0] + number[1]*10 + number[2]*100 + number[3]*1000 + number[4]*10000;
    number[4] = 0;
    number[3] = 0;
    number[2] = 0;
    number[1] = 0;
    number[0] = 0;

    //set read in number to DAC
    //myLoad.SetCurrent_mA(serialNumber);      
    
    actSerialCommand(serialDigitType, serialNumber);   

    readInDigit = false;
  }
  
  }
}


void actSerialCommand(char type, long value)
{
  
  switch(type)
  {
    case 'a':
      cp5.getController("sliderCurrent").setValue(value/1000.0); // current received in mA (show in A)
    break;
    case 'b':
      cp5.getController("sliderVoltLoad").setValue(value/1000.0); // voltage received in mV (show in V)
    break;
    case 'c':
      cp5.getController("sliderVoltExt").setValue(value/1000.0); // voltage received in mV (show in V)
    break;
    case 'd':
      cp5.getController("sliderTemp").setValue(value/10.0); // temp received in °Cx10 (show in °C)
      timeX++;
    break;
    case 'f': //raw ADC CH1
      cp5.getController("sliderRawCurrent").setValue(value);
    break;
    case 'g': //raw ADC CH2
      cp5.getController("sliderRawVoltLoad").setValue(value);
    break;
    case 'h': //raw ADC CH3
      cp5.getController("sliderRawVoltExt").setValue(value);
    break;
    case 'i': //raw ADC CH4
      cp5.getController("sliderRawTemp").setValue(value);
    break;
    default:
    
    break;
  }
    
  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
public void controlEvent(ControlEvent theControlEvent) 
{
  //println(theEvent.getController().getName());
  
  if (theControlEvent.isTab())
  {
    println("got an event from tab : "+theControlEvent.getTab().getName()+" with id "+theControlEvent.getTab().getId());
    
    if(theControlEvent.getTab().getId() == 1) //default tab
    {
      myPort.write('t'); //switch uC to send mA/mV data
    }
    else if(theControlEvent.getTab().getId() == 2) //extra tab
    {
      myPort.write('r'); //switch uC to send raw data
    }
    
    
  }
  
  
  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// 's'a'...'e' 0-99999
public void BTNsetCurrent(int theValue) 
{
  //send mA 
  
  myPort.write('s');
  myPort.write('a');
  float currentToSet = cp5.getController("sliderSetCurrent").getValue();
  int current = (int)(currentToSet);
  myPort.write(str(current));
  myPort.write('e'); 
}



// 's'f'...'e' 0-4095
public void BTNsetDAC(int theValue) 
{
  //send raw DAC value
  myPort.write('s');
  myPort.write('f');
  float currentToSet = cp5.getController("sliderSetRawDAC").getValue();
  int current = (int)(currentToSet);
  myPort.write(str(current));
  myPort.write('e'); 
  
}
