#include "quicklib.h"
#include <EEPROM.h>

#define EZO_MAX_PROBES 12
#define EZO_MAX_VALUES 3        // 3 for full HUM / 5 for full RGB...
#define EZO_1st_ADDRESS 32
#define EZO_LAST_ADDRESS 127

// EEprom
//  4x settings 151 Bytes = 602
// 12x ezoProbe  22 Bytes = 264     - 866 (133)
//  3x manual    41 Bytes = 123     - 989 (10)
// 12 Byte "MY"

// What is the actual Action
// 0 = all read
// 1 = write command(s)
// 2 = all written
// 3 = read answer(s)
static byte ezoAction = 0;

// Which is the actual ezo
static byte ezoAct = 0;

// How many ezo's we have
static byte ezoCnt = 0;

#define ezoRTD 0
#define ezoHUM 1    // Hardware has Temp & DEW, too !
#define ezoCO2 2
#define ezoTMP 3    // No real Hardware - Temp from HUM
#define ezoDEW 4    // No real Hardware - Dew from HUM

const char ezoStrType_0[] PROGMEM = "RTD";
const char ezoStrType_1[] PROGMEM = "HUM";
const char ezoStrType_2[] PROGMEM = "CO2";
const char ezoStrType_3[] PROGMEM = "DEW";
PGM_P const ezoStrType[] PROGMEM = {
    ezoStrType_0,
    ezoStrType_1,
    ezoStrType_2,
    ezoStrType_3
};


// RTD
const char ezoStrTimeType_1[] PROGMEM = "Exhaust";
const char ezoStrTimeType_2[] PROGMEM = "Intake";
const char ezoStrTimeType_3[] PROGMEM = "Circ.";
// Hum
// CO2
// DEW
PGM_P const ezoStrTimeType[] PROGMEM = {
    ezoStrType_0,
    ezoStrTimeType_1,
    ezoStrTimeType_2,
    ezoStrTimeType_3,
    ezoStrType_1,
    ezoStrType_2,
    ezoStrType_3,
};


const char ezoStrUnit_0[] PROGMEM = "°C";
const char ezoStrUnit_1[] PROGMEM = "rH%";
const char ezoStrUnit_2[] PROGMEM = "ppm";
// °C
// °C
PGM_P const ezoStrUnit[] PROGMEM = {
    ezoStrUnit_0,
    ezoStrUnit_1,
    ezoStrUnit_2,
    ezoStrUnit_0,
    ezoStrUnit_0,
};

// Waittime for readings...
const int ezoWait[] PROGMEM = {600, 900, 900}; // !! check the 900s !!

// Count of vals of probe
const byte ezoValCnt[] PROGMEM = {1, 3, 2};

// if type has a calibration
const byte ezoHasCal[] PROGMEM = {1, 0, 1};

typedef struct ezoProbeSTRUCT{
    // 22 Byte * ??? Probes Max = ??? Byte
    byte type;
    byte calibrated;
    //byte error;            // 0=OK, 1=processing, 2=syntax, 3=IIC, unknown
    byte address;
    uint16_t version; 
    char name[17];
    //long valueLast[EZO_MAX_VALUES];
}ezoProbeSTRUCT;
ezoProbeSTRUCT ezoProbe[EZO_MAX_PROBES];

int32_t ezoValue[EZO_MAX_PROBES][EZO_MAX_VALUES];

struct settingSTRUCT{
    // 151 Byte x 2 (day & night) 301 Byte
    uint16_t DelayTime[7];      // 14      // Temp / Exhaust / Intake / Circulation / Humidity / CO2 / Dew (heat on Wet)
    uint16_t TimeTooLow[7];     // 14
    uint16_t TimeLow[7];        // 14
    uint16_t TimeHigh[5];       // 10
    uint16_t TimeTooHigh[5];    // 10
    int32_t FailSaveValue[4];   // 16   // Temp / Hum / CO2 / DEW / 
    int32_t ValueTooLow[4];     // 16   // Temp / Hum / CO2 / Dew
    int32_t ValueLow[4];        // 16
    int32_t ValueHigh[2];       // 08
    int32_t ValueTooHigh[2];    // 08
    char Name[17];              // 17
    //                            143
}setting;

