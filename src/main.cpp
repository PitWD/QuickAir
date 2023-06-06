#include "quicklib.h"
#include "myMenu.h"
#define WIRE Wire

//void myToRom();
//void myFromRom();

// global RunTime Timing
byte myRunSec = 0;
byte myRunMin = 0;
byte myRunHour = 0;
uint16_t myRunDay = 0;
uint32_t myRunTime = 0;

// global RealTime Timing
byte mySec = 0;
byte myMin = 0;
byte myHour = 0;
byte myDay = 1;
byte myMonth = 1;
uint16_t myYear = 2023;
uint32_t myTime = 0;

void setup() {
  // put your setup code here, to run once:

  myFromRom();
  ManualTimesFromRom(my.Temporary);
  //LowHighValsFromRom(my.Model);
  SettingsFromRom(my.Setting);

  Serial.begin(my.Speed);

  Wire.setClock(31000L);
  Wire.begin();

  delay(300);

  // RTC_GetDateTime();

  delay(300);

  //memset(tooLowSince, 0, sizeof(tooLowSince));
  //memset(lowSince, 0, sizeof(lowSince));
  //memset(highSince, 0, sizeof(highSince));
  //memset(tooHighSince, 0, sizeof(tooHighSince));
  //memset(okSince, 0, sizeof(okSince));
  //memset(lastAction, 0, sizeof(lastAction));

  // OutPorts
  for (byte i = 2; i < 18; i++){
    if (i < 14){
      pinMode(i, OUTPUT);
      digitalWrite(i, LOW);
    }
    else{
      pinMode(i, INPUT_PULLUP);    // Level tooLow, low, high, tooHigh
    }
  }

  for (byte i = 0; i < 6; i++){
    // Set Fail-Save Values to avg_s
    avgVal[i] = setting.FailSaveValue[i];
  }    
// EMERGENCY-BOOT: EzoScan(); my.Default = 0;
  if (my.Default == 1 && my.Cnt && my.Cnt <= EZO_MAX_PROBES - INTERNAL_LEVEL_CNT){ 
    DefaultProbesFromRom();
    ezoCnt = my.Cnt;
    for (byte i = 0; i < ezoCnt; i++){
      // Set Fail-Save Values as Start-Values
      ezoValue[i][0] = setting.FailSaveValue[ezoProbe[i].type];
    }    
    PrintLoopMenu();
  }
  else{
    EzoScan();
    PrintMainMenu();
  }
}

uint32_t ValidTimeSince(uint32_t valIN){
  if (!valIN){
    return 0;
  }
  return myTime - valIN;
}
uint32_t checkAction(uint32_t valIN, uint32_t actionTime, byte ezotype, byte i, byte isHighPort, byte *backSet){

  uint32_t r = valIN;
  *backSet = 0;

  // If something is OnAction
  if (ValidTimeSince(valIN) > setting.DelayTime[ezotype]){
    // Action Valid
    if ((ValidTimeSince(valIN) - setting.DelayTime[ezotype]) > actionTime){
      // ActionTime done
      lastAction[i] = myTime;
      r = 0;
    }
    else{
      // DoAction
      *backSet = 1;
    }
  }
  else{
    // NoAction
  }
  
  // Port 2-8 - Action ports for low / tooLow
      // Temp / EC-1 / EC-2 / EC-3 / pH / redox / O2 / level
  // Port 9-11 - Action ports for high / tooHigh
      // Temp / EC / pH / level
  //digitalWrite(ezotype + 2 + (isHighPort * 6), *backSet);
  if (isHighPort){
    switch (i){
    case 2 ... 3:
      // 2nd & 3rd EC
    case 6:
      // O2  
      i = 0;
      break;
    case 5:
      // ORPs low-port is high-port, too
      // cause (too) high/low ORPs need the same action (change/waste water)
      i = 7;
      break;
    case 4:
      // pH
      i = 12;
      break;
    case 7:
      // Level
      i = 13;
      break;
    default:
      i += 10;
      break;
    }
  }
  else{
    i += 2;
  }
  
  if (i){
    digitalWrite(i, *backSet);
  }

  return r;

}

