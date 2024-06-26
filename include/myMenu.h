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
  return PrintMenuTop((char*)"- QuickAir 1.04 -");
}

byte PrintAllMenuOpt1(byte pos){

  EscLocate(5, pos++);
  PrintMenuKeyStd('a'); Serial.print(F("Reset   "));
  PrintMenuKeyStd('b'); Serial.print(F("Clear Cal(s)   "));
  PrintMenuKeyStd('c'); Serial.print(F("Del Name(s)   "));
  PrintMenuKeyStd('d'); Serial.print(F("Do Name(s)"));
  EscLocate(5, pos++);
  PrintMenuKeyStd('e'); Serial.print(F("Name = "));
  EscBold(1);
  Serial.print((char*)strDefault);
  PrintSpaces(3);
  PrintMenuKeyStd('f'); Serial.print(F("Addr. = "));
  EscBold(1);
  IntToIntStr(adrDefault, 3, '0');
  Serial.print((char*)strHLP);
  PrintSpaces(3);
  PrintMenuKeyStd('g'); Serial.print(F("Do Addr.(es)"));
    
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
  case 'e':
    // Edit Auto Name
    EditAutoName();
    break;
  case 'd':
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
  EscLocate(5, pos++);
  PrintMenuKeyStd('h');
  Serial.print(F("Boot Modes = "));
  EscBold(1);
  Serial.print((char*)Fa(bootMode[my.Boot]));
  EscBold(0);
  
  PrintShortLine(pos++, 8);
  EscLocate(19, pos++); Serial.print(F("Stepper    | FirstPort |  LastPort |  StepLogic |"));
  PrintLine(pos++, 15, 53);
  for (byte i = 0; i < 3; i++){
    EscLocate(15, pos++);
    EscBold(1);
    PrintCentered(Fa(ezoStrManType[i + 6]), 14);
    PrintSpacer(1);    /* code */
    PrintMenuKeyStdBoldFaint('i' + i, 0, !stepperDefinition.PortFirst[i] == 0);
    PrintInt(stepperDefinition.PortFirst[i], 5, ' ');
    PrintSpacer(1);
    PrintMenuKeyStdBoldFaint('l' + i, 0, !stepperDefinition.PortLast[i] == 0);
    PrintInt(stepperDefinition.PortLast[i], 5, ' ');
    PrintSpacer(1);
    PrintMenuKeyStd('o' + i);
    if (stepperDefinition.StepLogic[i]){
      Serial.print(F("binary"));
    }
    else{
      Serial.print(F("analog"));
    }
    PrintSpacer(1);
    
  }
  PrintLine(pos, 15, 53);
  
  PrintMenuEnd(pos + 2);

  pos = GetUserKey('q', -1);

  if (!SwitchAllAndProbeMenu(pos, 0, 2)){
    switch (pos){
    case 'h':
      // Boot Mode
      my.Boot++;
      if (my.Boot > 3){
        my.Boot = 0;
      }
      myToRom();
      break;
    case 'i'...'k':
      // FirstPort
      stepperDefinition.PortFirst[pos - 'i'] = GetUserInt(stepperDefinition.PortFirst[pos - 'i']);
      StepperToRom();
      break;
    case 'l'...'n':
      // LastPort
      stepperDefinition.PortLast[pos - 'l'] = GetUserInt(stepperDefinition.PortLast[pos - 'l']);
      StepperToRom();
      break;
    case 'o'...'q':
      stepperDefinition.StepLogic[pos - 'o'] = !stepperDefinition.StepLogic[pos - 'o'];
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
  
  PrintMenuKeyLong((char*)"1-4):");
  
  Serial.print(F(" Copy FULL SETTING "));
  EscColor(fgBlue);
  Serial.print((char*)setting.Name);
  Serial.print(F(" ("));
  Serial.print(my.Setting + 1);
  Serial.print(F(")"));
  EscColor(0);
  Serial.print(F(" to Setting-No. [1-4]"));
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

  pos = GetUserKey('p', 4);

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
  else if (IsKeyBetween(pos, '1', '4')){
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
    byte val = stepper[i - 6].Value;
    if (val != lastVal[i]){
      lastVal[i] = val;
      isChanged = 1;
    }
  }
  
  if (isChanged || !portStateFirstRun){
    for (byte i = 0; i < 4; i++){
      // Low-Ports
      byte state = (myTime < manTempTime[i]) * 2;
      if (manual[i].State){
        state = 1;
      }
      
      if (!my.Boot){
        EscLocate(posOfPort[i], myLastLine);
      }
      
      if (lastVal[i]){
        if (!my.Boot){
          // Terminal
          EscBold(1);
          SetAvgColorEZO(i);
          Serial.print(F(">"));
          EscFaint(1);
        }
        else if (my.Boot < 3){
          // ModBus RTU & AscII
        }
        else{
          // just values
          MBaction(my.Address, i, state, 1);
        }
      }
      else{
        if (!my.Boot){
          // Terminal
          EscFaint(1);
          Serial.print(F(">"));
          EscBold(1);
          SetAvgColorEZO(i);
        }
        else if (my.Boot < 3){
          // ModBus RTU & AscII
        }
        else{
          // just values
          MBaction(my.Address, i, state, 0);
        }
      }
      if (!my.Boot){
        Serial.print(F("~"));
      }
      
    }
    for (byte i = 4; i < 6; i++){
      // High-Ports

      byte state = (myTime < manTempTime[i]) * 2;
      if (manual[i].State){
        state = 1;
      }

      if (!my.Boot){
        EscLocate(posOfPort[i], myLastLine);
      }
      
      if (lastVal[i]){
        if (!my.Boot){
          // Terminal
          EscBold(1);
          SetAvgColorEZO(i - 4);
          Serial.print(F("<"));
          EscCursorLeft(2);
          EscFaint(1);
          Serial.print(F("~"));
        }
        else if (my.Boot < 3){
          // ModBus RTU & AscII
        }
        else{
          // just values
          MBaction(my.Address, i, state, 1);
        }
      }
      else{
        if (!my.Boot){
          // Terminal
          EscFaint(1);
          Serial.print(F("<"));        
        }
        else if (my.Boot < 3){
          // ModBus RTU & AscII
        }
        else{
          // just values
          MBaction(my.Address, i, state, 0);
        }
      }
    }

    // Loop the Analog Ports
    
    for (byte i = 6; i < 9; i++){
      // Analog-Ports
      byte state = (myTime < manTempTime[i]) * 2;
      if (manual[i].State){
        state = 1;
      }
      if (!my.Boot){
        // Terminal
        EscLocate(posOfVal[i - 6], myLastLine);
        EscBold(1);
        SetAvgColorEZO(ezoRTD);
        Serial.print(lastVal[i]);
        EscColor(0);
        EscBold(0);
        Serial.print(F("~"));
        EscCursorLeft(3);
        Serial.print(F("~"));
        EscBoldColor(0);
      }
      else if (my.Boot < 3){
        // ModBus RTU & AscII
      }
      else{
        // just values
        MBaction(my.Address, i, state, lastVal[i]);
      }
    }
    
    portStateFirstRun = 1;
    
  }
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

byte PrintManualMenu(){

    //                   |    State   |  permValue  | tempUntil |  tempValue  |
    //-------------------------------------------------------------------------
    //        RTD >>     | a)         | j)          | s) 00:00:00 | B)        |
    //        HUM >>     | b)         | k)          | t)          | C)        |
    //        CO2 >>     | c)         | l)          | u)          | D)        |
    //        DEW >>     | d)         | m)          | v)          | E)        |
    //-------------------------------------------------------------------------
    //        << RTD     | e)         | n)          | w)          | F)        |
    //        << HUM     | f)         | o)          | x)          | G)        |
    //-------------------------------------------------------------------------
    //       Exhaust     | g)         | p)          | y)          | H)        |
    //       Intake      | h)         | q)          | z)          | I)        |
    //    Circulation    | i)         | r)          | A)          | J)        |
    //-------------------------------------------------------------------------

  uint32_t tempTime = 900;

Start:

  int8_t pos = PrintMenuTop((char*)"- Manual Menu -") + 1;
  byte i = 0;
  
  EscLocate(20, pos++);
  EscBold(1);
  Serial.print(F("|     State     | permValue |   tempUntil  | tempValue |"));
  PrintLine(pos++, 5, 71);

  for (i = 0; i < 9; i++){

    if (!(manTempTime[i] > myTime)){
      // no temporary something (anymore)
      manTempTime[i] = 0;
    }

    EscLocate(5, pos++);

    EscBold(1);
    PrintCentered(Fa(ezoStrManType[i]), 14);
    PrintSpacer(1);
    
    PrintMenuKeyStd('a' + i);
    if (!(manTempTime[i])){
      if (manual[i].State > 1){
        manual[i].State = 0;
        ManualToRom();
      }
      
      Serial.print(Fa(ezoStrManState[manual[i].State]));
    }
    else{
      Serial.print(F("temp. ("));
      if (manual[i].State){
        Serial.print(F("p)"));
      }
      else{
        Serial.print(F("a)"));
      }
      
      
    }
    PrintSpacer(1);

    PrintMenuKeyStd('j' + i);
    PrintInt(manual[i].PermVal, 5, ' ');
    PrintSpacer(manTempTime[i] > 0);

    if (i < 8){
      PrintMenuKeyStd('s' + i);
    }
    else{
      PrintMenuKeyStd('A');
    }
    
    
    EscFaint(manTempTime[i] == 0);
    PrintSerTime(manTempTime[i], 0, 1);
    PrintSpacer(1);

    PrintMenuKeyStd('B' + i);
    PrintInt(manual[i].TempVal, 5, ' ');
    PrintSpacer(0);


    switch (i){
    case 3:
    case 5:
    case 8:
      PrintLine(pos++, 5, 71);
      break;
    default:
      break;
    }
  }

  pos++;
  EscLocate(5, pos++);
  
  PrintMenuKeyLong((char*)"a-g):");
  Serial.print(F(" Switch State   "));
  
  PrintMenuKeyLong((char*)"j-r):");
  Serial.print(F(" Permanent Value   "));
  
  PrintMenuKeyLong((char*)"s-z & A):");
  Serial.print(F(" Apply TempTime"));

  PrintShortLine(pos++, 8);

  EscLocate(5, pos++);

  PrintMenuKeyLong((char*)"B-J):");
  Serial.print(F(" Temporary Value   "));

  PrintMenuKeyStd('1');
  Serial.print(F("Set TempTime = "));
  EscBold(1);
  PrintSerTime(tempTime, 0, 1);
  PrintSpaces(3);

  PrintMenuKeyStd('2');
  Serial.print(F("Run Loop..."));

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('z', 2);

  if (pos < 1){
    // Exit & TimeOut
    return 0;
  }
  else if (IsKeyBetween(pos, 'a', 'i')){
    // State
    i = pos - 'a';
    manual[i].State = !manual[i].State;
    pos = 1;
  }
  else if (IsKeyBetween(pos, 'j', 'r')){
    // permanent Value
    i = pos - 'j';
    manual[i].PermVal = GetUserInt(manual[i].PermVal);
    pos = 1;
  }
  else if (IsKeyBetween(pos, 'B', 'J')){
    // temporary Value
    i = pos - 'B';
    manual[i].TempVal = GetUserInt(manual[i].TempVal);
    pos = 1;
  }
  else if (IsKeyBetween(pos, 's', 'z') || pos == 'A'){
    // Add Temporary Time
    i = pos;
    if (i == 'A'){
      i = 123;
    }
    i = i - 's';
    manTempTime[i] = myTime + tempTime;
    pos = 2;
  }
  else if (pos == '1'){
    // Set TempTime
    tempTime = GetUserTime(tempTime);
  }
  else if (pos == '2'){
    // Run Loop
    return 1;
  }
  
  if (pos == 1){
    // Save
    ManualToRom();
  }
  
  if (pos > 0){
    goto Start;
  }
  
  return 0;
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

//          |   UpDelay  |  DownDelay |  StartStep |   MinStep  |   MaxStep  |
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
    PrintTimingsMenuTime(i + 'a', setting.DelayTime[i], 0, 1);
    PrintSmallSpacer();
    PrintTimingsMenuTime(i + 'h', setting.TimeTooLow[i], 1, 1);
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
      Serial.print(F("  UpDelay  |  DownDelay |  StartStep |   MinStep  |   MaxStep"));
      PrintFlexSpacer(2, 0);
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

  pos = GetUserKey('z', 4);

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
    pos = GetUserTime16ptr(&setting.DelayTime[pos - 'a'], 1);
  }
  else if (IsKeyBetween(pos, 'h', 'n')){
    // tooLow
    pos = GetUserTime16ptr(&setting.TimeTooLow[pos - 'h'], 1);
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
  else if (IsKeyBetween(pos, '1', '4')){
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
        if (!my.Boot){
          // Terminal
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
        }
        else if (my.Boot < 3){
          // ModBus RTU & AscII
        }
        else{
          // just values
          MBanalog(my.Address, i, j, ezoValue[i][j]);
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
    // avg[0] = avgVal[ezotype];
    if (ezotype == ezoHUM){
      // Humidity has three Values...
      avg_TMP = avg[1] / (long)posAct;
      // avg[1] = avg_TMP;
      avg_DEW = avg[2] / (long)posAct;
      // avg[2] = avg_DEW;
    }
    if (!my.Boot){
      // Terminal
    }
    else if (my.Boot < 3){
      // ModBus RTU & AscII
    }
    else{
      // just values
      /*
      for (byte i = 0; i < 3; i++){
        if (!i || ezotype == ezoHUM){
          MBstart(my.Address);
          iicStr[2] = 2;          // 0 = QuickTimer, 1 = QuickWater, 2 = QuickAir
          iicStr[3] = 3;          // Value AVG
          iicStr[4] = ezotype;    // type of probe
          iicStr[5] = i;          // value ID (e.g. HUM has multiple values on one probe...)
          // Value of probe
          MBaddLong(avg[i], 6);
          MBstop(10);
        } 
      }
      */
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

  byte pos = 0;

  if (!my.Boot){
    EscCls();
    EscInverse(1);
    pos = PrintQuickAir();
    EscInverse(0);
    pos++;

    EscLocate(3, pos++);
    
    //EscBold(1);
    //Serial.print(F("|   Temp.    |    EC     |     pH     |    Redox    |    O2     |   Level  |"));
    //pos = PrintLine(pos, 3, 76);
    //EscBold(0);

    pos = PrintTempToLevel(pos);

    PrintErrorOK(0, 0, (char*)"Read Loop started...", 0);
  }
  

  pos = PrintWaterVals(pos);

  if (!my.Boot){
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

  PrintMenuKeyLong((char*)"o-r):");

  Serial.print(F(" Sel.Setting [1-4] = "));
  EscBoldColor(fgBlue);
  Serial.print(my.Setting + 1);
  EscBoldColor(0);

  PrintMenuEnd(pos + 1);

  pos = GetUserKey('r', ezoCnt);
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
    if (PrintManualMenu()){
      pos = 0;
    }
    break;
  case 'n':
    // Setting Name
    GetUserString(setting.Name);
    SettingsToRom(my.Setting);
    break;
  case 'o':
  case 'p':
  case 'q':
  case 'r':
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