struct manualSTRUCT{
    // 41 Bytes x doesn't matter
    // m) SomeBlaBlaName     |     Low    |     High   |   Value    |
    //---------------------------------------------------------------
    //          RTD          | a)[0+2]    | h)[4]      |
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //         Exhaust       | b)         |            | i)         |
    //         Intake        | c)         |            | j)         |
    //         Circ.         | d)         |            | k)         |
    //---------------------------------------------------------------
    //          HUM          | e)[1]      | l)[5]      |
    //--------------------------------------------------
    //          CO2          | f)[2]      | 
    //-------------------------------------
    //          DEW          | g)[3]      |
    uint16_t LowPort[7];    // 14            // Heat / Exh. / Intake / Circ. / Hum / Add(CO2) / Heat(DEW)
    uint16_t HighPort[5];   // 10            // Cool / Val  /   Val  /  Val  / Dry 
    char Name[17];          // 17
    //                      // 41
}manual;

// Counter for Low/High
uint32_t tooLowSince[7];
uint32_t lowSince[7];
uint32_t okSince[7];
uint32_t highSince[2];
uint32_t tooHighSince[2];
// Time of last action 
uint32_t lastAction[7];

long avgVal[5]; //  = {21000L, 1250000L, 6000L, 225000L, 99999L, 66666L};
#define avg_RTD avgVal[0]
#define avg_HUM avgVal[1]
#define avg_CO2 avgVal[2]
#define avg_TMP avgVal[3]
#define avg_DEW avgVal[4]


#define CAL_RTD_RES -1         // Value for Reset
#define CAL_RTD_LOW 0          // Value for LowPoint
#define CAL_RTD_MID 21000      // Value for MidPoint
#define CAL_RTD_HIGH 100000    // Value for HighPoint

#define CAL_CO2_RES -1         // Value for Reset
#define CAL_CO2_LOW 0          // Value for LowPoint
#define CAL_CO2_MID 3900000    // Value for MidPoint
#define CAL_CO2_HIGH 0         // Value for HighPoint


void DefaultProbesToRom(){
    // Save actual probe-constellation as Standard to Eeprom
    // 22 byte * 12 probes max = 264 (id 263)
    EEPROM.put(0, ezoProbe);
}
void SettingsToRom(int set){ //(int set){
    // 143 byte * 3 sets = 429 + 264 = 693(next)
    EEPROM.put(264 + (set * 143), setting);
}
void ManualTimesToRom(int set){
    // 41 byte + 3 sets = 123 + 693 = 816(next)
    EEPROM.put(693 + set * 41, manual);
}

void DefaultProbesFromRom(){
    EEPROM.get(0, ezoProbe);
}
void SettingsFromRom(int set){ //(int set){
   EEPROM.get(264 + set * (143), setting);
}
void ManualTimesFromRom(int set){
    //set *= 41;
    EEPROM.get(693 + set * 41, manual);
}

void OffOutPorts(){
    // Missing that the ventilation-ports 'OFF' is the value of 'OK'
    for (byte i = 2; i < 18; i++){
        digitalWrite(i, LOW);
    }
}

byte GetAvgState(long avg, long tooLow, long low, long high, long tooHigh){
  if (avg < tooLow){
    return fgCyan;
  }
  else if (avg < low){
    return fgBlue;
  }
  else if (avg > tooHigh){
    return fgRed;
  }
  else if (avg > high){
    return fgYellow;
  }
  return fgGreen;
}
void SetAvgColor(long avg, long tooLow, long low, long high, long tooHigh){
  EscColor(GetAvgState(avg, tooLow, low, high, tooHigh));
}

// #define SetAvgColorEZO(avgVal, ezoType) SetAvgColor(avgVal, tooLow[ezoType], low[ezoType], high[ezoType], tooHigh[ezoType])
void SetAvgColorEZO(byte ezoType){
    // - 46 Flash (5x used)
    // +128 Ram
    SetAvgColor(avgVal[ezoType], setting.ValueTooLow[ezoType], setting.ValueLow[ezoType], setting.ValueHigh[ezoType], setting.ValueTooHigh[ezoType]);
}

char EzoStartValues(byte ezo){
    return IIcSetStr(ezoProbe[ezo].address, (char*)"R", 0);
}

void EzoWaitValues(byte ezo){
    delay(Fi(ezoWait[ezoProbe[ezo].type]));
}

