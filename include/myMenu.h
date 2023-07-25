#ifndef MYMENU_H
#define MYMENU_H

#include "quicklib.h"
#include <EEPROM.h>
#include "myAir.h"

void (*softReset)(void) = 0;


// my Eeprom - Variables (Def. in quicklib.h)
mySTRUCT my;

// Helper for action-port refresh
  byte myLastLine = 0;
  byte portStateFirstRun = 0;

void myToRom(){
  // 12 Byte
  EEPROM.put(1000, my);
  // 1001 is next...
}
void myFromRom(){
  // 1001 is next...
  EEPROM.get(1000, my);
  if (!IsSerialSpeedValid(my.Speed)){
    my.Speed = 9600;
  }
  /*
  if (!myAddress || myAddress > 254){
    myAddress = 123;
  }
  */
  if (my.Setting > 2){
    my.Setting = 0;
    my.Default = 0;
  }
  if (my.Temporary > 3){
    // Manual (Prefill) Settings
    my.Temporary = 0;
    my.Default = 0;
  }
  if (my.Cnt > EZO_MAX_PROBES){
    my.Cnt = 0;
    my.Default = 0;
  }
  
  fgFaint = my.Solarized;

}

void DummyNameToStrDefault(void){
  strcpy_P(strDefault,(PGM_P)F( "-DummyName-"));
  //strcpy(strDefault, "-DummyName-");
}
void EditAutoName(){
  GetUserString(strDefault);
}
//#define EditAutoName() (GetUserString(strDefault))
void EditAutoAddress(){
  adrDefault = GetUserInt(adrDefault);
  if (adrDefault > 127 - ezoCnt){
    adrDefault = 127 - ezoCnt;
  }
  else if (adrDefault < 32){
    adrDefault = 32;
  }
}

byte PrintQuickAir(){
  return PrintMenuTop((char*)"- QuickAir 1.01 -");
}

byte PrintAllMenuOpt1(byte pos){

  EscLocate(5, pos++);
  PrintMenuKeyStd('a'); Serial.print(F("Reset     "));
  PrintMenuKeyStd('b'); Serial.print(F("Clear Calibration(s)     "));
  PrintMenuKeyStd('c'); Serial.print(F("Delete Name(s)"));
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStd('d'); Serial.print(F("(Auto-)Name = "));
  EscBold(1);
  Serial.print((char*)strDefault);
  EscLocate(42, pos++);
  PrintMenuKeyStd('e'); Serial.print(F("Do (Auto-)Name(s)"));
  PrintShortLine(pos++, 8);
  EscLocate(5, pos++);
  PrintMenuKeyStd('f'); Serial.print(F("(1st) (Auto-)Address = "));
  EscBold(1);
  IntToIntStr(adrDefault, 3, '0');
  Serial.print((char*)strHLP);
  PrintSpaces(5);
  PrintMenuKeyStd('g'); Serial.print(F("Do (Auto-)Address(es)"));
    
  return pos;

}

byte SwitchAllAndProbeMenu(int8_t pos, byte ezo, byte all){
  
  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // Return
    break;
  case 'a':
    // Factory Reset for All
    EzoReset(ezo, all);
    break;
  case 'c':
    // Delete all names
    EzoSetName((char*)"", ezo, all, 0);
    break;
  case 'd':
    // Edit Auto Name
    EditAutoName();
    break;
  case 'e':
    // Set AutoNames
    EzoSetName(strDefault, ezo, all, 1);
    break;
  case 'f':
    // Edit 1st Address
    EditAutoAddress();
    break;
  case 'g':
    // Set Addresses
    EzoSetAddress(ezo, adrDefault, all);
    // wait 4 reboots done
    delay(1000);
    // Scan new
    EzoScan();
    
    // SetAutoAddress();
    // direct back to main
    pos = 0;
    break;
  case 'b':
    // Clear calibration
    EzoSetCal((char*)"clear", ezo, all, 0, 5);
    break;
  default:
    return 0;
    break;
  }
  return 1;
}

void PrintProbeLine(byte ezo, byte pos, byte bold, byte noKey){

    EscLocate(5, pos);
    if (!noKey){
      // Print Menu-Key
      PrintMenuKey(ezo + '1', 0, '(', 0, 0, bold, !bold);
      EscCursorLeft(1);
      //EscLocate(10, pos);
    }
    if (bold){
      EscBold(1);
    }
    else{
      EscFaint(1);
    }
    
    EscCursorRight(3);
    // Name
    Serial.print((char*)ezoProbe[ezo].name);
    EscLocate(26, pos);
    PrintSpacer(bold);
    // Value
    IntToFloatStr(ezoValue[ezo][0], 5, 2, ' ');
    SetAvgColorEZO(ezoProbe[ezo].type);
    if (!bold){
      EscFaint(1);
    }
    Serial.print(strHLP);
    EscBoldColor(0);
    EscFaint(1);
    Print1Space();
    Serial.print((char*)Fa(ezoStrUnit[ezoProbe[ezo].type]));
    //PrintFa(Fa(ezoStrUnit[ezoProbe[ezo].type]));
    EscLocate(41, pos);
    PrintSpacer(0);
    // Type
    Serial.print((char*)Fa(ezoStrType[ezoProbe[ezo].type]));
    //PrintFa(Fa(ezoStrType[ezoProbe[ezo].type]));
    EscLocate(48, pos);
    PrintSpacer(0);
    // Version
    IntToFloatStr(ezoProbe[ezo].version, 2, 2, '0');
    Serial.print(strHLP);
    EscLocate(56, pos);
    PrintSpacer(0);
    // Address
    IntToIntStr(ezoProbe[ezo].address, 3, ' ');
    Serial.print(strHLP);
    EscLocate(62, pos);
    PrintSpacer(ezoProbe[ezo].calibrated);
    // Calibrated
    Serial.print(ezoProbe[ezo].calibrated);
    EscBold(0);
    PrintSpacer(0);

}
int8_t PrintProbesOfType(byte ezo, byte all, int8_t pos, byte noKey){

  byte veryAll = 0;
  if (ezo == 255){
    // Print all types
    veryAll = 1;
    ezo = 0;
  }
  
  PrintLine(pos, 5, 63);

  for (int i = 0; i < ezoCnt; i++){  
    if ((ezoProbe[i].type == ezoProbe[ezo].type) || veryAll) {
      // Right Probe Type
      pos++;
      PrintProbeLine(i, pos, (i == ezo) || all, noKey);
    }    
  }
  EscFaint(0);
  pos++;
  PrintLine(pos++, 5, 63);
  
  return pos;
}

