#ifndef QUICKLIB_H
#define QUICKLIB_H

// True = No floating-point support...
#define SMALL_GetUserVal 0  
// ASCII IIC protocol
#define USE_ATLAS_PRTCL 1
// Hardware RTC
#define USE_RTC 0
// Debug Values...
#define USE_DEBUG_VALS 0

// IIC IO/Buffer & Global HLP Strings
#define IIC_STR_LEN 34
#define STR_HLP_LEN 17


#include <Arduino.h>
#include <Wire.h>
#include "quickESC.h"
#include "quickTIME.h"
#include "quickIIC.h"
#include "quickPRINT.h"

// Seconds between RTC sync's (0 disables sync
#define syncRTCinterval 86400L

// ESC / Terminal hacks
#define ESC_CAN_FAINT 0
#define ESC_SOLARIZED 1

// Access PROGMEM variables like F() for Serial.print
// array of strings (char[][])
#define Fa(var) strcpy_P(strHLP, (PGM_P)pgm_read_word(&(var)))
#define FaStrange(var) strcpy_P(strHLP, (PGM_P)var)
//char *Fa(PGM_P strIN);

// single string (char[])
#define Fc(var) (__FlashStringHelper*)(var)
// byte[]
#define Fb(var) (byte)pgm_read_byte(&(var))
// int[]
#define Fi(var) (int)pgm_read_word(&(var))


// Declarations Declarations Declarations Declarations Declarations Declarations Declarations Declarations

byte IsSerialSpeedValid(uint32_t speed);

byte GetONEchar();

#if SMALL_GetUserVal
  byte IntToStr_SMALL(long val, char cntLeadingChar, char leadingChar);
  #define IntToIntStr(val, cntLeadingChar, leadingChar) IntToStr_SMALL(val, cntLeadingChar, leadingChar)
#else
  byte IntToStr_BIG(long val, int8_t lz, byte dp, char lc);
  #define IntToIntStr(val, cntLeadingChar, leadingChar) IntToStr_BIG((long)((long)(val) * 1000), (uint8_t)(cntLeadingChar), 0, (char)(leadingChar))
  #define IntToFloatStr(val, cntLeadingChar, cntDecimalPlaces, leadingChar) IntToStr_BIG((long)(val), (uint8_t)(cntLeadingChar), (byte)(cntDecimalPlaces), (char)(leadingChar))
#endif

byte getBit(byte byteIN, byte bitToGet);
byte setBit(byte byteIN, byte bitToSet, byte setTo);

#if SMALL_GetUserVal
    // just integer
    long GetUserInt(long valIN);
#else
    long GetUserVal(long defVal, int8_t type);
    #define GetUserInt(valIN) GetUserVal(valIN, 0)
    #define GetUserFloat(valIN) GetUserVal(valIN, 1)
#endif      
#if SMALL_GetUserVal
#else
  //long StrToInt(char *strIN, byte next);
  long StrTokFloatIntToInt(char *strIN, int8_t autoScale);
  #define StrTokFloatToInt(strIN) StrTokFloatIntToInt(strIN, -1)
  #define StrTokIntToInt(strIN) StrTokFloatIntToInt(strIN, 0)
  long StrFloatIntToInt(char *strIN, int8_t autoScale);
  #define StrFloatToInt(strIN) StrFloatIntToInt(strIN, -1)
  #define StrIntToInt(strIN) StrFloatIntToInt(strIN, 0)
#endif

byte GetUserString(char *strIN);

byte ToBCD (byte val);
byte FromBCD (byte bcd);
char ByteToChar(byte valIN);

uint32_t GetUserTime(uint32_t timeIN);
uint32_t GetUserDate(uint32_t timeIN);

char GetUserKey(byte maxChar, byte noCnt);


// Globals Globals Globals Globals Globals Globals Globals Globals Globals Globals Globals Globals Globals

// RTC-Temp
extern long myRtcTemp;

// global IIC I/O buffer and HLP-Strings
extern char iicStr[IIC_STR_LEN];
extern char strHLP[STR_HLP_LEN];
extern char strHLP2[STR_HLP_LEN];
extern char strDefault[STR_HLP_LEN];
extern byte adrDefault;

// ModBusAddress
extern byte myAddress;

struct mySTRUCT{
  // 12 Byte
  byte Boot;        // = 0;    // 0 = Terminal  /  1 = RTU  /  2 = ASCII  /  3 = just-values
  uint32_t Speed;   // = 9600;
  byte Solarized;   // = 0;
  byte Address;     // = 123;
  byte Default;     // = 0;
  byte Cnt;         // = 0;
  byte Setting;     // = 0;
  byte Temporary;   // = 0;   ??? Is this manual ???
  byte KeyColor;
};
extern mySTRUCT my;

#endif /* QUICKLIB_H */