byte EzoGetValues(byte ezo){
    if (IIcGetAtlas(ezoProbe[ezo].address) > 0){
        ezoValue[ezo][0] = StrTokFloatToInt(iicStr);
        for (byte i = 1; i < Fb(ezoValCnt[ezoProbe[ezo].type]); i++){
            ezoValue[ezo][i] = StrTokFloatToInt(NULL);
        }
        return 1;        
    }
    else{
        return 0;
    }
}

byte EzoCheckOnSet(byte ezo, byte all, byte i){
    // Check, if Module on i is valid in a EzoSetXYZ() function...
    if (i == ezo || (all == 1 && ezoProbe[i].type == ezoProbe[ezo].type) || (all == 2)){
        return 1;
    }
    return 0;
}

void EzoSetName(char *strIN, byte ezo, byte all, byte autoName){
    
    byte cnt[] = {0, 0, 0, 0, 0, 0};//, 0, 0, 0, 0, 0};

    byte len = 0;


    for (int i = 0; i < ezoCnt; i++){

        if (EzoCheckOnSet(ezo,all, i)){

            strcpy_P(iicStr, (PGM_P)F("Name,"));

            if (autoName){
                cnt[ezoProbe[i].type]++;
                // Type
                strcpy(&iicStr[5], Fa(ezoStrType[ezoProbe[i].type]));
            }
            
            strcpy(&iicStr[strlen(iicStr)], strIN);
            
            if (autoName){
                len = strlen(iicStr);
                iicStr[len] = cnt[ezoProbe[i].type] + 48;
                iicStr[len + 1] = 0;
            }
            
            if (IIcSetStr(ezoProbe[i].address, iicStr, 1) > 0){
                strcpy(ezoProbe[i].name, &iicStr[5]);
            }               
        }
        
        // No clue why - but a second 0 is needed... otherwise module go kind of crazy - sometimes...
        Wire.beginTransmission(ezoProbe[i].address);
        Wire.write(0);
        Wire.endTransmission(ezoProbe[i].address);
    }
}

void EzoReset(byte ezo, byte all){
    // 'Factory' Reset
    // All = 1 = all of type ezo
    // All = 2 = all types

    byte cntSetup;           // count of setup-lines to send
    char strSetup[6][9];

    for (int i = 0; i < ezoCnt; i++){

        if (EzoCheckOnSet(ezo,all, i)){
        
            strcpy_P(strSetup[0],(PGM_P)F("L,1"));  // Indicator LED
            cntSetup = 1;

            switch (ezoProbe[i].type){
            case ezoRTD:
                strcpy_P(strSetup[1],(PGM_P)F("S,c"));  // Celsius (k = kelvin / f = fahrenheit)
                cntSetup = 2;
                break;
            case ezoHUM:
                strcpy_P(strSetup[1],(PGM_P)F("O,HUM,1"));
                strcpy_P(strSetup[2],(PGM_P)F("O,T,1"));
                strcpy_P(strSetup[3],(PGM_P)F("O,Dew,1"));
                cntSetup = 4;
                break;
            case ezoCO2:
                strcpy_P(strSetup[1],(PGM_P)F("O,t,0"));
                cntSetup = 2;
                break;
            default:
                break;
            }
            // Write Setup
            for (int i2 = 0; i2 < cntSetup; i2++){
                IIcSetStr(ezoProbe[i].address, strSetup[i2], 0);
                delay(300);
            }
            delay(300);
        }
    }   
}

void EzoSetCalTemp(byte ezo, byte all){
    
    for (byte i = 0; i < ezoCnt; i++){
        if (EzoCheckOnSet(ezo, all, i)){
            IIcSetStr(ezoProbe[i].address, (char*)"T,25", 0);
        }
    }
    
}