void PrintAllMenu(){

  adrDefault = 33;
  DummyNameToStrDefault();

Start:

  int8_t pos = PrintMenuTop((char*)"- ALL Menu -");
  
  pos = PrintProbesOfType(255, 1, pos, 1);
  pos = PrintAllMenuOpt1(pos);
  
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStdBoldFaint('h', my.Boot, !my.Boot);
  Serial.print(F("Boot As ModBUS Slave     "));
  EscFaint(0);
  PrintMenuKeyStdBoldFaint('i', !my.Boot, my.Boot);
  Serial.print(F("Boot For Terminal Use"));
  EscFaint(0);
  
  PrintMenuEnd(pos + 2);

  pos = GetUserKey('i', -1);

  if (!SwitchAllAndProbeMenu(pos, 0, 2)){
    switch (pos){
    case 'h':
      // Boot As Slave
      my.Boot = 1;
      myToRom();
      break;
    case 'i':
      // Boot For Terminal
      my.Boot = 0;
      myToRom();
      break;
    default:
      break;
    }
  }
  
  if (pos > 0){
    goto Start;
  }
  
}

void PrintPointEqual(uint8_t pos, int32_t value){
  Serial.print (F("-Pt. "));
  EscLocate(29, pos);
  Serial.print (F("= "));
  PrintBoldFloat(value, 4, 2, ' ');
}
void PrintCalMenu(byte ezo, byte all){

  byte ImInside = 0;

  int32_t calLow = 0;      // Value for LowPoint
  int32_t calMid = 0;      // Value for MidPoint
  int32_t calHigh = 0;     // Value for HighPoint
  // long calRes = 0;      // Value for Reset
  int32_t calAvg = 0;      // Actual avg of actual ezoType

  int32_t calTemp = avg_RTD;
  int32_t calVal[3];
  byte calCnt = 1;
  byte calAction[3];

  struct myMenu{
    byte a:1;
    byte b:1;
    byte c:1;
    byte d:1;
    byte e:1;
    byte f:1;
    byte g:1;
  }myMenu;
  
  myMenu.a = 0;
  myMenu.b = 0;
  myMenu.c = 0;
  myMenu.d = 0;
  myMenu.e = 0;
  myMenu.f = 0;
  myMenu.g = 0;

  Start:

  strcpy_P(iicStr,(PGM_P)F( "- Calibrate "));
  //strcpy(iicStr, "- Calibrate ");

  switch (ezoProbe[ezo].type){
  case ezoRTD:
    // RTD
    strcpy_P(&iicStr[12], (PGM_P)F("RTD -"));
    if (!ImInside){
      myMenu.a = 1;
      myMenu.d = 1;
      myMenu.e = 1;
      calLow = CAL_RTD_LOW;
      calMid = CAL_RTD_MID;
      calHigh = CAL_RTD_HIGH;
      calAvg = avg_RTD;
    }
    break;
  case ezoHUM:
    // ezoHUM has no calibration on board !
    strcpy_P(&iicStr[12], (PGM_P)F("HUM -"));
    break;
  case ezoCO2:
    // CO2
    strcpy_P(&iicStr[12], (PGM_P)F("CO2 -"));
    if (!ImInside){
      myMenu.a = 1;
      myMenu.b = 1;
      myMenu.d = 1;
      myMenu.f = 1;
      calLow = CAL_CO2_LOW;
      calMid = CAL_CO2_MID;
      calHigh = CAL_CO2_HIGH;
      calAvg = avg_CO2;
    }
    break;
  default:
    break;
  }
  
  int8_t pos = PrintMenuTop(iicStr);
  
  if (!ImInside){
    EzoReset(ezo, all);
  }  
  ImInside = 1;

  pos = PrintProbesOfType(ezo, all, pos++, 1);
//  pos++;

  EscLocate(5, pos++);
  if (myMenu.a){
    // 1-Point Cal.
    PrintMenuKeyStd('a'); Serial.print(F("Do 1-Pt. Cal.     "));
    //PrintSpaces(5);
  }
  if (myMenu.b){
    // 3-Point Cal.
    PrintMenuKeyStd('b'); Serial.print(F("Do 2-Pt. Cal.     "));
    //PrintSpaces(5);
  }
  if (myMenu.c){
    // 3-Point Cal.
    PrintMenuKeyStd('c'); Serial.print(F("Do 3-Pt. Cal."));
  }
  //pos++;
  pos = PrintShortLine(pos, 8);

  if (myMenu.d){
    // Set Single/Mid Point
    EscLocate(5, pos);
    PrintMenuKeyStd('d'); Serial.print(F("Set Single/Mid"));
    PrintPointEqual(pos++, calMid);
  }
  if (myMenu.e){
    // Use Avg as Single/Mid Point
    EscLocate(5, pos);
    PrintMenuKeyStd('e'); Serial.print(F("Set AVG As Mid"));
    PrintPointEqual(pos++, calAvg);
  }
  if (myMenu.f){
    // Set Low Point
    EscLocate(5, pos);
    PrintMenuKeyStd('f'); Serial.print(F("Set Low"));
    PrintPointEqual(pos++, calLow);
}
  if (myMenu.g){
    // Set High Point
    EscLocate(5, pos);
    PrintMenuKeyStd('g'); Serial.print(F("Set High"));
    PrintPointEqual(pos++, calHigh);
  }
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('g', 0);
  
  calCnt = 1;
  calTemp = 0;
  calAction[0] = 0;
  calAction[1] = 0;
  calAction[2] = 0;
  calVal[0] = 0;
  calVal[1] = 0;
  calVal[2] = 0;

  switch (pos){
  case 'a':
    // 1-Pt. Cal
    if (myMenu.a){
      calTemp = PrintValsForCal(ezo, all, calMid);
      if (calTemp){
        calAction[0] = 4;
        calVal[0] = calMid;
      }      
    }
    break;
  case 'b':
    // 2-Pt. Cal
    // just CO2
    calCnt = 2;
    if (myMenu.b){
      calTemp = PrintValsForCal(ezo, all, calLow);
      if (calTemp){
        // "Cal,0"
        // "Cal,value"
        calAction[0] = 4;
        calVal[0] = calLow;
        calAction[1] = 4;
        calVal[1] = calMid;
      }      
    }  
    break;
  case 'c':
    // 3-Pt. Cal
    // QuickAir Probes don't have 3-Point calibrations
    break;
  case 'd':
    calMid = GetUserFloat(calMid);
    break;
  case 'e':
    calMid = calAvg;
    break;
  case 'f':
    calLow = GetUserFloat(calLow);
    break;
  case 'g':
    calHigh = GetUserFloat(calHigh);
    break;
  default:
    break;
  }

  if (calTemp){
    EzoSetCal((char*)"", ezo, all, calVal[0], calAction[0]);
    for (byte i = 1; i < calCnt; i++){
      calTemp = PrintValsForCal(ezo, all, calVal[i]);
      if (calTemp){
        EzoSetCal((char*)"", ezo, all, calVal[i], calAction[i]);
      }
      else{
        //ESC
        i = calCnt;
      }
    } 
    calCnt = ezoCnt;
    EzoScan();
    if (ezoCnt < calCnt){
      // not all modules recognized
      // sometimes needed...
      EzoScan();
    }  
    if (my.Default){
      DefaultProbesToRom();
    }
  }

  if (pos > 0){
    goto Start;
  }
  
}

