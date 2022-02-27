// MagTweez.ino

// magnetic tweezer current control firmware
// for Teensy 3.2 under Arduino 1.8.3 and Teensyduino
// Steve Sawtelle
// HHMI Janelia jET
// 20190709

// requires CmdArduino-master, Originally written by Christopher Wang, FreakLabs
// altered by Steve Sawtelle HHMi Janelia jET
// Edited by Cristian Castillo UACh

// VERSIONS
#define VERSION "20200416"

// 20200416 ccv
// - Add looping function for list and ramp

// 20191004 sws
// - when stopping sine or ramp, set current to 0
// - add external trigger 

// 20190913 sws
// - add list comamnd back in

// 20190827 sws
// - added sine out, interpolate on list 
// - add support for two channels on sine and list 

// 20190711 sws
// - added offset zero cmd

// 20190709 sws
// - started

#include <Cmd.h>
#include <SPI.h>
#include <DAC_AD57x4SA.h>

#define ZERO 1626
#define enable0Pin 8
#define enable1Pin 9
#define DACCS 10
#define TTLinPin 21
#define TTLoutPin 22

#define MAXLIST 100

uint16_t calZero = ZERO;
float ISet[3] = { 0.0, 0.0 , 0.0};
float IValue[MAXLIST];
uint16_t ITime[MAXLIST];
uint8_t iStep = 0;

float listIValue[MAXLIST];
uint16_t listITime[MAXLIST];
uint8_t listiStep = 0;

DAC_AD57x4SA dac;    // instantiate a DAC panel driver

IntervalTimer listTimer; 
IntervalTimer sineTimer;
IntervalTimer rampTimer;

float listValue;      // current output for DAC
int16_t listIndex;   // index into list values
int16_t listCounts;  // how many interpolated values in this set
float listInc;        // amount to change value each interpolation
int listChannel;  // channel to run list on

float rampValue;      // current output for DAC
uint16_t rampIndex;   // index into ramp list values
uint16_t rampCounts;  // how many interpolated values in this set
float rampInc;        // amount to change value each interpolation
int rampChannel;  // channel to run ramp on
int sineChannel;

boolean sineTrigFlag = false;
boolean rampTrigFlag = false;
boolean listTrigFlag = false;
boolean sineStartedFlag = false;
boolean rampStartedFlag = false;
boolean listStartedFlag = false;

boolean listloopFlag = false;
boolean ramploopFlag = false;

float sinewave[] = 
{
0, 0.034899497, 0.069756474, 0.104528463, 0.139173101, 0.173648178, 0.207911691, 0.241921896, 0.275637356, 0.309016994, 0.342020143, 0.374606593, 0.406736643,0.438371147, 0.469471563, 
0.5, 0.529919264, 0.559192903, 0.587785252, 0.615661475, 0.64278761, 0.669130606, 0.69465837, 0.7193398, 0.743144825, 0.766044443,0.788010754, 0.809016994, 0.829037573, 0.848048096, 
0.866025404, 0.882947593, 0.898794046, 0.913545458, 0.927183855, 0.939692621, 0.951056516, 0.961261696, 0.970295726, 0.978147601, 0.984807753, 0.990268069, 0.994521895, 0.99756405, 0.999390827, 
1, 0.999390827, 0.99756405, 0.994521895, 0.990268069, 0.984807753, 0.978147601, 0.970295726, 0.961261696, 0.951056516, 0.939692621, 0.927183855, 0.913545458, 0.898794046, 0.882947593, 
0.866025404, 0.848048096, 0.829037573, 0.809016994, 0.788010754, 0.766044443, 0.743144825, 0.7193398, 0.69465837, 0.669130606, 0.64278761, 0.615661475, 0.587785252, 0.559192903, 0.529919264,
0.5, 0.469471563, 0.438371147, 0.406736643, 0.374606593, 0.342020143, 0.309016994, 0.275637356, 0.241921896, 0.207911691, 0.173648178, 0.139173101, 0.104528463, 0.069756474, 0.034899497,
0, -0.034899497, -0.069756474, -0.104528463, -0.139173101, -0.173648178, -0.207911691, -0.241921896, -0.275637356, -0.309016994, -0.342020143, -0.374606593, -0.406736643, -0.438371147, -0.469471563,
-0.5, -0.529919264, -0.559192903, -0.587785252, -0.615661475, -0.64278761, -0.669130606, -0.69465837, -0.7193398, -0.743144825, -0.766044443, -0.788010754, -0.809016994, -0.829037573, -0.848048096,
-0.866025404, -0.882947593, -0.898794046, -0.913545458, -0.927183855, -0.939692621, -0.951056516, -0.961261696, -0.970295726, -0.978147601, -0.984807753,-0.990268069, -0.994521895, -0.99756405, -0.999390827, 
-1, -0.999390827, -0.99756405, -0.994521895, -0.990268069, -0.984807753, -0.978147601, -0.970295726, -0.961261696, -0.951056516, -0.939692621, -0.927183855, -0.913545458, -0.898794046, -0.882947593,
-0.866025404, -0.848048096, -0.829037573, -0.809016994, -0.788010754, -0.766044443, -0.743144825, -0.7193398, -0.69465837, -0.669130606, -0.64278761, -0.615661475, -0.587785252, -0.559192903, -0.529919264, 
-0.5, -0.469471563, -0.438371147, -0.406736643, -0.374606593, -0.342020143, -0.309016994, -0.275637356, -0.241921896, -0.207911691, -0.173648178, -0.139173101, -0.104528463, -0.069756474, -0.034899497,
0
};

