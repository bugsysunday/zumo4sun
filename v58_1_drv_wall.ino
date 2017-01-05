#include <Wire.h>
#include <Zumo32U4.h>
Zumo32U4LCD lcd;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4LineSensors lineSensors;
unsigned int lineSensorValues[3];
const uint16_t lineSensorThreshold = 1000;
const uint16_t reverseSpeed = 200;
const uint16_t turnSpeed = 200;
const uint16_t forwardSpeed = 200;
const uint16_t veerSpeedLow = 0;
const uint16_t veerSpeedHigh = 250;
const uint16_t fullSpeed = 400;
const uint16_t reverseTime = 200;
const uint16_t scanTimeMin = 200;
const uint16_t scanTimeMax = 2100;
const uint16_t waitTime = 5000;
const uint16_t stalemateTime = 4000;
uint16_t stateStartTime;
uint16_t accSpeed=100;
uint16_t displayTime;
uint8_t sum ;
int8_t diff ;
bool justChangedState;
bool displayCleared;
bool proxLeftActive;
bool proxFrontActive;
bool proxRightActive;

enum State
{
  StateScanning,
  StatePausing,
  StateWaiting,
  StateDriving,
  StateBacking,
  StateSense
};

State state = StatePausing;


void setup()
{
  proxSensors.initThreeSensors();
  lineSensors.initThreeSensors();
  changeState(StatePausing);
  loadCustomCharacters();
}