void PrintProbeMenu(byte ezo){

  byte all = 0;

    DummyNameToStrDefault();
    adrDefault = 33;

Start:


  int8_t pos = PrintMenuTop((char*)"- Probe(Type) Menu -");
  
  pos = PrintProbesOfType(ezo, all, pos, 0);

  pos = PrintAllMenuOpt1(pos + 1);
  PrintShortLine(pos++, 8);
  EscLocate(5, pos);
  PrintMenuKeyStd('h'); Serial.print(F("Calibration(s)..."));
  EscLocate(30, pos);
  PrintMenuKeyStdBoldFaint('i', !all, all); 
  Serial.print(F("Select Single"));
  EscLocate(51, pos++);
  PrintMenuKeyStdBoldFaint('j', all, !all);
  Serial.print(F("Select ALL"));
  EscBold(0);

  PrintMenuEnd(pos);

  pos = GetUserKey('j', 9);

  if (!SwitchAllAndProbeMenu(pos, ezo, all)){
    switch (pos){
    case 'h':
      PrintCalMenu(ezo, all);
      break;
    case 'i':
      // Single
      all = 0;
      break;
    case 'j':
      // All
      all = 1;
      break;
    default:
      // Select another...
      ezo = pos - 49;
      break;
    }
  }
   
  if (pos > 0){
    goto Start;
  }
  
}

void PrintFlexSpacer(byte leading, byte trailing){
  PrintSpaces(leading);
  PrintSpacer(1);
  PrintSpaces(trailing);
}
void PrintSmallSpacer(){
  PrintSpacer(0);
  EscCursorLeft(1);
}
void PrintSmallMenuKey(char key){
  PrintMenuKey(key, 0, 0, ' ', 0, 0, 0);
}
void PrintCenteredWithSpacer(char *strIN, byte centerLen){
  PrintSpacer(1);
  PrintCentered(strIN, centerLen);
}

void PrintMenuKeyLong(char *strIN){
  EscKeyStyle(1);
  Serial.print(strIN);
  EscKeyStyle(0);
}

void PrintLowToHigh(){
  Serial.print(F("tooLow   |    Low     |    High    |  tooHigh   |"));
}

int8_t PrintCopySettingTo(int8_t pos){
  EscLocate(5, pos++);
  
  PrintMenuKeyLong((char*)"1-3):");
  
  Serial.print(F(" Copy FULL SETTING "));
  EscColor(fgBlue);
  Serial.print((char*)setting.Name);
  Serial.print(F(" ("));
  Serial.print(my.Setting + 1);
  Serial.print(F(")"));
  EscColor(0);
  Serial.print(F(" to Setting-No. [1-3]"));
  return pos;
}

void PrintValuesMenuHlp(char key, byte i, uint32_t value){
  PrintSmallMenuKey(key + i);
  PrintFloat(value, 4, 2, ' ');
  PrintSpacer(0);
}
byte PrintValuesMenuChangeVal(int32_t *valIN){
  // THIS IS STRANGE
  //    Using this just two times has a lower flash use than
  //    if it's used more or less times as replacement for:
  //        pos -= 'a';
  //        setting.FailSaveValue[pos] = GetUserFloat(setting.FailSaveValue[pos]);
  //        pos = 1;
  // THIS IS STRANGE
  *valIN = GetUserFloat(*valIN);
  return 1;
}

byte IsKeyBetween(char key, char start, char stop){
  // THIS IS STRANGE
  //    Using this some times has a lower flash use than
  //    if it's used more or less times
  // THIS IS STRANGE
  return (key >= start && key <= stop);
}
//#define IsKeyBetween(key, start, stop) ((key >= start) && (key <= stop))