void EzoSetCal(char *strCmd, byte ezo, byte all, int32_t value, byte calAction){
    
    // if !calAction
    //    "Cal"
    // else
    // calAction = 1    = ",low,"   + value
    //           = 2    = ",mid,"   + value
    //           = 3    = ",high,"  + value
    //           = 4    = ","   + value
    //           = 5    = ","   + strCmd
    //           = 6    = ",dry"
    strcpy_P(iicStr, (PGM_P)F("Cal"));
    if (calAction){
        iicStr[3] = ',';
        byte len = 4;
        switch (calAction){
        case 1:
            // "low,"
            strcpy_P(&iicStr[len], (PGM_P)F("low,"));
            len = 8;
            break;
        case 2:
            // "mid,"
            strcpy_P(&iicStr[len], (PGM_P)F("mid,"));
            len = 8;
            break;
        case 3:
            // "high,"
            strcpy_P(&iicStr[len], (PGM_P)F("high,"));
            len = 9;
            break;
        case 5:
            // strCmd
            strcpy(&iicStr[len], strCmd);
            break;
        case 6:
            // "dry"
            strcpy_P(&iicStr[len], (PGM_P)F("dry"));
            break;
        }
        if (calAction < 5){
            // value
            IntToFloatStr(value, 1, 2, '0');
            strcpy(&iicStr[len], strHLP);
        }
    }
    
    for (byte i = 0; i < ezoCnt; i++){
        if (EzoCheckOnSet(ezo,all, i)){
            if (Fb(ezoHasCal[ezoProbe[ezo].type])){
                // Has set-able calibration
                Serial.println();
                Serial.print(ezoProbe[i].address);
                Serial.print(":");
                Serial.println(iicStr);
                IIcSetStr(ezoProbe[i].address, iicStr, 0);
                ezoProbe[i].calibrated = 0;
            }
        }
    }
}

void EzoSetAddress(byte ezo, byte addrNew, byte all){
    for (int i = 0; i < ezoCnt; i++){
        if (addrNew > 79 && addrNew < 88){
            // Exclude Eprom-Addresses
            addrNew = 88;
        }
        if (addrNew == 104){
            // Exclude RTCs
            addrNew = 105;
        }
        if (EzoCheckOnSet(ezo,all, i)){
            strcpy_P(strHLP, (PGM_P)F("I2C,"));
            itoa(addrNew, strHLP2, 10);
            strcpy(&strHLP[4], strHLP2);
            IIcSetStr(ezoProbe[i].address, strHLP, 0);
            addrNew++;
            delay(300);
        }
    }
}

int32_t GetRtdAvgForCal(){

    long avgTemp = 0;
    byte avgCnt = 0;

    for (byte i = 0; i < ezoCnt; i++){
        if (ezoProbe[i].type == ezoRTD){
            // RTD
            avgCnt++;
            EzoStartValues(i);
            EzoWaitValues(i);
            EzoGetValues(i);
            avgTemp += ezoValue[i][0];
        }
    }
    if (avgCnt){
        // Real RTD exist
        return avgTemp / avgCnt;
    }
    else{
        return avg_RTD;
    }
}

int32_t PrintValsForCal(byte ezo, byte all, int32_t refValue){
    
    long avgTemp = 25;
    byte pos = 1;
    byte startPos = 1;
    byte timeOut = 120;

    START:

    startPos = 0;
    pos = 0;
    //if (ezoProbe[ezo].type == ezoPH || ezoProbe[ezo].type == ezoEC){
        // Need on temperature to do cal right
        avgTemp = GetRtdAvgForCal();
        avg_RTD = avgTemp;
        Serial.println();

        EscKeyStyle(1);
        Serial.print(F("["));
        PrintFloat(refValue, 4, 2, ' ');
        Serial.print(F("] @:"));
        EscKeyStyle(0);
        Print1Space();

        PrintFloat(avgTemp, 3, 2, ' ');
        EscFaint(1);
        Serial.print(F("°C"));
        EscFaint(0);
        EscCursorRight(3);
        pos += 20;
        startPos = 20;
    //}   // else no need on temp

    // Read Vals of all to calibrate probes
    for (byte i = 0; i < ezoCnt; i++){
        if (EzoCheckOnSet(ezo, all, i)){
            EzoStartValues(i);
            EzoWaitValues(i);
            EzoGetValues(i);
            PrintInt(i + 1, 2, '0');
            Serial.print(F(": "));
            PrintBoldFloat(ezoValue[i][0], 4, 2, ' ');
            PrintSpacer(0);
            pos += 14;
            if (pos > 66){
                Serial.println();
                EscCursorRight(startPos);
                pos = startPos + 1;
            }
        }
        if (DoTimer()){
            timeOut--;
        }
        if (Serial.available() || !timeOut){
            // Accept
            if (Serial.read() != 27){
                // valid return
                return avgTemp;
            }
            return 0;
        }
    }
    goto START;
    return 0;
}

