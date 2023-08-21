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
  ManualFromRom();
  StepperFromRom();
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

  // Stepper...
  for (byte i = 0; i < 3; i++){
    StepperWrite(i, stepperStartStep(i));
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
uint32_t checkAction(uint32_t valIN, uint32_t actionTime, byte timeID, byte i, byte isHighPort, byte *backSet){

  uint32_t r = valIN;
  *backSet = 0;

  // If something is OnAction
  if (ValidTimeSince(valIN) > setting.DelayTime[timeID]){
    // Action Valid
    if ((ValidTimeSince(valIN) - setting.DelayTime[timeID]) > actionTime){
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
  
  i += 2;     // Port to set (if isLowPort)
  // Port 2   heat (RTD low/tooLow)
  // Port 3   humidify (HUM low/tooLow)
  // Port 4   raise CO2 (CO2 low/tooLow)
  // Port 5   heat or dry (DEW close/tooClose)
  if (isHighPort){
    i += 4;
  }
  // Port 6   cool (RTD high/tooHigh)
  // Port 7   dry (HUM high/tooHigh)

  if (manual[i - 2].State){
    // permanent State
    *backSet = manual[i - 2].PermVal;  
  }

  if (manTempTime[i - 2]){
    // A temporary Action is active
    if (manTempTime[i - 2] > myTime){
      // Action still valid - set Value
      *backSet = manual[i - 2].TempVal;
    }
    else{
      // Action Time expired
      manTempTime[i - 2] = 0;
    }  
  }
  else{
    // No temporary action...    
  }
  
    
  digitalWrite(i, *backSet);

  return r;

}

void loop() {
// put your main code here, to run repeatedly:

  if (DoTimer()){
    // A Second is over...

    byte err = 1;

    uint32_t preToo = 0;

    PrintPortStates();
    if (!my.Boot){
      PrintLoopTimes();    
    }
    else if (my.Boot < 3){
      // ModBus RTU & AscII
    }
    else{
      // just values - send heart-beat
      MBstart(my.Address);
      // iicStr[2] = type;    // 0 = QuickTimer, 1 = QuickWater, 2 = QuickAir
      iicStr[2] = 4;          // HeartBeat
      MBaddLong(myTime, 3);
      MBstop(7);
    }

    // Check High/Low of AVGs 
    // compare action-ports timeOuts with timing-setting
    for (byte i = 0; i < 4; i++){

      // MemPos of *Time* in setting...
      byte timeID = 0;      
      if (i){
        timeID += 4;
      }

      // Check On needed/pending low-actions
      preToo = tooLowSince[i];
      tooLowSince[i] = checkAction(tooLowSince[i], setting.TimeTooLow[timeID], timeID, i, 0, &err);
      if (!err){
        // TooLow isn't in Action...

        lowSince[i] = checkAction(lowSince[i], setting.TimeLow[timeID], timeID, i, 0, &err);
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
      if (!i || i == ezoHUM){
        
        // Check On needed/pending high-actions
        preToo = tooHighSince[i];

        tooHighSince[i] = checkAction(tooHighSince[i], setting.TimeTooHigh[timeID], timeID, i, 1, &err);
        if (!err){
          // TooHigh isn't in Action...
          highSince[i] = checkAction(highSince[i], setting.TimeHigh[timeID], timeID, i, 1, &err);
          
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

    // Check On Stepper Actions...
    for (byte i = 0; i < 3; i++){
      CheckOnStep(i);
    }

      int32_t highToUse;
      int32_t tooHighToUse;
      
      // we've just 2 high-actions... (RTD, HUM)
      if (!i || i == ezoHUM){
        highToUse = setting.ValueHigh[i];
        tooHighToUse = setting.ValueTooHigh[i];
      }
      else{
        // simulate very high "(too)HighValues" for CO2 & DEW
        highToUse = 999999999;
        tooHighToUse = highToUse;
      }
      
      // Set / Reset Since-Variables depending on high/low state...
      avgState[i] = ColorStateToStepState(GetAvgState(avgVal[i], setting.ValueTooLow[i], setting.ValueLow[i], highToUse, tooHighToUse));
      switch (avgState[i]){
      case -2:
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
      case -1:
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
      case 2:
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
      case 1:
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
    
    //Read EZO's
    if (EzoDoNext() == 1){
      // All read
      err = PrintWaterVals(5);

      // Correct AVGs
      avg_DewVal = avg_DEW;               // Save Hardware Value for eventually broken Sensor
      avg_DEW = avg_TMP - avg_DEW;        // absolute temp of dewing point doesn't matter
                                          // Differenz to actual temp makes the point...

      avg_RTD = (avg_RTD + avg_TMP) / 2;  // Temp calculations are based on "ezoRTD",
                                          // but avg of RTD is avg of TMP and RTD
      if (!my.Boot){
        PrintAVGs(err + 1);
      }
    } 
  }

  if (GetONEchar()){
    if (!my.Boot){
      //OffOutPorts();
      PrintMainMenu();
    }
    else{
      // We're not in Terminal-Mode
      // Force 1x values output
      portStateFirstRun = 0;
    }
  }

}