void PrintValuesMenu(){

//             |  FailSafe  |   tooLow   |    Low     |    High    |  tooHigh   |
//-------------------------------------------------------------------------------
//     RTD     | a)         | e)         | i)         | m)         | o)         |
//-------------------------------------------------------------------------------
//     HUM     | b)         | f)         | j)         | n)         | p)         |
//-------------------------------------------------------------------------------
//     CO2     | c)         | g)         | k)         |
//-----------------------------------------------------
//     DEW     | d)         | h)         | l)         |


//   Offset   |   OnTime   |  OffTime   |
//---------------------------------------
//  a)        | b)         | c)         |

Start:

  int8_t pos = PrintMenuTop((char*)"- Set Values -") + 1;
  byte i = 0;
  
  EscLocate(12, pos++);
  PrintFlexSpacer(0, 1);
  Serial.print(F("FailSafe"));
  PrintFlexSpacer(1, 2);
  PrintLowToHigh();

  PrintLine(pos++, 3, 76);
  EscLocate(3, pos++);
  
  for (i = 0; i < 4; i++){
    
    EscBold(1);
    PrintCentered(Fa(ezoStrType[i]), 9);
    PrintSpacer(0);

    PrintValuesMenuHlp('a', i, setting.FailSaveValue[i]);

    PrintValuesMenuHlp('e', i, setting.ValueTooLow[i]);

    PrintValuesMenuHlp('i', i, setting.ValueLow[i]);

    if (i < 2){
      // DEW & CO2 have no High / tooHigh
      PrintValuesMenuHlp('m', i, setting.ValueHigh[i]);
      PrintValuesMenuHlp('o', i, setting.ValueTooHigh[i]);
    }
    
    EscLocate(3, pos++);

  }

  pos = PrintCopySettingTo(pos);
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('p', 3);

  if (pos < 1){
    // Exit & TimeOut
  }
  
  else if (IsKeyBetween(pos, 'a', 'd')){
    // FailSave
    pos = PrintValuesMenuChangeVal(&setting.FailSaveValue[pos - 'a']);
  }
  else if (IsKeyBetween(pos, 'e', 'h')){
    // tooLow
    pos = PrintValuesMenuChangeVal(&setting.ValueTooLow[pos - 'e']);
  }
  else if (IsKeyBetween(pos, 'i', 'l')){
    // Low
    pos = PrintValuesMenuChangeVal(&setting.ValueLow[pos - 'i']);
  }
  else if (IsKeyBetween(pos, 'm', 'n')){
    // High
    pos = PrintValuesMenuChangeVal(&setting.ValueHigh[pos - 'm']);
  }
  else if (IsKeyBetween(pos, 'o', 'p')){
    // tooHigh
    pos = PrintValuesMenuChangeVal(&setting.ValueTooHigh[pos - 'o']);
  }
  else if (IsKeyBetween(pos, '1', '2')){
    // Copy to Day / Night
    SettingsToRom(pos - '1'); 
    pos = 2;   
  }

  if (pos == 1){
    SettingsToRom(my.Setting);
  }
  
  if (pos > 0){
    goto Start;
  }
  
}


byte PrintTempToLevel(byte pos){
  EscBold(1);
  //Serial.print(F("|   Temp.    |    EC     |     pH     |    Redox    |    O2     |   Level  |"));
  Serial.print(F("|   TempRTD   |   TempHUM   |   Humidity   |     CO2      |  Dewing Point  |"));
  //              1234567890123456789012345678901234567890123456789012345678901234567890123456
  //                         21.50°C                100%         1200ppm         -10.00°C    
  //                 >~<    ~1~   ~2~   ~3~         >~<             >~               >~
  pos = PrintLine(pos, 3, 76);    
  EscBold(0);
  return pos;
}
void PrintPortStates(){

  // 4 low-ports / 2 high ports
  byte posOfPort[] = {7, 38, 53, 69, 9, 40};
  // Three "analog" values
  byte posOfVal[] = {15, 20, 25};

  static byte lastVal[9];
  byte isChanged = 0;

  // Check on Digital Port_Changes
  // 4 low-ports / 2 high-ports
  for (byte i = 0; i < 6; i++){
    byte val = digitalRead(i + 2);
    if (val != lastVal[i]){
      lastVal[i] = val;
      isChanged = 1;
    }
  }
  // Check on "analog" Port_Changes
  for (byte i = 6; i < 9; i++){
    byte val = 3;//stepRead(i - 6);
    if (val != lastVal[i]){
      lastVal[i] = val;
      isChanged = 1;
    }
  }
  
  if (isChanged || !portStateFirstRun){
    for (byte i = 0; i < 4; i++){
      // Low-Ports
      EscLocate(posOfPort[i], myLastLine);
      if (lastVal[i]){
        EscBoldColor(fgBlue);
        Serial.print(F(">"));
        EscFaint(1);
      }
      else{
        EscFaint(1);
        Serial.print(F(">"));
        EscBoldColor(fgGreen);
      }
      Serial.print(F("~"));
    }
    for (byte i = 4; i < 6; i++){
      // High-Ports
      EscLocate(posOfPort[i], myLastLine);
      if (lastVal[i]){
        EscBoldColor(fgYellow);
        Serial.print(F("<"));        
        EscCursorLeft(2);
        EscFaint(1);
        Serial.print(F("~"));
      }
      else{
        EscFaint(1);
        Serial.print(F("<"));        
      }
    }

    // Loop the Analog Ports
    
    for (byte i = 6; i < 9; i++){
      // Analog-Ports
      EscLocate(posOfVal[i - 6], myLastLine);
      SetAvgColorEZO(ezoRTD);
      EscBold(1);
      Serial.print(lastVal[i]);
      EscColor(0);
      EscBold(0);
      Serial.print(F("~"));
      EscCursorLeft(3);
      Serial.print(F("~"));
    }
    
    portStateFirstRun = 1;
    EscBoldColor(0);
  }
}