int8_t EzoDoNext(){

    int8_t err = 1;
    char errInfo[] = "'?'";
    byte errCnt = 0;

    switch (ezoAction){
    case 0:
      // Set Avg-RTD to EC & pH probes
      // This case doesn't exist in QuickAir...
      break;
    
    case 1:
      // Ask for Data
      errCnt = 0;
      err = -1;
      while (err < 0){
        err = EzoStartValues(ezoAct);
        if (err < 0){
          errCnt++;
          if (errCnt > 3){
            // Fatal for this Probe
            errInfo[1] = 'R';
            errCnt = ezoAct;
            break;
          }
          delay(333);
        }
        else{
          // errCnt = ezoAct;
        }          
      }
    
      break;
    
    case 2:
      // Get Data
      err = EzoGetValues(ezoAct);

      if (err == 0){
        // Immediately Fatal for this Probe
        errInfo[1] = 'D';
        errCnt = ezoAct;
        err = - 1;
      }
      else{
        // errCnt = ezoAct;
      }          
      break;

    default:
      ezoAction = 0;
      ezoAct = 0;
      return 0;
      break;
    }

    if (ezoAction == 2){
        // Actions done for this Module
        ezoAct++;
        ezoAction = 0;
        if (ezoAct == ezoCnt){
            // All Modules Done
            ezoAct = 0;
            return 1;
        }
    }
    else{
      // Next Module  
      ezoAction++;
    }

    if (err < 0){
        PrintErrorOK(-1, strlen(errInfo), errInfo);
        return -1;
    }
    else{
        // PrintErrorOK(1, errCnt, errInfo);
        return 0;
    }

}

void PrintColon(){
    PrintCharInSpaces(':');
}
void EzoScan(){
    // Scan for Ezo's

    int8_t err;
    int8_t recEzo;         // recognized EzoProbe[ezoCnt].module.version
    int8_t hasCal;         // has a calibration
    byte lastAddr = 0;     // last read address (to check on 1-time re-read)
    ezoCnt = 0;
    Serial.println();

    for (byte i = EZO_1st_ADDRESS; i < EZO_LAST_ADDRESS + 1 && ezoCnt < EZO_MAX_PROBES; i++){
        
        // Exclude known stuff (eeprom & rtc)
        if (!(i > 79 && i < 88) && !(i == 104)){        

            if (lastAddr == i){
                // we're on a re-read
                // Slave exist, but we failed on "i" 
                lastAddr = 0;
                delay(333);
            }
            else{
                lastAddr = i;
            }
            recEzo = -1;

            if (lastAddr == i){
                Wire.beginTransmission(i);
                err = Wire.endTransmission();
            }
            else{
                // we're on a 2nd read, cause we failed on 1st try with "I"
                err = 0;
            }
            //Wire.beginTransmission(i);
            //err = Wire.endTransmission();

            if (!err){
                Serial.print(F("Slave @: "));
                Serial.print(i);
                PrintColon();
                delay(333);
                // Slave found... looking for EZO-ID
                err = IIcSetStr(i, (char*)"i", 0);
                Serial.print(err);
                PrintColon();
                delay(333);
                err = IIcGetAtlas(i);
                Serial.print(err);
                PrintColon();
                switch (err){
                case 0:
                    // nothing received
                    break;
                case -1:
                case -2:
                case -4:
                    // ezo errors
                    break;
                case -3:
                    // IIC error
                    break;
                default:
                    // something received
                    Serial.print(iicStr);
                    PrintColon();
                    if (iicStr[0] == '?' && iicStr[1] == 'I'){
                        // It's an ezo...

                        //recEzo = -1;
                        //verPos = 7;
                        hasCal = 1;
                        ezoProbe[ezoCnt].calibrated = 0;
                        ezoProbe[ezoCnt].name[0] = 0;

                        switch (iicStr[3]){
                        case 'R':
                            // RGB or RTD
                           if (iicStr[4] == 'T'){
                            recEzo = ezoRTD;
                           }                           
                           break;
                        case 'C':
                            // CO2
                            recEzo = ezoCO2;
                            hasCal = 0;
                            break;
                        case 'H':
                            // HUM + Temp + Dew
                            recEzo = ezoHUM;
                            break;    
                        default:
                            // LATE ERROR or not supported...
                            break;
                        }
                        Serial.println(recEzo);
                        if (recEzo > -1){
                            // Valid Probe found
                            
                            // Save address and type
                            ezoProbe[ezoCnt].address = i;
                            ezoProbe[ezoCnt].type = recEzo;
                            
                            // Extract Version
                            ezoProbe[ezoCnt].version = StrTokFloatToInt(iicStr);
                            // Calibration
                            if (hasCal){
                                IIcSetStr(i,(char*)"Cal,?", 0);
                                delay(333);
                                if (IIcGetAtlas(i) > 0){
                                    ezoProbe[ezoCnt].calibrated = iicStr[5] - 48;
                                }                            
                            }
                            
                            // Name
                            IIcSetStr(i,(char*)"Name,?", 0);
                            delay(333);
                            if (IIcGetAtlas(i) > 0){
                                strcpy(ezoProbe[ezoCnt].name, &iicStr[6]);
                            }
// ********************
                            // Output (only stored in Module)
                            // Status
                            IIcSetStr(i, (char*)"Status", 0);
                            Serial.print(F("Status: "));
                            delay(300);
                            if (IIcGetAtlas(i) > 0){
                                Serial.print(iicStr[8]);
                                PrintCharInSpaces('@');
                                Serial.print(&iicStr[10]);
                                Serial.println(F(" V"));
                            }
                            else{
                                Serial.println(F("ERR"));
                            }
                            
                            // Value(s)
                            Serial.print(F("Val: "));
                            EzoStartValues(ezoCnt);
                            EzoWaitValues(ezoCnt);
                            if (EzoGetValues(ezoCnt)){
                                for (byte i2 = 1; i2 < Fb(ezoValCnt[recEzo]); i2++){
                                    PrintCharInSpaces(',');
                                    Serial.print(ezoValue[ezoCnt][i2]);
                                }          
                                Serial.println(F(""));
                            }
                            else{
                                Serial.println(F("ERR"));
                            }
                            Serial.println();
                            
                            // done
                            ezoCnt++;
                            my.Cnt = ezoCnt;
                        }
                    } 
                    break;
                }
                if (recEzo < 0 && lastAddr == i){
                    // Slave exist, but "I" wasn't working 
                    // and it's the 1st call - so we do ONE re-read
                    i--;
                }
            }
        }
    }

}