float freq = 0.0;
float ampl = 0.0;
float offset = 0.0;



// ====================================
// === L I S T   T I M E R   I N T  ===
// ====================================
void listStep()
{
  if( ++listCounts >= listITime[listIndex] )
  {
     if( ++listIndex < listiStep )
     {
        listCounts = 0;
        setCurrent( listChannel, listIValue[listIndex]);                   
     }
     else
     {
      if(listloopFlag==true)
      {
        listIndex = 0;
        listCounts = 0;
        setCurrent( listChannel, listIValue[listIndex]);
      }
      else
      {
        listTimer.end();
      }
     } 
  }             
}
   
// ====================================
// === R A M P   T I M E R   I N T  ===
// ====================================
void rampStep()
{
   if( rampCounts > 0 ) // still iterating
   {
       rampValue += rampInc;
       setCurrent( rampChannel, rampValue);
       rampCounts--;       
   }
   else
   {
       if( ++rampIndex < iStep )
       {
          rampCounts = 10 * ITime[rampIndex];
          rampInc = (IValue[rampIndex] - rampValue)/rampCounts; 
//          Serial.print(listCounts);
//          Serial.print(" , ");
//          Serial.println( listInc);                  
       }
       else
       {
        if(ramploopFlag==true)
        {
          rampValue = 0;
          rampIndex = 0;
          rampCounts = 10 * ITime[rampIndex];
          rampInc = IValue[rampIndex]/rampCounts;
          rampValue += rampInc;
          setCurrent( rampChannel, rampValue);
          rampCounts--;  
        }
        else
        {
          rampTimer.end();
        }
       }          
   }  
}


// =============================
// === S E T   C U R R E N T ===
// =============================

// set DAC to produce current of 'fI' Amps
void setCurrent(uint8_t ch, float fI)
{
    if( (ch == 1) || (ch == 2) )
    {
   //  digitalWrite(enablePin, HIGH);  // start disabled
      int16_t I = (int16_t) (fI * 0x7FFF) ; // (fI * 0.5 * 4095.0 / 1.2 +calZero)
      dac.write( ch, I);
//      Serial.print(fI);
//      Serial.print("  ");
//      Serial.println(I);
   //
   //digitalWrite(enablePin, LOW);  // enable
    }
}

// =============================
// === S T A R T    R A M P  ===
// =============================