void RunManualSetting(byte port, byte style){

  struct runManualSTRUCT{
      uint16_t runTime;
      uint16_t offset;
      uint16_t onTime;
      uint16_t offTime;
  }manualTiming[9];

  byte idToPort[] = {2, 0, 0, 0, 3, 4, 5, 6, 7};

  int8_t pos = PrintMenuTop((char*)"- RUN Manual -") + 1;
  EscLocate(3, pos);
  pos++;

  pos = PrintTempToLevel(pos);
  PrintLine(pos + 1, 3, 76);    

  // we need this pos in "loop()" / see PrintPortStates()
  myLastLine = pos;
  portStateFirstRun = 0;

  // Copy Low & High values to manualTiming array
  for (byte i = 0; i < 7; i++) {
    manualTiming[i].runTime = manual.LowPort[i];
  }
  // the other 3 highs are "analog" values for their low-times
    manualTiming[7].runTime = manual.HighPort[0];
    manualTiming[8].runTime = manual.HighPort[4];

  // Search longest time
  uint16_t maxTime = 0;
  for (byte i = 0; i < 9; i++){
    if ((i == port || port == 255) && manualTiming[i].runTime){
      // Port is in action...
      if (manualTiming[i].runTime > maxTime){
        maxTime = manualTiming[i].runTime;
      }
    }
  }
  
  // Calc offset / onTime / offTime
  for (byte i = 0; i < 9; i++){

    uint16_t repeats = 0;

    if ((i == port || port == 255) && manualTiming[i].runTime){

      // we're in Action and have a time...
      switch (style){
      case 0:
        // Distributed

        repeats = (maxTime / manualTiming[i].runTime);
        
        if (repeats > manualTiming[i].runTime){
          // we can't On/Off shorter than 1sec.
          repeats = manualTiming[i].runTime;
        }
        
        ReCalc:   // sucking integer resolution need this up & down
                  // to get total onTime exact but also as distributed as possible
        manualTiming[i].onTime = manualTiming[i].runTime / repeats;
        if ((manualTiming[i].onTime * repeats) < manualTiming[i].runTime){
          // onTime too short - raise onTime (suck - 1)
          manualTiming[i].onTime++;
        }
        if ((manualTiming[i].onTime * repeats) > manualTiming[i].runTime){
          // onTime too long - lower repeats (suck - 2)
          if (repeats){
            repeats--;
            goto ReCalc;
          }          
        }

        manualTiming[i].offTime = (maxTime - manualTiming[i].runTime) / repeats;               
        manualTiming[i].offset = (maxTime - ((manualTiming[i].onTime + manualTiming[i].offTime) * repeats)) / 2;
        break;
      case 1:
        // Centered
        repeats = 1;
        manualTiming[i].onTime = manualTiming[i].runTime;
        manualTiming[i].offTime = maxTime - manualTiming[i].runTime;
        manualTiming[i].offset = 0;
        break;
      default:
        break;
      }
      if (!manualTiming[i].offset){
        manualTiming[i].offset = manualTiming[i].offTime / 2;
      }
    }
  }

  // Run times...
  uint16_t runTime = 0;
  maxTime++;
  while (maxTime){
    if (DoTimer()){
      // A second is gone...

      maxTime--;

      // Time left as PrintErrOK...
      PrintSerTime(maxTime, 0, 0); // Time left to strHLP2
      strcpy(&strHLP2[8], (char*)" left...");
      PrintErrorOK(0, strlen(strHLP2), strHLP2);

      for (byte i = 0; i < 9; i++){

        byte portState = 0;
        if ((i == port || port == 255) && manualTiming[i].runTime){
          // port is valid & timing exist

          if (runTime >= manualTiming[i].offset ){
            // Offset is expired - calc interval...
            portState = ((runTime - manualTiming[i].offset) % (manualTiming[i].onTime + manualTiming[i].offTime) < manualTiming[i].onTime);
          }
          else{
            // Offset is still active
          }
        }
        if (i && i < 4){
          // Exhaust / Intake / Circulation ("analog" - ports)
        }
        else{
          // regular high/low ports
          digitalWrite(idToPort[i], portState);
        }        
      }
      runTime++;

      PrintLoopTimes();    
      PrintPortStates();

    }
    if (Serial.available()){
      // STOP manual action...
      Serial.read();
      maxTime = 0;
    }
  }
  OffOutPorts();

}
void PrintManualMenuHlp1(char key, uint16_t value, byte spacer, byte isTime, byte key2){
  PrintMenuKeySmallBoldFaint(key, 0, !value);
  if (isTime){
    // Time 00:00:00
    PrintSerTime(value, 0, 1);
  }
  else{
    /* value */
    PrintInt(value, 8, ' ');
  }  
  if (key2){
    PrintMenuKey(key - 32, 1, '(', 0, 0, !value, !value);
  }
  else{
    Serial.print(F("   "));
  }
    
  PrintSpacer(spacer);
}
byte GetUserTime16ptr(uint16_t *valIN, byte isTime){
  // THIS IS STRANGE
  //    Using this just two times has a lower flash use than
  //    if it's used more or less times as replacement for:
  //        pos -= 'a';
  //        setting.FailSaveValue[pos] = GetUserFloat(setting.FailSaveValue[pos]);
  //        pos = 1;
  // THIS IS STRANGE
  if (isTime){
    *valIN = GetUserTime(*valIN);
  }
  else{
    *valIN = GetUserInt(*valIN);
  }
  
  return 1;
}