/*
void EzoScan(){
    // Scan for Ezo's

    int err;
    int recEzo;         // recognized EzoProbe[ezoCnt].module.version
    int verPos;         // 'pointer' on 1st char of version
    int hasCal;         // has a calibration
    
    ezoCnt = 0;
    Serial.println("");

    for (int i = EZO_1st_ADDRESS; i < EZO_LAST_ADDRESS + 1 && ezoCnt < EZO_MAX_PROBES; i++){
        
        //Exclude known stuff
        if (!(i > 79 && i < 88) && !(i == 104)){        

            Wire.beginTransmission(i);
            err = Wire.endTransmission();
            if (!err){
                Serial.print(F("Slave @: "));
                Serial.print(i);
                Serial.print(F(" : "));
                // Slave found... looking for EZO-ID
                err = IIcSetStr(i, (char*)"i", 0);
                Serial.print(err);
                Serial.print(F(" : "));
                delay(333);
                err = IIcGetAtlas(i);
                Serial.print(err);
                Serial.print(F(" : "));
                switch (err){
                case 0:
                    // nothing received
                    break;
                case -1:
                case -2:
                case -4:
                    // ezo errors
                    break;
                case -3:
                    // IIC error
                    break;
                default:
                    // something received
                    Serial.print(iicStr);
                    Serial.print(F(" : "));
                    if (iicStr[0] == '?' && iicStr[1] == 'I'){
                        // It's an ezo...

                        recEzo = 0;
                        verPos = 7;
                        hasCal = 1;
                        ezoProbe[ezoCnt].calibrated = 0;
                        ezoProbe[ezoCnt].name[0] = 0;

                        switch (iicStr[3]){
                        case 'R':
                            // RGB or RTD
                            switch (iicStr[4]){
                            case 'G':
                                // RGB
                                recEzo = ezoRGB;
                                hasCal = 0;
                                break;
                            case 'T':
                                // RTD
                                recEzo = ezoRTD;
                                break;
                            default:
                                // LATE ERROR
                                break;
                            }
                            break;
                        case 'F':
                            // FLO(W)
                            recEzo = ezoFLOW;
                            hasCal = 0;
                            break;
                        case 'O':
                            // ORP
                            recEzo = ezoORP;
                            break;
                        case 'C':
                            // CO2
                            recEzo = ezoCO2;
                            hasCal = 0;
                            break;
                        case 'H':
                            // HUM
                            recEzo = ezoHUM;
                            hasCal = 0;
                            break;    
                        case 'E':
                            // EC
                            recEzo = ezoEC;
                            verPos = 6;
                            break;           
                        case 'p':
                            // pH
                            recEzo = ezoPH;
                            verPos = 6;
                            break;
                        case 'D':
                            // dissolved oxygen
                            recEzo = ezoDiO2;
                            verPos = 8;
                            break;
                        case 'P':
                            // Embedded Pressure
                            recEzo = ezoPRES;
                            break;
                        default:
                            // LATE ERROR
                            break;
                        }
                        Serial.println(recEzo);
                        if (recEzo){
                            // Valid Probe found
                            
                            // Save address and type
                            ezoProbe[ezoCnt].address = i;
                            ezoProbe[ezoCnt].type = recEzo;
                            
                            // Extract Version
                            ezoProbe[ezoCnt].version = StrTokFloatToInt(iicStr);
                            // Calibration
                            if (hasCal){
                                IIcSetStr(i,(char*)"Cal,?", 0);
                                delay(333);
                                if (IIcGetAtlas(i) > 0){
                                    ezoProbe[ezoCnt].calibrated = iicStr[5] - 48;
                                }                            
                            }
                            
                            // Name
                            IIcSetStr(i,(char*)"Name,?", 0);
                            delay(333);
                            if (IIcGetAtlas(i) > 0){
                                strcpy(ezoProbe[ezoCnt].name, &iicStr[6]);
                            }

                            // Output (in RAM)
                            Serial.print(F("Found: "));
                            //strcpy_P(strHLP,(PGM_P)pgm_read_word(&(ezoStrType[recEzo])));
                            Serial.print(Fa(ezoStrType[recEzo]));
                            Serial.print(F(" @: "));
                            Serial.println(i);

                            Serial.print(F("         Name: "));
                            Serial.print((char*)ezoProbe[ezoCnt].name);
                            Serial.println(F(""));
                            Serial.print(F("      Version: "));
                            Serial.println(ezoProbe[ezoCnt].version);

                            Serial.print(F("  Calibration: "));
                            Serial.println(ezoProbe[ezoCnt].calibrated);

                            // Output (in Module)
                            // Status
                            IIcSetStr(i, (char*)"Status", 0);
                            Serial.print(F("        State: "));
                            delay(300);
                            if (IIcGetAtlas(i) > 0){
                                switch (iicStr[8]){
                                case 'P':
                                    // powered off
                                    Serial.print(F("'powered off'"));
                                    break;
                                case 'S':
                                    // software reset
                                    Serial.print(F("'software reset'"));
                                    break;
                                case 'B':
                                    // brown out
                                    Serial.print(F("'brown out'"));
                                    break;
                                case 'W':
                                    // watchdog
                                    Serial.print(F("'watchdog'"));
                                    break;
                                case 'U':
                                    // Unknown
                                default:
                                    Serial.print(F("'unknown'"));
                                    break;
                                }             
                                Serial.print(F(" @ "));
                                Serial.print(&iicStr[10]);
                                Serial.println(F(" Volt"));
                            }
                            else{
                                Serial.println(F("ERROR"));
                            }
                            
                            // Value(s)
                            Serial.print(F("     Value(s): "));
                            EzoStartValues(ezoCnt);
                            EzoWaitValues(ezoCnt);
                            if (EzoGetValues(ezoCnt)){
                                Serial.print(ezoProbe[ezoCnt].value[0]);
                                for (byte i2 = 1; i2 < Fb(ezoValCnt[recEzo]); i2++){
                                    Serial.print(F(" , "));
                                    Serial.print(ezoProbe[ezoCnt].value[i2]);
                                }          
                                Serial.println(F(""));
                            }
                            else{
                                Serial.println(F("ERROR"));
                            }
                            Serial.println(F(""));
                            
                            // done
                            ezoCnt++;
                        }
                    } 
                    break;
                }
            }
        }
    }   
}
*/