void startRamp(int ch)
{
   rampValue = 0;
   rampIndex = 0;
   rampCounts = 10 * ITime[rampIndex];
   rampInc = IValue[rampIndex]/rampCounts;
           Serial.print(ITime[rampIndex]);
           Serial.print(" , ");      
           Serial.print(IValue[rampIndex]);
           Serial.print(" , ");          
           Serial.print(rampCounts);
           Serial.print(" , ");
           Serial.println( rampInc);
   
   rampTimer.begin(rampStep, 100);
}

// =============================
// === S T A R T    L I S T  ===
// =============================

void startList(int ch)
{
   listIndex = 0;
   listCounts = 0;
   listChannel = ch;
   setCurrent( listChannel, listIValue[listIndex]);
   listTimer.begin(listStep, 1000);
}


// ================= C O M M A N D S ==================

// Commands that can be sent to the device
// Paramters separated by spaces, end with LF,CR

// =================
// === I   C M D ===
// =================

// parse new current

void ICmd(int arg_cnt, char **args)
{  
    if( arg_cnt > 1 )
    {
        uint8_t  ch = cmdStr2Float(args[1]);
        float fI = cmdStr2Float(args[2]);
        if( (ch == 1) || (ch == 2) )
        {
          if( (fI >= -1.0) && (fI <= 1.0) )
          {
              ISet[ch] = fI;
              Serial.print(ch);
              Serial.print("=");
              Serial.println(ISet[ch]);
              setCurrent(ch, ISet[ch]);
          }      
        }  
    }
    else
    {
       Serial.print("1=");
       Serial.print(ISet[1]);
       Serial.print(" 2=");
       Serial.println(ISet[2]);
    } 
}

// =================
// === C   C M D ===
// =================

// parse new DAC count to set (for testing)

void CCmd(int arg_cnt, char **args)
{
   if( arg_cnt > 2 )
   {
      uint8_t ch = cmdStr2Num(args[1], 10);
      uint16_t cnt = cmdStr2Num(args[2], 10);
      if( (ch == 1) || (ch == 2) )
      {
        if( (cnt >= 0) && (cnt <= 0xFFFF) )
          dac.write(ch, cnt);
      }    
   }   
  
}

// =======================
// === L I S T   C M D ===
// =======================

// list of current settings (A) and times (msecs) to run
// 1. To enter a list of currents send 'L' then the current then time to run that current
// 2. To view the current list, send 'L' with no prarameters
// 3. To clear the list, send 'L 0' (zero)
// 4. To run the list , send 'L ch' where ch is 1 or 2 for the channel

void listCmd(int arg_cnt, char **args)
{
   if( arg_cnt > 2 )
   {
      if( strcmp( args[1], "T") != 0 )
      {
          float fI = cmdStr2Float(args[1]);
          if( (fI >= -.9999) && (fI <= 1.0001) )
          {
             uint16_t iT = cmdStr2Num(args[2], 10);
             if( iT == 0 )
             {
                listiStep = 0;
                listChannel = 0;
                listloopFlag=false;
             }
             else
             {
                listIValue[listiStep] = fI;
                listITime[listiStep] = iT;
                if( listiStep < MAXLIST)
                    listiStep++;                              
             }       
          }    // endif good freq
      }    
      else  // ext trig
      {
        int ch = cmdStr2Num(args[2],10);
        if ( ch == 0 ) 
        {
            listTimer.end();
            setCurrent( listChannel,0);        
            listTrigFlag = false;
        }   
        else if ( (ch == 1) || (ch == 2) )
        {   
            listChannel = ch;
            listTrigFlag = true;
        }          
      }    
   }  // endif arg_cnt  > 2
   else if (arg_cnt == 2 )
   {
    if( strcmp( args[1], "O") == 0 )
    {
      listloopFlag=true;
      Serial.println("looping list");
    }
    else
    {
       int ch = cmdStr2Num(args[1],10);
       if ( ch == 0 ) 
       {
          listTimer.end();
          setCurrent( listChannel,0);
         // listiStep = 0;
         // listChannel = 0;
       }   
       else if ( (ch == 1) || (ch == 2) )
       {      
          startList(ch);
       }           
     }
   }
   else
   {    
      if( listiStep == 0 ) 
      {
          Serial.println("Set List of Interpolated Steps:");
          Serial.println("   L i t; where:");
          Serial.println("       i = current (-1.0 to 1.0 amps)");
          Serial.println("       t = time (milliseconds) to stay at that value");
          Serial.println("       time = 0 will reset list");
          Serial.println("   L c;  run previousy entered list on channel 'c' (1 or2)");
          Serial.println("       c = 0  to stop current list run");
          Serial.println("   L O;  loop the list");
      }
      else
      {
        for( uint8_t i = 0; i < listiStep; i++)
        {
           Serial.print(i);
           Serial.print(" ");
           Serial.print(listIValue[i]);
           Serial.print(" ");
           Serial.println(listITime[i]);
        } 
      }  // endif - does a list exist?    
   } // endif add, reset, or print list
}

