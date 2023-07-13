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
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  for (byte i = 0; i < 4; i++){
    // Set Fail-Save Values to avg_s
    avgVal[i] = setting.FailSaveValue[i];
  }    

// EMERGENCY-BOOT: EzoScan(); my.Default = 0;

  if (my.Default == 1 && my.Cnt && my.Cnt <= EZO_MAX_PROBES){ 
    DefaultProbesFromRom();
    ezoCnt = my.Cnt;
    for (byte i = 0; i < ezoCnt; i++){
      // Set Fail-Save Values as Start-Values
      ezoValue[i][0] = setting.FailSaveValue[ezoProbe[i].type];
      if (ezoProbe[i].type == ezoHUM){
        ezoValue[i][1] = setting.FailSaveValue[ezoRTD];
        ezoValue[i][2] = setting.FailSaveValue[ezoDEW];
      }
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
uint32_t checkAction(uint32_t valIN, uint32_t actionTime, byte i, byte isHighPort, byte *backSet){

  uint32_t r = valIN;
  *backSet = 0;

  // If something is OnAction
  if (ValidTimeSince(valIN) > setting.DelayTime[i]){
    // Action Valid
    if ((ValidTimeSince(valIN) - setting.DelayTime[i]) > actionTime){
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
  
  // Port 2   heat (RTD low/tooLow)
  // Port 3   humidify (HUM low/tooLow)
  // Port 4   raise CO2 (CO2 low/tooLow)
  // Port 5   heat or dry (DEW close/tooClose)
  // Port 6   cool (RTD high/tooHigh)
  // Port 7   dry (HUM high/tooHigh)

  if (isHighPort){
    switch (i){
    case 0:
      // RTD
      i = 6;
      break;
    case 4:
      // HUM
      i = 7;
      break;
    default:
      i = 0;
      break;
    }
  }
  else{
    // LowPort
    switch (i){
    case 0:
      // RTD
      i = 2;
      break;
    case 4 ... 6:
      // HUM / CO2 / DEW
      i -= 1;
      break;
    default:
      i = 0;
      break;
    }
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
    for (byte i = 0; i < 7; i++){

      byte type;      
      // Correct type for the four times EC
      if (i < 4){
        type = ezoRTD;
      }
      else{
        type = i - 3;
      }

      // Check On needed/pending low-actions
      preToo = tooLowSince[i];
      tooLowSince[i] = checkAction(tooLowSince[i], setting.TimeTooLow[i], i, 0, &err);
      if (!err){
        // TooLow isn't in Action...
        lowSince[i] = checkAction(lowSince[i], setting.TimeLow[i], i, 0, &err);
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
            
      // we've just 2 high-actions... (RTD, HUM)
      byte j;
      switch (i){
      case 0 ... 3:
        // All Temperatures
        j = ezoRTD;
      case 4:
        // Humidity
        j = ezoHUM;
        break;
      default:
        j = 0;
        break;
      }          

      // Check On needed/pending high-actions
      preToo = tooHighSince[j];

      if (j || (!j && !i)){
        
        tooHighSince[j] = checkAction(tooHighSince[j], setting.ValueTooHigh[j], i, 1, &err);
        if (!err){
          // TooHigh isn't in Action...
          highSince[j] = checkAction(highSince[j], setting.ValueHigh[j], i, 1, &err);
          if (preToo != tooHighSince[j]){ 
            // after finished tooXYZ-Action - reset highSince, too
            highSince[j] = 0;
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
        if (i < 2){
          highSince[i] = 0;
          tooHighSince[i] = 0;
        }
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
        if (i < 2){
          highSince[i] = 0;
          tooHighSince[i] = 0;
        }
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
        if (i < 2){
          if (!tooHighSince[i]){
            // 1st time tooLow recognized
            tooHighSince[i] = myTime;
          }
          if (!highSince[i]){
            // If tooXYZ is active... regular state becomes active too
            highSince[i] = tooHighSince[i];
          }
        }
        break;
      case fgYellow:
        // High
        lowSince[i] = 0;
        tooLowSince[i] = 0;
        tooHighSince[i] = 0;
        okSince[i] = 0;
        if (i < 2){
          if (!highSince[i]){
            // 1st time High recognized
            highSince[i] = myTime;
          }
        }
        break;
      default:
        // OK
        // Reset ...Since Vars
        tooLowSince[i] = 0;
        lowSince[i] = 0;
        if (i < 2){
          highSince[i] = 0;
          tooHighSince[i] = 0;
        }
        if (!okSince[i]){
          okSince[i] = myTime;
        }
        break;
      }
      
    }

    //Read EZO's
    if (EzoDoNext() == 1){
      // All read
      err = PrintWaterVals(5);

      // Correct AVGs
      avg_RTD = (avg_RTD + avg_TMP) / 2;  // All calculations are based on "ezoRTD",
                                          // but avg of RTD is avg of TMP and RTD
      
      avg_DEW = avg_RTD - avg_DEW;        // absolute temp of dewing point doesn't matter
                                          // Differenz to actual temp makes the point...
      PrintAVGs(err + 1);
    } 
  }

  if (Serial.available()){
    Serial.read();
    OffOutPorts();
    PrintMainMenu();
  }

}