void loop()
{
  //bool buttonPress = buttonC.getSingleDebouncedPress();
  bool buttonSense = buttonB.getSingleDebouncedPress();
  bool buttonPress = buttonA.getSingleDebouncedPress();
  if (state == StatePausing)
  {
    motors.setSpeeds(0, 0);
    if (justChangedState)
        {
          justChangedState = false;
          lcd.gotoXY(0,0);
          lcd.print("A/B/C");
          
        }

    if (displayIsStale(100))
        {
          displayUpdated();
          lcd.gotoXY(0, 1);
          lcd.print(readBatteryMillivolts());
        }

    if (buttonSense)
      {
        // The user pressed button B, so go to the waiting state.
        changeState(StateSense);
      }

    else if (buttonPress)
      {
        // The user pressed button A, so go to the waiting state.
        changeState(StateWaiting);
      }
      
  }
  else if (buttonPress || buttonSense)
  {
    // The user pressed button A while the robot was running, so pause.
    changeState(StatePausing);
  }
  else if (state == StateWaiting)
  {
    // In this state, we wait for a while and then move on to the
    // scanning state.

    motors.setSpeeds(0, 0);

    uint16_t time = timeInThisState();

    if (time < waitTime)
    {
      // Display the remaining time we have to wait.
      uint16_t timeLeft = waitTime - time;
      lcd.gotoXY(0, 0);
      lcd.print(timeLeft / 1000 % 10);
      lcd.print('.');
      lcd.print(timeLeft / 100 % 10);
    }
    else
    {
      // We have waited long enough.  Start moving.
      changeState(StateScanning);
    }
  }
  else if (state == StateSense)
  {
       if (justChangedState)
    {
      justChangedState = false;
      lcd.print("sense");
    }

    motors.setSpeeds(0, 0);
    while (!buttonSense)
    {
      proxSensors.read();
      delay(20);
      lcd.gotoXY(0, 0);
      printBar(proxSensors.countsLeftWithLeftLeds());
      printBar(proxSensors.countsLeftWithRightLeds());
      lcd.print(' ');
      printBar(proxSensors.countsFrontWithLeftLeds());
      printBar(proxSensors.countsFrontWithRightLeds());
      lcd.print(' ');
      printBar(proxSensors.countsRightWithLeftLeds());
      printBar(proxSensors.countsRightWithRightLeds());
      lcd.gotoXY(0, 1);
      lcd.print(proxSensors.countsLeftWithLeftLeds());
      lcd.print(proxSensors.countsLeftWithRightLeds());
      lcd.print(' ');
      lcd.print(proxSensors.countsFrontWithLeftLeds());
      lcd.print(proxSensors.countsFrontWithRightLeds());
      lcd.print(' ');
      lcd.print(proxSensors.countsRightWithLeftLeds());
      lcd.print(proxSensors.countsRightWithRightLeds());
    }
    
    
  }

  
  else if (state == StateBacking)
  {
    // In this state, the robot drives in reverse.

    if (justChangedState)
    {
      justChangedState = false;
      lcd.print(F("back"));
    }
    ledRed(1);
    motors.setSpeeds(-reverseSpeed, -reverseSpeed);

    // After backing up for a specific amount of time, start
    // scanning.
    if (timeInThisState() >= reverseTime)
    {
      changeState(StateScanning);
    }
  }
  else if (state == StateScanning)
      {
        if(justChangedState)
          {
            justChangedState = false;
            lcd.gotoXY(0, 0);
            printBar(proxSensors.countsLeftWithLeftLeds());
            printBar(proxSensors.countsLeftWithRightLeds());
            lcd.print(' ');
            printBar(proxSensors.countsFrontWithLeftLeds());
            printBar(proxSensors.countsFrontWithRightLeds());
            lcd.print(' ');
            printBar(proxSensors.countsRightWithLeftLeds());
            printBar(proxSensors.countsRightWithRightLeds());
            lcd.gotoXY(0, 1);
            lcd.print(proxSensors.countsLeftWithLeftLeds());
            lcd.print(proxSensors.countsLeftWithRightLeds());
            lcd.print(' ');
            lcd.print(proxSensors.countsFrontWithLeftLeds());
            lcd.print(proxSensors.countsFrontWithRightLeds());
            lcd.print(' ');
            lcd.print(proxSensors.countsRightWithLeftLeds());
            lcd.print(proxSensors.countsRightWithRightLeds());
          }
        proxSensors.read();
        sum = proxSensors.countsFrontWithRightLeds() + proxSensors.countsFrontWithLeftLeds();
        diff = proxSensors.countsFrontWithRightLeds() - proxSensors.countsFrontWithLeftLeds();
        //rsum = proxSensors.countsFrontWithRightLeds() + proxSensors.countsRightWithLeftLeds();
        //lsum = 

        changeState(StateDriving);
      }


          
  
   else if (state == StateDriving)
      {
        if(justChangedState)
          {
            justChangedState = false;
            lcd.print("drv");
          }
          if(0==sum)
          {
          if (justChangedState)
            {
              justChangedState = false;
              lcd.gotoXY(0,0);
              lcd.print("full");
            }
            ledGreen(1);
            motors.setSpeeds(fullSpeed, fullSpeed);
            changeState(StateScanning);
         }
        else if(4>sum && diff==0) //|| ((proxSensors.countsRightWithLeftLeds() <=4) && (proxSensors.countsFrontWithLeftLeds()<=5)) ||  ((proxSensors.countsLeftWithRightLeds() <=4) && (proxSensors.countsFrontWithRightLeds()<=5))|| (( proxSensors.countsLeftWithLeftLeds()>=5) && (proxSensors.countsFrontWithLeftLeds()<=4 )) || ((proxSensors.countsRightWithRightLeds()>=5) && (proxSensors.countsFrontWithRightLeds()<=4)) )
          {
          if (justChangedState)
            {
              justChangedState = false;
              lcd.gotoXY(0,0);
              lcd.print("accel");
            }
            ledGreen(1);
            accSpeed++;
            motors.setSpeeds(accSpeed, accSpeed);
            changeState(StateScanning);
         }
        
        
        else if(diff>=0)  /// linksfahren
        {
         if(proxSensors.countsFrontWithRightLeds()>=5)
            {
            if (justChangedState)
                  {
                    justChangedState = false;
                    lcd.gotoXY(0,0);
                    lcd.print("lele");
                  }
                ledYellow(1);
                motors.setSpeeds(-turnSpeed, turnSpeed);
                changeState(StateScanning);
                }
        else if(proxSensors.countsFrontWithRightLeds()<5)
          {
          if (justChangedState)
            {
              justChangedState = false;
              lcd.gotoXY(0,0);
              lcd.print("le");
            }
           ledYellow(1);
           motors.setSpeeds(veerSpeedLow, veerSpeedHigh);
           changeState(StateScanning);
          }
        }
        
        
        
        else if(diff<= 0) /// rechtsfahren
        {
         if (proxSensors.countsFrontWithLeftLeds()>=5)
              {
                if (justChangedState)
                    {
                      justChangedState = false;                      
                      lcd.gotoXY(0,0);
                      lcd.print("riri");
                    }
                    ledYellow(1);                   
                    motors.setSpeeds(turnSpeed, -turnSpeed);
                    changeState(StateScanning);
                    }
        else if(proxSensors.countsFrontWithLeftLeds()<5)
         {
              if (justChangedState)
                  {
                    justChangedState = false;
                    lcd.gotoXY(0,0);
                    lcd.print("ri");
                  }
                  ledYellow(1);
                  motors.setSpeeds(veerSpeedHigh, veerSpeedLow);
                  changeState(StateScanning);
                  }
          }
          else               
                {
                  if (justChangedState)
                      {
                       justChangedState = false;                      
                       lcd.gotoXY(0,0);
                       lcd.print("emp");
                      }
                      
                      ledRed(1);
                      ledYellow(1);
                      ledGreen(1);
                      delay(100);
                      ledRed(0);
                      ledYellow(0);
                      ledGreen(0);
                      delay(100);
                  changeState(StateScanning);
                 }
              }
      }
 

 


  