// =======================
// === R A M P    C M D ===
// =======================

// list of current settings (A) and times (msecs) to run
// 1. To enter a list of currents send 'R' then the current then time to run that current
// 2. To view the current list, send 'R' with no parameters
// 3. To clear the list, send 'R 0' (zero)
// 4. To run the list , send 'R ch' where ch is 1 or 2 for the channel

void rampCmd(int arg_cnt, char **args)
{
   if( arg_cnt > 2 )  // more than one argument 
   {
      if( strcmp( args[1], "T") != 0 )  // enter a value
      {
          float fI = cmdStr2Float(args[1]);  
          if( (fI >= -1.0) && (fI < 1.0) )
          {
             uint16_t iT = cmdStr2Num(args[2], 10);
             if( iT == 0 )
             {
                iStep = 0;
                rampChannel = 0;
             }
             else
             {
                IValue[iStep] = fI;
                ITime[iStep] = iT;
                if( iStep < MAXLIST)
                    iStep++;                              
             }       
          }    // endif good freq  
      } 
      else  // ext trig
      {
        int ch = cmdStr2Num(args[2],10);
        if ( ch == 0 ) 
        {
            setCurrent( rampChannel,0);
            rampTimer.end();
            rampTrigFlag = false;
            ramploopFlag = false;
        }   
        else if ( (ch == 1) || (ch == 2) )
        {   
            rampChannel = ch;
            rampTrigFlag = true;
        }       
      }    
   }  // endif arg_cnt  > 2
   else if (arg_cnt == 2 ) 
   {
    if( strcmp( args[1], "O") == 0 )
    {
      ramploopFlag=true;
      Serial.println("looping ramp");
    }
    else
    {

  Serial.println(args[1]);
       int ch = cmdStr2Num(args[1],10);
        if ( ch == 0 )      // end any ramp in progress
        {
            setCurrent( rampChannel,0);
            rampTimer.end();
            ramploopFlag = false;
          // iStep = 0;
          // listChannel = 0;
        }   
        else if ( (ch == 1) || (ch == 2) )  // start ramp list output
        {   
            rampChannel = ch;
            startRamp(ch);
        }
    }                      
   }
   else
   {    
      if( iStep == 0 ) 
      {
          Serial.println("Set Ramp List of Interpolated Steps:");
          Serial.println("   R i t; where:");
          Serial.println("       i = current (-1.0 to 1.0 amps)");
          Serial.println("       t = time (milliseconds) to ramp up to that value");
          Serial.println("       time = 0 will reset list");
          Serial.println("   R T c;  use trigger in to run previousy entered ramp list on channel 'c' (1 or2)");
          Serial.println("   R c;  run previousy entered ramp list on channel 'c' (1 or2)");
          Serial.println("       c = 0  to stop current ramp run");
          Serial.println("   R O;  Loop the ramp list");
      }
      else
      {
        for( uint8_t i = 0; i < iStep; i++)
        {
           Serial.print(i);
           Serial.print(" ");
           Serial.print(IValue[i]);
           Serial.print(" ");
           Serial.println(ITime[i]);
        } 
      }  // endif - does a list exist?    
   } // endif add, reset, or print list
}

// ==========================
// === S I N E  T I M E R ===
// ==========================