void PrintManualMenu(){

// m) SomeBlaBlaName     |     Low    |     High   |   Value    |
//---------------------------------------------------------------
//          RTD          | a)         | h)         |
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//         Exhaust       | b)         |            | i)         |
//         Intake        | c)         |            | j)         |
//         Circ.         | d)         |            | k)         |
//---------------------------------------------------------------
//          HUM          | e)         | l)         |
//--------------------------------------------------
//          CO2          | f)         | 
//-------------------------------------
//          DEW          | g)         |   

Start:

  int8_t pos = PrintMenuTop((char*)"- Manual Menu -") + 1;
  byte i = 0;
  
  EscLocate(5, pos++);
  PrintMenuKey(i + 'o', 0, '(', ' ', 0, 1, 0);
  EscColor(fgBlue);
  PrintCentered(manual.Name, 16);
  EscColor(0);
  Serial.print(F(" |       LOW       |      HIGH       |      Value      |"));
  PrintLine(pos++, 5, 73);
  byte ecCnt = 0;

  for (i = 0; i < 7; i++){

    EscLocate(8, pos++);
    EscBold(1);
    PrintCentered(Fa(ezoStrTimeType[i]), 17);
    PrintSpacer(1);
    
    PrintManualMenuHlp1('a' + i + ecCnt, manual.LowPort[i + ecCnt], 1, 1, 1);

    if (i == 0 || i == 4){
      // RTD & Hum - High Ports (Time)
      PrintManualMenuHlp1('h' + i, manual.HighPort[i], 1, 1, 1);
    }
    else if (i > 0 && i < 4){
      // Exhaust / Intake / Circulation
      PrintFlexSpacer(15,0);
      PrintManualMenuHlp1('h' + i, manual.HighPort[i], 1, 0, 0);
    }

  }

  PrintLine(pos++, 5, 73);
  pos++;
  EscLocate(5, pos++);
  
  PrintMenuKeyLong((char*)"a-l):");
  
  Serial.print(F(" Edit   "));
  
  PrintMenuKeyLong((char*)"A-H & L):");
  
  Serial.print(F(" RunSingle   "));
  PrintMenuKeyStd('m');
  Serial.print(F("RunALL   "));
  PrintMenuKeyStd('n');
  Serial.print(F("RunAllCent."));
    
  PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  PrintMenuKeyStd('o');
  Serial.print(F("EditName   "));
  
  PrintMenuKeyLong((char*)"1-4):");
  
  Serial.print(F(" SelectSet = "));
  EscColor(fgBlue);
  EscBold(1);
  Serial.print(my.Temporary + 1);
  EscColor(0);
  PrintSpaces(3);
  
  PrintMenuKeyLong((char*)"5-8):");
  
  Serial.print(F(" CopyToSet [1-4]"));
  
  PrintMenuEnd(pos + 1);

  pos = GetUserKey('o', 8);

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (IsKeyBetween(pos, 'a', 'g')){
    // LowTime
    pos = GetUserTime16ptr(&manual.LowPort[pos - 'a'], 1);
  }
  else if (IsKeyBetween(pos, 'i', 'k')){
    // HighTime (Values)
    pos = GetUserTime16ptr(&manual.HighPort[pos - 'h'], 0);
  }
  else if (pos == 'h' || pos == 'l'){
    // HighTime - Times
    pos = GetUserTime16ptr(&manual.HighPort[pos - 'h'], 1);
  }
  else if (IsKeyBetween(pos, 'A', 'G')){
    // Run Single LowTime
    pos -= 'A';
    RunManualSetting(pos, 1);
    pos = 2;
  }
  else if (pos == 'H' || pos == 'L'){
    // Run Single HighTime
    pos = pos - 'H' + 6;
    RunManualSetting(pos, 1);
    pos = 2;
  }
  else if (pos == 'm'){
    // Run Distributed
    RunManualSetting(255, 0);
    pos = 2;
  }
  else if (pos == 'n'){
    // Run Centered
    RunManualSetting(255, 1);
    pos = 2;
  }
  else if (IsKeyBetween(pos, '1', '4')){
    // Load Set
    my.Temporary = pos - '1';
    ManualTimesFromRom(my.Temporary);
    myToRom();
  }
  else if (IsKeyBetween(pos, '5', '8')){
    // Copy Set
    ManualTimesToRom(pos - '5');
    pos = 2;
  }
  else if (pos == 'o'){
    // Edit Name
    GetUserString(manual.Name);
    // strcpy(manual.Name, strHLP);
    pos = 1;
  }
  
  
  if (pos == 1){
    ManualTimesToRom(my.Temporary);
  }
  
  if (pos > 0){
    goto Start;
  }

  OffOutPorts();
  
}

void PrintTimingsMenuTime(char key, uint16_t timeIN, byte printSpacer, byte isTime){
  PrintMenuKeySmallBoldFaint(key, 0, !timeIN);
  if (isTime){
    PrintSerTime(timeIN, 0, 1);
  }
  else{
    IntToIntStr(timeIN, 6, ' ');
    Serial.print(strHLP);
    PrintSpaces(2);
  }
  if (printSpacer){
    PrintSmallSpacer();
  }
}