// Gets the amount of time we have been in this state, in
// milliseconds.  After 65535 milliseconds (65 seconds), this
// overflows to 0.
uint16_t timeInThisState()
{
  return (uint16_t)(millis() - stateStartTime);
}

// Changes to a new state.  It also clears the LCD and turns off
// the LEDs so that the things the previous state were doing do
// not affect the feedback the user sees in the new state.
void changeState(uint8_t newState)
{
  state = (State)newState;
  justChangedState = true;
  stateStartTime = millis();
  ledRed(0);
  ledYellow(0);
  ledGreen(0);
  lcd.clear();
  displayCleared = true;
}

// Returns true if the display has been cleared or the contents
// on it have not been updated in a while.  The time limit used
// to decide if the contents are staled is specified in
// milliseconds by the staleTime parameter.
bool displayIsStale(uint16_t staleTime)
{
  return displayCleared || (millis() - displayTime) > staleTime;
}

// Any part of the code that uses displayIsStale to decide when
// to update the LCD should call this function when it updates the
// LCD.
void displayUpdated()
{
  displayTime = millis();
  displayCleared = false;
}


void loadCustomCharacters()
{
  static const char levels[] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63
  };
  lcd.loadCustomCharacter(levels + 0, 0);  // 1 bar
  lcd.loadCustomCharacter(levels + 1, 1);  // 2 bars
  lcd.loadCustomCharacter(levels + 2, 2);  // 3 bars
  lcd.loadCustomCharacter(levels + 3, 3);  // 4 bars
  lcd.loadCustomCharacter(levels + 4, 4);  // 5 bars
  lcd.loadCustomCharacter(levels + 5, 5);  // 6 bars
  lcd.loadCustomCharacter(levels + 6, 6);  // 7 bars
}

void printBar(uint8_t height)
{
  if (height > 8) { height = 8; }
  const char barChars[] = {' ', 0, 1, 2, 3, 4, 5, 6, 255};
  lcd.print(barChars[height]);
}