volatile uint16_t nextsine = 0;

void sineStep()
{ 
    setCurrent( sineChannel, sinewave[nextsine] * ampl + offset);
    if( ++nextsine >= 180 ) nextsine = 0;
}


// =======================
// === S I N E   C M D ===
// =======================

// Set up for a sine wave out

void sineCmd(int arg_cnt, char **args)
{
   if( arg_cnt > 3 )
   {
   //   uint8_t ch = cmdStr2Num(args[1], 10);
      freq = cmdStr2Float(args[1]);
      ampl = cmdStr2Float(args[2]);
      offset = cmdStr2Float(args[3]);
      nextsine = 0;
      
   } 
   else if ( arg_cnt == 3)  // external trigger
   {
      if( *args[1] == 'T' ) 
      {  
        int ch = cmdStr2Num(args[2],10);
        if ( (ch == 1) || (ch == 2) )
        {       
          if( (freq >= 0.1) && (freq <= 100.0) )
          {
              sineChannel = ch;     
              sineTrigFlag = true;                 
           }           
        }  
        else
        {
           sineTrigFlag = false;    
        }
      }  
   }
   else if (arg_cnt == 2 )  // command  start
   {     
       int ch = cmdStr2Num(args[1],10);
       if ( ch == 0 ) 
       {
          setCurrent( sineChannel, 0);
          sineTimer.end(); 
       }   
       else if ( (ch == 1) || (ch == 2) )
       {          
          if( (freq >= 0.1) && (freq <= 100.0) )
          {
             sineChannel = ch;
             sineTimer.begin( sineStep, (1000000.0 / (freq * 180.0)) );
          }           
       }      
   }
   else
   {
       if( freq < 0.1 )
       {
          Serial.println("Set Sine wave:");
          Serial.println("   S f a o ; where:");
          Serial.println("       f = freq (0.1 to 100.0)");
          Serial.println("       a = p-p current amplitude (0.0 to 1.0 amps)");
          Serial.println("       o = offset (+/-1.0 amps)");
          Serial.println("   S T c; Start sine wave on channel 'c' (1 or 2) on trigger");
          Serial.println("   S c; Start sine wave on channel 'c' (1 or 2)");
          Serial.println("     'c' = 0 to stop sine wave");         
       }
       else
       {
          Serial.print("Sine settings: freq:");
          Serial.print(freq);
          Serial.print(" amp:");
          Serial.print(ampl);
          Serial.print(" offset:");
          Serial.println(offset);       
       }
   }
}




// =======================
// ===  R U N   C M D ===
// =======================

// run the list of currents and time

void runCmd(int arg_cnt, char **args)
{
   listValue = 0;
   listIndex = 0;
   listCounts = 10 * ITime[listIndex];
   listInc = IValue[listIndex]/listCounts;
          Serial.print(listCounts);
          Serial.print(" ---- ");
          Serial.println( listInc);
   
   listTimer.begin(listStep, 100);
}

// =============================
// === D I S A B L E   C M D ===
// =============================

// disable the output

void disableCmd(int arg_cnt, char **args)
{
   if( arg_cnt > 1 )
   {
        uint8_t ch = cmdStr2Num(args[1], 10);
        if( ch == 1 )
            digitalWrite(enable0Pin, HIGH); 
        if( ch == 2 )
            digitalWrite(enable1Pin, HIGH);    
   }         
}

// =============================
// === E N A B L E   C M D ===
// =============================

// enable the output

void enableCmd(int arg_cnt, char **args)
{
   if( arg_cnt > 1 )
   {
        uint8_t ch = cmdStr2Num(args[1], 10);
        if( ch == 1 )
            digitalWrite(enable0Pin, LOW); 
        if( ch == 2)
            digitalWrite(enable1Pin, LOW);    
   } 
}


// ===============================
// === T R I A N G L E   C M D ===
// ===============================

// test command to send a triangle wave 'n' times ( 'T n' )