void loop() {
// put your main code here, to run repeatedly:

  if (DoTimer()){
    // A Second is over...

    byte err = 1;

    uint32_t preToo = 0;

    PrintPortStates();

    PrintLoopTimes();    

    // Check High/Low of AVGs 
    // compare timeOuts with timing-setting
    //for (byte i = 0; i < 6; i++){
    for (byte i = 0; i < 8; i++){

      byte type = i;
      // Correct type for the three times EC
      if (i == 2){
        type = 1;
      }
      else if (i > 2){
        type -= 2;
      }

      // Check On needed/pending low-actions
      preToo = tooLowSince[i];
      tooLowSince[i] = checkAction(tooLowSince[i], setting.TimeTooLow[i], type, i, 0, &err);
      if (!err){
        // TooLow isn't in Action...
        lowSince[i] = checkAction(lowSince[i], setting.TimeLow[i], type, i, 0, &err);
        //if (preToo != tooLowSince[i]){ 
        if (preToo != tooLowSince[i]){ 
          // after finished tooXYZ-Action - reset lowSince, too          
          lowSince[i] = 0;
          err = 0;
        }      
        if (err){
          // Low in Action
        }
      }
      else{
        //  TooLow in Action
      }
      if (err){
        // something is in action
      }
      

      // Check On needed/pending high-actions
      preToo = tooHighSince[i];
      
      // we've just 4 high-actions... (Temp, EC, pH, Level)
      byte j = i;
      switch (i){
      case 2 ... 3:
        // 2nd & 3rd EC
      case 6:
        // O2
        j = 0;
        break;
      case 5:
        // ORP
        // j = 5;
        break;
      case 4:
        j = 2;
        break;
      case 7:
        j = 3;
      default:
        break;
      }    

      uint16_t highToUse = setting.TimeHigh[j];
      uint16_t tooHighToUse = setting.TimeTooHigh[j];
      if (j == 5){
        // ORPs (too)High timing is same like ORPs (too)Low timing
        // ORP can't be fixed if it's high/low - change/waste water is the only option
        highToUse = setting.TimeLow[j];
        tooHighToUse = setting.TimeTooLow[j];
      }
      
      
      if (j || (!j && !i)){
        
        tooHighSince[i] = checkAction(tooHighSince[i], tooHighToUse, type, i, 1, &err);
        if (!err){
          // TooHigh isn't in Action...
          highSince[i] = checkAction(highSince[i], highToUse, type, i, 1, &err);
          if (preToo != tooHighSince[i]){ 
            // after finished tooXYZ-Action - reset highSince, too
            highSince[i] = 0;
            err = 0;
          }
          if (err){
            // High in Action
          }
        }
        else{
          //  TooHigh in Action
        }
        if (err){
          // something is in action
        }
      }
      
      // Set / Reset Since-Variables depending on high/low state...
      switch (GetAvgState(avgVal[type], setting.ValueTooLow[type], setting.ValueLow[type], setting.ValueHigh[type], setting.ValueTooHigh[type])){
      case fgCyan:
        // tooLow
        highSince[i] = 0;
        tooHighSince[i] = 0;
        okSince[i] = 0;
        if (!tooLowSince[i]){
          // 1st time tooLow recognized
          tooLowSince[i] = myTime;
        }
        if (!lowSince[i]){
          // If tooXYZ is active... regular state becomes active too
          lowSince[i] = tooLowSince[i];
        }  
        break;
      case fgBlue:
        // Low
        highSince[i] = 0;
        tooHighSince[i] = 0;
        tooLowSince[i] = 0;
        okSince[i] = 0;
        if (!lowSince[i]){
          // 1st time Low recognized
          lowSince[i] = myTime;
        }
        break;
      case fgRed:
        // tooHigh
        lowSince[i] = 0;
        tooLowSince[i] = 0;
        okSince[i] = 0;
        if (!tooHighSince[i]){
          // 1st time tooLow recognized
          tooHighSince[i] = myTime;
        }
        if (!highSince[i]){
          // If tooXYZ is active... regular state becomes active too
          highSince[i] = tooHighSince[i];
        }
        break;
      case fgYellow:
        // High
        lowSince[i] = 0;
        tooLowSince[i] = 0;
        tooHighSince[i] = 0;
        okSince[i] = 0;
        if (!highSince[i]){
          // 1st time High recognized
          highSince[i] = myTime;
        }
        break;
      default:
        // OK
        // Reset ...Since Vars
        tooLowSince[i] = 0;
        lowSince[i] = 0;
        highSince[i] = 0;
        tooHighSince[i] = 0;
        if (!okSince[i]){
          okSince[i] = myTime;
        }
        break;
      }
      
    }

    // Check for EC synchronization
    err = 0;   // use to save if one of the EC-Ports is active
    for (byte i = 3; i < 6; i++){
      // Low Ports
      if (digitalRead(i)){
        err = 1;
      }          
    }
    if (digitalRead(11)){
      // High Port
      err = 1;
    }          
    
    uint32_t minmax;
    if (!err){
      // Al EC action-ports are OFF
      minmax = tooLowSince[1];  // use type as highest / lowest
      for (byte i = 2; i < 4; i++){
        minmax = ((minmax) > (tooLowSince[i]) ? (minmax) : (tooLowSince[i]));
      }
      for (byte i = 1; i < 4; i++){
        tooLowSince[i] = minmax;
      }
      minmax = lowSince[1];  // use type as highest / lowest
      for (byte i = 2; i < 4; i++){
        minmax = ((minmax) > (lowSince[i]) ? (minmax) : (lowSince[i]));
      }
      for (byte i = 1; i < 4; i++){
        lowSince[i] = minmax;
      }

      minmax = highSince[1];  // use type as highest / lowest
      for (byte i = 2; i < 4; i++){
        minmax = ((minmax) > (highSince[i]) ? (minmax) : (highSince[i]));
      }
      for (byte i = 1; i < 4; i++){
        highSince[i] = minmax;
      }
      minmax = tooHighSince[1];  // use type as highest / lowest
      for (byte i = 2; i < 4; i++){
        minmax = ((minmax) > (tooHighSince[i]) ? (minmax) : (tooHighSince[i]));
      }
      for (byte i = 1; i < 4; i++){
        tooHighSince[i] = minmax;
      }
    }

    //Read EZO's
    if (EzoDoNext() == 1){
      // All read
      err = PrintWaterVals(5);
      PrintAVGs(err + 1);
    } 
  }

  if (Serial.available()){
    Serial.read();
    OffOutPorts();
    PrintMainMenu();
  }

}