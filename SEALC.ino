/****************** PLC = Physical Live Coding ********************
  Live coding on stepper, servo motors and pots, piezo, hammer, tools,
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

#include "Motor.h"
#include "Stepper.h"
#include "Servomotor.h"
#include "Vibro.h"

#define MAX_MOTORS 10
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
#define COMMAND_RO MODE_RO // ROtate
#define COMMAND_ST MODE_ST // STop
#define COMMAND_RA MODE_RA // Rotate Angle (stepper) / Rotate Absolute (servo)
#define COMMAND_NONE MODE_IDLE
#define COMMAND_RW MODE_RW // Rotate Wave
#define COMMAND_SQ MODE_SQ // SeQuence
#define COMMAND_ERROR 66
#define COMMAND_RP MODE_RP // Rotate Pause
#define COMMAND_GS 20      // Get Speed
#define COMMAND_GD 21      // Get Direction
#define COMMAND_GM 22      // Get Mode
#define COMMAND_GI 23      // Get Id
#define COMMAND_SA 24      // Stop All
#define COMMAND_RR MODE_RR // Rotate Angle (stepper) / Rotate Relative (servo)
#define COMMAND_WA MODE_WA // WAit command (ms)
// states
#define STATE_SETUP 0
#define STATE_READY 1
// motor types
#define TYPE_STEPPER 0
#define TYPE_SERVO 1
#define TYPE_VIBRO 2
#define TYPE_UNKNOWN -1

// vars
Motor *motors[MAX_MOTORS];
int selectedMotor;
int currentCommand, currentValue;
bool firstChar = true;
char command[2];
int iCommand;
int nMotors, iMotors;
int state;
int motorType, nMotorArg, iMotorArg;
int args[3];

void displaySelectedMotor()
{
  Serial.print(">> selected motor: ");
  Serial.print(selectedMotor);
  Serial.println(motors[selectedMotor]->getType());
}

void displayIntro()
{
  Serial.println("*************************************************");
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
      selectedMotor = a - 48;
      if (selectedMotor >= nMotors)
        selectedMotor = nMotors - 1;
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
          motors[selectedMotor]->columnSQ(currentValue);
          break;
        case COMMAND_RP:
          motors[selectedMotor]->columnRP(currentValue);
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
        motors[selectedMotor]->SS(currentValue);
        break;
      case COMMAND_SD:
      case COMMAND_RO:
      case COMMAND_ST:
      case COMMAND_RA:
      case COMMAND_RR:
      case COMMAND_RW:
      case COMMAND_SQ:
      case COMMAND_RP:
      case COMMAND_WA:
        motors[selectedMotor]->fillQ(currentCommand, currentValue);
        break;
      case COMMAND_GS:
        motors[selectedMotor]->GS();
        break;
      case COMMAND_GD:
        motors[selectedMotor]->GD();
        break;
      case COMMAND_GM:
        motors[selectedMotor]->GM();
        break;
      case COMMAND_GI:
        motors[selectedMotor]->GI(selectedMotor);
        Serial.println(motors[selectedMotor]->getType());
        break;
      case COMMAND_SA:
        for (int i = 0; i < nMotors; i++)
          motors[i]->ST();
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
        motors[selectedMotor]->initSQ();
        break;
      }
      break;
    case 'R':
      switch (command[1])
      {
      case 'O':
        currentCommand = COMMAND_RO; //RO
        break;
      case 'W':
        currentCommand = COMMAND_RW; //RW
        break;
      case 'A':
        currentCommand = COMMAND_RA; //RA
        break;
      case 'R':
        currentCommand = COMMAND_RA; //RA
        break;
      case 'P':
        currentCommand = COMMAND_RP; //RP
        break;
      }
      break;
    case 'G':
      switch (command[1])
      {
      case 'S':
        currentCommand = COMMAND_GS; //GS
        break;
      case 'D':
        currentCommand = COMMAND_GD; //GD
        break;
      case 'M':
        currentCommand = COMMAND_GM; //GM
        break;
      case 'I':
        currentCommand = COMMAND_GI; //GI
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
      if (nMotors == 0)
      {
        nMotors = currentValue;
        iMotors = 0;
        currentValue = -1;
      }
      else
      {
        if (motorType == TYPE_UNKNOWN)
        {
          motorType = currentValue;
          currentValue = -1;
        }
        else
        {
          args[iMotorArg++] = currentValue;
          currentValue = -1;
          if (iMotorArg == 3)
          {
            switch (motorType)
            {
            case TYPE_STEPPER:
              motors[iMotors] = new Stepper(args[0], args[1], args[2]);
              break;
            case TYPE_SERVO:
              motors[iMotors] = new Servomotor(args[0], args[1], args[2]);
              break;
            case TYPE_VIBRO:
              motors[iMotors] = new Vibro(args[0]);
              break;
            }
            iMotors++;
            if (iMotors == nMotors)
            {
              Serial.println("");
              displaySelectedMotor();
              currentCommand = COMMAND_NONE;
              state = STATE_READY;
            }
            else
            {
              iMotorArg = 0;
              motorType = TYPE_UNKNOWN;
            }
          }
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
  nMotors = 0;
  state = STATE_SETUP;
  motorType = TYPE_UNKNOWN;
  nMotorArg = 0;
  iMotorArg = 0;
  selectedMotor = 0;
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
    // get serial commands
    if (Serial.available())
    {
      char a = Serial.read();
      processCommand(a);
    }
    for (int i = 0; i < nMotors; i++)
      motors[i]->action();
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