void PrintTimingsMenu(){

//          | DelayTimes |   tooLow   |    Low     |    High    |  tooHigh   |
//----------------------------------------------------------------------------
//   RTD    | a)         | h)         | o)         | v)         | A)         |
//----------------------------------------------------------------------------

//          |     OK     |   tooLow   |    Low     |    High    |  tooHigh   |
//----------------------------------------------------------------------------
//  Exhaust | b)         | i)         | p)         | w)         | B)         |
//  Intake  | c)         | j)         | q)         | x)         | C)         |
//  Circ.   | d)         | k)         | r)         | y)         | D)         |
//----------------------------------------------------------------------------

//          | DelayTimes |   tooLow   |    Low     |    High    |  tooHigh   |
//----------------------------------------------------------------------------
//   HUM    | e)         | l)         | s)         | z)         | E)         |
//   CO2    | f)         | m)         | t)         | 
//   DEW    | g)         | n)         | u)         | 

Start:

  int8_t pos = PrintMenuTop((char*)"- Set Timings & Steps -") + 1;
  byte i = 0;

  
  for (i = 0; i < 7; i++){

    switch (i){
    case 4:
      pos++;
    case 0:
      EscLocate(12, pos++);
      PrintSpacer(1);
      Serial.print(F("DelayTimes"));
      PrintFlexSpacer(0, 2);
      PrintLowToHigh();
      PrintLine(pos++, 3, 76);
      EscLocate(3, pos++);
      break;    
    default:
      EscLocate(3, pos);
      break;
    }

    byte j = (!i || i > 3);
    
    EscBold(1);
    PrintCentered(Fa(ezoStrTimeType[i]), 9);
    PrintSmallSpacer();
    PrintTimingsMenuTime(i + 'a', setting.DelayTime[i], 0, j);
    PrintSmallSpacer();
    PrintTimingsMenuTime(i + 'h', setting.TimeTooLow[i], 1, j);
    PrintTimingsMenuTime(i + 'o', setting.TimeLow[i], 1, j);
    
    if (i < 5){
      // Has High Times
      PrintTimingsMenuTime(i + 'v', setting.TimeHigh[i], 1, j);
      PrintTimingsMenuTime(i + 'A', setting.TimeTooHigh[i], 0, j);
      PrintSmallSpacer();
    }
    else{
      // No HighTimes
      // PrintFlexSpacer(11, 10);
      
    }
    
    EscLocate(3, pos);

    switch (i){
    case 0:
      pos = PrintLine(pos, 3, 76);
      pos++;
      EscLocate(12, pos++);
      PrintSpacer(1);
      Serial.print(F(" OK-Step  "));
      PrintFlexSpacer(0, 2);
      PrintLowToHigh();
      pos = PrintLine(pos++, 3, 76);
      break; 
    case 3: 
      pos++;  
      pos = PrintLine(pos, 3, 76);
      break;
    case 4:
      break;
    default:
      pos++;
      break;
    }
  }
  
  pos = PrintCopySettingTo(pos + 1);

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('z', 3);

  byte j = 1;

  switch (pos){
  case 'b'...'d':
  case 'i'...'k':
  case 'p'...'r':
  case 'w'...'y':
  case 'B'...'D':
    j = 0;
    break;
  default:
    break;
  }

  if (pos < 1){
    // Exit & TimeOut
  }
  else if (IsKeyBetween(pos, 'a', 'g')){
    // FailSave
    pos = GetUserTime16ptr(&setting.DelayTime[pos - 'a'], j);
  }
  else if (IsKeyBetween(pos, 'h', 'n')){
    // tooLow
    pos = GetUserTime16ptr(&setting.TimeTooLow[pos - 'h'], j);
  }
  else if (IsKeyBetween(pos, 'o', 'u')){
    // Low
    pos = GetUserTime16ptr(&setting.TimeLow[pos - 'o'], j);
  }
  else if (IsKeyBetween(pos, 'v', 'z')){
    // High
    pos = GetUserTime16ptr(&setting.TimeHigh[pos - 'v'], j);
  }
  else if (IsKeyBetween(pos, 'A', 'E')){
    // tooHigh
    pos = GetUserTime16ptr(&setting.TimeTooHigh[pos - 'A'], j);
  }
  else if (IsKeyBetween(pos, '1', '3')){
    SettingsToRom(pos - '1'); 
    pos = 2;   
  }
  
  if (pos == 1){
    SettingsToRom(my.Setting);
  }
  
  if (pos > 0){
    goto Start;
  }
  
}

void PrintUnit (byte ezotype, byte faint, byte leadingSpaces, byte trailingSpaces){
  // Prints Unit of ezoType
  // Adds optional leading and trailing spaces
  // Sets and resets faint
  PrintSpaces(leadingSpaces);
  if (faint){
    EscFaint(1);
  }
  else{
    EscColor(0);
  }
  
  Serial.print((char*)Fa(ezoStrUnit[ezotype]));
  //PrintFa(Fa(ezoStrUnit[ezotype]));
  PrintSpaces(trailingSpaces);
  EscFaint(0);
}

byte PrintWaterValsHlp(byte pos, byte posX, byte ezotype, byte lz, byte dp, int divisor){ //, long *avgExt){

  byte posAct = 0;
  long avg[] = {0, 0, 0};

  for (int i = 0; i < ezoCnt; i++){
    if (ezoProbe[i].type == ezotype){
      byte j = 0;
      posAct++;
      avg[0] += ezoValue[i][0];
      avg[1] += ezoValue[i][1];
      avg[2] += ezoValue[i][2];
      
      while (!j || (ezotype == ezoHUM && j < 3)){
        EscLocate(posX, pos);
        Serial.print(i + 1);
        Serial.print(F(": "));
        PrintBoldFloat(ezoValue[i][j] / divisor, lz, dp, ' ');
        if (ezotype == ezoHUM && j){
          posX += 45;
          PrintUnit(5 - j, 1, 0, 3);
        }
        else{
          PrintUnit(ezotype, 1, 0, 3);
          if (ezotype == ezoHUM){
            posX -= 14; // Shift for Temp of HUM
          }
        }
        j++;        
      }
      if (ezotype == ezoHUM){
        posX -= 76; // Reset for next HUM
      }
      pos++;
    }
  }

  if (posAct){
    avgVal[ezotype] = avg[0] / (long)posAct;
    if (ezotype == ezoHUM){
      // Humidity has three Values...
      avg_TMP = avg[1] / (long)posAct;
      avg_DEW = avg[2] / (long)posAct;
    }
  }

  return posAct;

}

/*
byte GetPosMax(byte posAct, byte posMax){
  //if (posAct > posMax){
    //return posAct;
  //}
  //return posMax;
  return (posAct > posMax) ? posAct : posMax;
}
*/
#define GetPosMax(posAct, posMax) ((posAct > posMax) ? posAct : posMax)

byte PrintWaterVals(byte pos){

  byte posMax = 0;
  byte posAct = 0;

  byte posX[] = {4, 32, 48};
  byte lz[] = {3, 3, 4};
  byte dp;
  int16_t divisor;

  for (byte i = 0; i < 3; i++){

    dp = 2;
    divisor = 1;
    if (i == ezoCO2){
      dp = 0;
      divisor = 1000;
    }
    
    posAct = PrintWaterValsHlp(pos, posX[i], i, lz[i], dp, divisor); //, &avg_EC);
    posMax = GetPosMax(posAct, posMax);
  }
  
  return pos + posMax;

}

void PrintAVGsHLP(byte type, byte posX, byte posY, byte preDot, byte printUnit){
  SetAvgColorEZO(type);
  EscLocate(posX, posY);
  PrintBoldFloat(avgVal[type], preDot, 2, ' ');
  if (printUnit){
    PrintUnit(type, 0, 0, 3);
  }
}
byte PrintAVGs(byte pos){

  PrintAVGsHLP(ezoRTD, 14, pos, 3, 1);  
  PrintAVGsHLP(ezoHUM, 35, pos, 3, 1);  

  SetAvgColorEZO(ezoCO2);
  EscLocate(51, pos);
  PrintBoldInt(avg_CO2 / 1000, 4, ' ');
  PrintUnit(ezoCO2, 0, 0, 3);

  PrintAVGsHLP(ezoDEW, 66, pos, 3, 1);

  return pos + 1;

}