void triangleCmd(int arg_cnt, char **args)
{
    uint16_t nreps = cmdStr2Num(args[1], 10);

    Serial.println(nreps); 

   for( uint16_t rep = 0; rep < nreps; rep++)
   {
    
        for( int8_t i = -10; i < 10; i++)
        {
             setCurrent( 1, (float)i / 50.0);
             delay(10);
        }
        
        for( int8_t i = 10; i > -10; i--)
        {
             setCurrent( 1, (float)i / 50.0);
             delay(10);
        }      
    }
    setCurrent(0, 0.0); 
}

// =======================
// === Z E R O   C M D ===
// =======================

// set the DAC value that produces zero ouput current
// this is used to calibarate the system 
// Zero should be about 1626 counts
// Test by measuring the actual current output or monitoring the 
// drive voltage (at the Voltage Monitor BNC). Send 'Z n' with n being
// the new zero value. 
// NOTE: This value is not saved at power down

void zeroCmd(int arg_cnt, char **args)
{

     if( arg_cnt > 1 )
    {
        uint16_t zero = cmdStr2Num(args[1], 10);
        if( (zero > 1500) && (zero < 1800))
        {
          calZero = zero;
        }
    }
    else
    {
       Serial.println(calZero);
    } 
}  

// =======================
// === H E L P   C M D ===
// =======================
// sends version info, zero value, and list of commands

void helpCmd(int arg_cnt, char **args) 
{
    Stream *s = cmdGetStream();
    s->print("Mag Tweezers V:");
    s->println(VERSION);
#ifdef DEBUG       
    s->println("Debug ON");    
#endif
    cmdList();
}  

// ==================
// === S E T U P  ===
// ==================

// normal Arduino setup - do all inits

void setup() 
{
  Serial.begin(115200);

  SPI.begin();

  dac.begin(DACCS);     // 
  dac.setRange(1, DAC_BIP5);
  dac.setRange(2, DAC_BIP5);

  pinMode(enable0Pin, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(TTLinPin, INPUT);
  pinMode(TTLoutPin, OUTPUT);
  digitalWrite(enable0Pin, HIGH);  // start disabled
  digitalWrite(enable1Pin, HIGH);  // start disabled
  digitalWrite(TTLoutPin,LOW);
  setCurrent(1, 0.0);
  setCurrent(2, 0.0);
  cmdInit(&Serial);
  cmdAdd("I", ICmd);
  cmdAdd("L", listCmd);
  cmdAdd("R", rampCmd);
  cmdAdd("D", disableCmd);
  cmdAdd("E", enableCmd);
  cmdAdd("T", triangleCmd);
  cmdAdd("C", CCmd);
  cmdAdd("Z", zeroCmd);
  cmdAdd("S", sineCmd);
  cmdAdd("?", helpCmd);

  while(!Serial);

 // digitalWrite(enablePin, LOW);  // enable
}

// ================
// === L O O P  ===
// ================

// normal Arduino loop, this code is driven by command prompts

void loop() 
{
    cmdPoll();

    if( sineTrigFlag )
    {
      if( digitalReadFast(TTLinPin) == HIGH )
      {
          if(sineStartedFlag == false )
          {
            sineTimer.begin( sineStep, (1000000.0 / (freq * 180.0)) );   
            sineStartedFlag = true;    
          }  
      }
      else
      {
          setCurrent( sineChannel, 0);
          sineTimer.end(); 
          sineStartedFlag = false;
      }
    }

    if( rampTrigFlag )
    {
      if( digitalReadFast(TTLinPin) == HIGH )
      {
          if( rampStartedFlag == false)
          {
             startRamp(rampChannel);  
             rampStartedFlag = true; 
          }   
      }
      else
      {
          setCurrent( rampChannel,0);
          rampTimer.end();
          rampStartedFlag = false;
      }
    }

   if( listTrigFlag )
    {
      if( digitalReadFast(TTLinPin) == HIGH )
      {
          if( listStartedFlag == false)
          {
             startList(listChannel);  
             listStartedFlag = true; 
          }   
      }
      else
      {
          setCurrent( listChannel,0);
          listTimer.end();
          listStartedFlag = false;
      }
    }
    
}
