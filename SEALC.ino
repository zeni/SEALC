/****************** PLC = Physical Live Coding ********************
  Live coding on stepper, servo steppers and pots, piezo, hammer, tools,
  objects, etc. to create sounds.

  By 23N!
  Created: 2018/08/31
  Updated: 2018/09/11

  TODO:
  - Error messages
  - wave as a rotate option ?
  - servo: use writeMicroseconds instead ?
  - Stepper: keep track of zero/origin ?
*******************************************************************/

#include "Stepper.h"

#define MAX_STEPPERS 21
// pins
#define EN 8 // stepper motor enable, low level effective
// serial commands
#define SEPARATOR ','
#define EOL '\n'
#define COLUMN ':'
// commands
#define COMMAND_SELECT 25
#define COMMAND_SS 10      // Set Speed
#define COMMAND_SD MODE_SD // Set Direction
#define COMMAND_ST MODE_ST // STop
#define COMMAND_RA MODE_RA // Rotate Angle (stepper) / Rotate Absolute (servo)
#define COMMAND_NONE MODE_IDLE
#define COMMAND_RW MODE_RW // Rotate Wave => not usable as such !
#define COMMAND_SQ MODE_SQ // SeQuence
#define COMMAND_ERROR 66
#define COMMAND_SA 24      // Stop All
#define COMMAND_RR MODE_RR // Rotate Angle (stepper) / Rotate Relative (servo)
#define COMMAND_WA MODE_WA // WAit command (ms)
// states
#define STATE_SETUP 0
#define STATE_READY 1

// vars
Stepper *steppers[MAX_STEPPERS];
int selectedStepper;
int currentCommand, currentValue;
bool firstChar = true;
char command[2];
int iCommand;
int nSteppers, iSteppers;
int state;
int nStepperArg, iStepperArg;
int args[3];

void displaySelectedMotor()
{
  Serial.print(">> selected motor: ");
  Serial.print(selectedStepper);
}

void displayIntro()
{
  Serial.println(">************************************************");
  Serial.println("SEALC: System for Electo-Acoustic Live Coding");
  Serial.println("by S. Shibatsuji-Perrin - 2018/09");
  Serial.print("************************************************<");
}

void updateValue(char a)
{
  if (currentValue < 0)
    currentValue = 0;
  currentValue *= 10;
  currentValue += (a - 48);
}

void processCommand(char a)
{
  if ((a >= 48) && (a < 58))
  {
    if (firstChar)
    {
      currentCommand = COMMAND_SELECT;
      selectedStepper = a - 48;
      if (selectedStepper >= nSteppers)
        selectedStepper = nSteppers - 1;
      displaySelectedMotor();
      firstChar = false;
    }
    else
      updateValue(a);
  }
  else
    command[iCommand++] = a;
  if (iCommand == 1)
  {
    switch (command[0])
    {
    case COLUMN:
      if (firstChar)
        currentCommand = COMMAND_ERROR;
      else
      {
        switch (currentCommand)
        {
        case COMMAND_SQ:
          steppers[selectedStepper]->columnSQ(currentValue);
          break;
        }
      }
      currentValue = -1;
      iCommand = 0;
      command[0] = 0;
      break;
    case SEPARATOR:
    case EOL:
      if (firstChar)
        currentCommand = COMMAND_NONE;
      switch (currentCommand)
      {
      case COMMAND_SS:
        steppers[selectedStepper]->SS(currentValue);
        break;
      case COMMAND_SD:
      case COMMAND_ST:
      case COMMAND_RA:
      case COMMAND_RR:
      case COMMAND_RW:
      case COMMAND_SQ:
      case COMMAND_WA:
        steppers[selectedStepper]->fillQ(currentCommand, currentValue);
        break;
      case COMMAND_SA:
        for (int i = 0; i < nSteppers; i++)
          steppers[i]->ST();
        break;
      case COMMAND_SELECT:
      case COMMAND_ERROR:
      case COMMAND_NONE:
        break;
      }
      currentValue = -1;
      firstChar = true;
      iCommand = 0;
      command[0] = 0;
      break;
    }
  }
  else if (iCommand == 2)
  {
    currentCommand = COMMAND_ERROR;
    switch (command[0])
    {
    case 'S':
      switch (command[1])
      {
      case 'S':
        currentCommand = COMMAND_SS; //SS
        break;
      case 'D':
        currentCommand = COMMAND_SD; //SD
        break;
      case 'T':
        currentCommand = COMMAND_ST; //ST
        break;
      case 'A':
        currentCommand = COMMAND_SA; //ST
        break;
      case 'Q':
        currentCommand = COMMAND_SQ; //SQ
        steppers[selectedStepper]->initSQ();
        break;
      }
      break;
    case 'R':
      switch (command[1])
      {
      case 'W':
        currentCommand = COMMAND_RW; //RW
        break;
      case 'A':
        currentCommand = COMMAND_RA; //RA
        break;
      case 'R':
        currentCommand = COMMAND_RR; //RA
        break;
      }
      break;
    case 'W':
      switch (command[1])
      {
      case 'A':
        currentCommand = COMMAND_WA; //WA
        break;
      }
      break;
    }
    currentValue = -1;
    firstChar = false;
    iCommand = 0;
    command[0] = 0;
    command[1] = 0;
  }
}

void processSetup(char a)
{
  if ((a >= 48) && (a < 58))
  {
    updateValue(a);
  }
  else
  {
    switch (a)
    {
    case EOL:
    case SEPARATOR:
      if (nSteppers == 0)
      {
        nSteppers = currentValue;
        iSteppers = 0;
        currentValue = -1;
      }
      else
      {
        args[iStepperArg++] = currentValue;
        currentValue = -1;
        if (iStepperArg == 3)
        {
          steppers[iSteppers] = new Stepper(args[0], args[1], args[2]);
          iSteppers++;
          if (iSteppers == nSteppers)
          {
            Serial.println("");
            displaySelectedMotor();
            currentCommand = COMMAND_NONE;
            state = STATE_READY;
          }
          else
            iStepperArg = 0;
        }
      }
      break;
    default:
      currentValue = -1;
      break;
    }
  }
}

void setup()
{
  nSteppers = 0;
  state = STATE_SETUP;
  nStepperArg = 0;
  iStepperArg = 0;
  selectedStepper = 0;
  Serial.begin(115200);
  displayIntro();
  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);
  currentValue = -1;
  currentCommand = COMMAND_NONE;
  command[0] = 0;
  command[1] = 0;
  iCommand = 0;
}

void loop()
{
  switch (state)
  {
  case STATE_READY:
    if (Serial.available())
    {
      char a = Serial.read();
      processCommand(a);
    }
    for (int i = 0; i < nSteppers; i++)
      steppers[i]->action();
    break;
  case STATE_SETUP:
    if (Serial.available())
    {
      char a = Serial.read();
      processSetup(a);
    }
    break;
  }
}