void PrintLoopMenu(){

  EscCls();
  EscInverse(1);
  byte pos = PrintQuickAir();
  EscInverse(0);
  pos++;

  EscLocate(3, pos++);
  
  //EscBold(1);
  //Serial.print(F("|   Temp.    |    EC     |     pH     |    Redox    |    O2     |   Level  |"));
  //pos = PrintLine(pos, 3, 76);
  //EscBold(0);

  pos = PrintTempToLevel(pos);

  PrintErrorOK(0, 0, (char*)"Read Loop started...");

  pos = PrintWaterVals(pos);

  pos = PrintLine(pos, 3, 76);
  
  // Avg 
  pos = PrintAVGs(pos);

  pos = PrintLine(pos, 3, 76);
  
  EscBold(1);
  
  // we need this pos in loop() / PrintPortStates()
  myLastLine = pos;
  portStateFirstRun = 0;
  
  PrintLine(pos + 1, 3, 76);

}

void PrintMainMenu(){

Start:

  int pos = PrintQuickAir();
  
  uint32_t hlpTime = 0;

  pos = PrintProbesOfType(255, 1, pos, 0);

  EscLocate(5, pos++);
  PrintMenuKeyStd('a'); Serial.print(F("ReScan   "));
  PrintMenuKeyStd('b'); Serial.print(F("ReBoot   "));
  PrintMenuKeyStd('c'); Serial.print(F("Date   "));
  PrintMenuKeyStd('d'); Serial.print(F("Time   "));
  PrintMenuKeyStd('e'); Serial.print(F("Addr. = "));
  PrintBoldInt(my.Address, 3, '0');

  PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  PrintMenuKeyStd('f'); Serial.print(F("Speed = "));
  EscBold(1);
  Serial.print(my.Speed);
  EscBold(0);
  PrintSpaces(3);  
  PrintMenuKeyStd('g'); Serial.print(F("Dim"));
  EscFaint(1);
  Serial.print(F("Color   "));
  EscFaint(0);
  PrintMenuKeyStd('h'); Serial.print(F("KeyColor   "));
  PrintMenuKeyStdBoldFaint('i', (my.Default), (!my.Default)); Serial.print(F("AsDefault"));

  pos = PrintShortLine(pos++, 8);

  EscLocate(5, pos++);
  PrintMenuKeyStd('j');
  Serial.print(F("More...   "));
  PrintMenuKeyStd('k'); Serial.print(F("Values...   "));
  PrintMenuKeyStd('l'); Serial.print(F("Times & Steps...   "));
  PrintMenuKeyStd('m'); Serial.print(F("Manual..."));

  PrintShortLine(pos++, 8);

  EscLocate(5, pos);
  PrintMenuKeyStd('n'); Serial.print(F("Setting-Name = "));
  EscBoldColor(fgBlue);
  Serial.print((char*)setting.Name);
  PrintSpaces(3);

  PrintMenuKeyLong((char*)"o-q):");

  Serial.print(F(" Sel.Setting [1-3] = "));
  EscBoldColor(fgBlue);
  Serial.print(my.Setting + 1);
  EscBoldColor(0);

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('q', ezoCnt);
  switch (pos){
  case -1:
    // TimeOut
  case 0:
    // EXIT
    break;
  case 'j':
    // All/More Menu
    PrintAllMenu();
    break;
  case 'a':
    // ReScan
    EscCls();
    EscLocate(1,1);
    EzoScan();
    break;
  case 'b':
    // ReBoot
    softReset();
    break;
  case 'c':
    // Date
    hlpTime = SerializeTime(1, 1, 2023, myHour, myMin, mySec);    // Time of now
    hlpTime += GetUserDate(myTime);
  case 'd':
    // Time
    if (pos == 'c'){
      hlpTime = SerializeTime(myDay, myMonth, myYear, 0, 0, 0);    // Midnight of today
      hlpTime += GetUserTime(myTime);
    }
    DeSerializeTime(hlpTime, &myDay, &myMonth, &myYear, &myHour, &myMin, &mySec);    
    //RTC_SetDateTime();
    myTime = SerializeTime(myDay, myMonth, myYear, myHour, myMin, mySec);
    break;
  case 'e':
    // Slave Address
    my.Address = GetUserInt(my.Address);
    if (my.Address > 254){
      // illegal address - reload from eeprom
      myFromRom();
    }
    else{
      // save to eeprom...
      myToRom();
    }
    break;
  case 'f':
    // Speed
    // Set Speed
    my.Speed = GetUserInt(my.Speed);
    if (IsSerialSpeedValid(my.Speed)){ 
      // valid - save to eeprom
      myToRom();
    }
    else{
      // illegal - reload from eeprom
      myFromRom();
    }
    break;
  case 'i':
    // Use and save actual Scan as Default
    my.Default = !my.Default;
    myToRom();
    if (my.Default){
      DefaultProbesToRom();
    }  
    break;
  case 'k':
    // Values
    PrintValuesMenu();
    break;
  case 'l':
    // Values
    PrintTimingsMenu();
    break;
  case 'm':
    PrintManualMenu();
    break;
  case 'n':
    // Setting Name
    GetUserString(setting.Name);
    SettingsToRom(my.Setting);
    break;
  case 'o':
  case 'p':
  case 'q':
    // Select Setting
    my.Setting = pos - 'o';
    SettingsFromRom(my.Setting);
    myToRom();
    break;
  case 'g':
    // Solarized
    my.Solarized = EscGetNextColor(my.Solarized);
    fgFaint = my.Solarized;
    myToRom();
    break;
  case 'h':
    // KeyColor
    my.KeyColor = EscGetNextColor(my.KeyColor);
    myToRom();
    break;    
  default:
    // Single Probe/Type
    PrintProbeMenu(pos - 49);
    break;
  }
  if (pos > 0){
    goto Start;
  }
  PrintLoopMenu();
}

#endif