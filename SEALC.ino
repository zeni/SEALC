/****************** PLC = Physical Live Coding ********************
  Live coding on stepper motors and pots, piezo, hammer, tools,
  objects, etc. to create sounds.

  By 23N!
  Created: 2018/08/31
  Updated: 2018/09/05

  TODO:
  - Error messages
  - Deal with typing errors
  - better transition from a command to another one (wait to be stopped)
  - wave as a rotate option ?
  - new command: rotate x turns, pause, rotate x turns, ...
*******************************************************************/

#include "Motor.h"
#include "Stepper.h"
#include "Servomotor.h"

#define N_MOTORS 3
// pins
#define EN 8 // stepper motor enable, low level effective
// serial commands
#define SEPARATOR ','
#define EOL '\n'
#define COLUMN ':'
// commands
#define COMMAND_SELECT 0
#define COMMAND_SPEED 1
#define COMMAND_DIR 2
#define COMMAND_ROTATE 3
#define COMMAND_STOP 4
#define COMMAND_MOVE 5
#define COMMAND_NONE 6
#define COMMAND_WAVE 7
#define COMMAND_BEAT 8
#define COMMAND_ERROR 9

// vars
Motor *motors[N_MOTORS];
int selectedMotor;
int currentCommand, currentValue;
bool firstChar = true;
char command[2];
int iCommand;

void displaySelectedMotor()
{
  Serial.print(">> selected motor: ");
  Serial.print(selectedMotor);
  Serial.println(motors[selectedMotor]->getType());
}

void displayIntro()
{
  Serial.println("*************************************************");
  Serial.println("* SEALC: System for Electo-Acoustic Live Coding *");
  Serial.println("* by 23N! - 2018/08                             *");
  Serial.println("*************************************************");
}

void updateValue(char a)
{
  if (currentValue < 0)
    currentValue = 0;
  currentValue *= 10;
  currentValue += (a - 48);
}

void processCommand()
{
  if (iCommand == 1)
  {
    switch (command[0])
    {
    case COLUMN:
      if (firstChar)
      {
        currentCommand = COMMAND_NONE;
      }
      motors[selectedMotor]->setInterBeat(currentValue);
      currentValue = -1;
      iCommand = 0;
      command[0] = 0;
      break;
    case SEPARATOR:
    case EOL:
      if (firstChar)
      {
        currentCommand = COMMAND_NONE;
      }
      switch (currentCommand)
      {
      case COMMAND_SPEED:
        motors[selectedMotor]->setSpeed(currentValue);
        break;
      case COMMAND_DIR:
        motors[selectedMotor]->setDir(currentValue);
        break;
      case COMMAND_ROTATE:
        motors[selectedMotor]->setRotate(currentValue);
        break;
      case COMMAND_STOP:
        motors[selectedMotor]->setNextMode(MODE_STOP);
        motors[selectedMotor]->stop();
        break;
      case COMMAND_MOVE:
        motors[selectedMotor]->setMove(currentValue);
        break;
      case COMMAND_WAVE:
        motors[selectedMotor]->setWave(currentValue);
        break;
      case COMMAND_BEAT:
        motors[selectedMotor]->initBeat();
        motors[selectedMotor]->setBeat(currentValue);
        break;
      case COMMAND_SELECT:
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
    switch (command[0])
    {
    case 's':
    case 'S':
      switch (command[1])
      {
      case 's':
      case 'S':
        currentCommand = COMMAND_SPEED;
        break;
      case 'd':
      case 'D':
        currentCommand = COMMAND_DIR;
        break;
      case 't':
      case 'T':
        currentCommand = COMMAND_STOP;
        break;
      case 'q':
      case 'Q':
        currentCommand = COMMAND_BEAT;
        break;
      }
      break;
    case 'r':
    case 'R':
      switch (command[1])
      {
      case 'o':
      case 'O':
        currentCommand = COMMAND_ROTATE;
        break;
      case 'W':
      case 'w':
        currentCommand = COMMAND_WAVE;
        break;
      case 'a':
      case 'A':
        currentCommand = COMMAND_MOVE;
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

void setup()
{
  motors[0] = new Stepper(48, 2, 5);
  motors[1] = new Stepper(48, 3, 6);
  motors[2] = new Servomotor(9, 0, 180);
  selectedMotor = 0;
  Serial.begin(115200);
  displayIntro();
  displaySelectedMotor();
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
  // get serial commands
  if (Serial.available())
  {
    char a = Serial.read();
    if ((a >= 48) && (a < 58))
    {
      if (firstChar)
      {
        currentCommand = COMMAND_SELECT;
        selectedMotor = a - 48;
        if (selectedMotor >= N_MOTORS)
          selectedMotor = N_MOTORS - 1;
        displaySelectedMotor();
        firstChar = false;
      }
      else
      {
        updateValue(a);
      }
    }
    else
    {
      command[iCommand++] = a;
    }
    processCommand();
  }
  /*case COMMAND_BEAT:
        if (firstChar)
          firstChar = false;
        currentCommand = C_COMMAND_BEAT;
        motors[selectedMotor]->initBeat();
        break;
      case COLUMN:
        if (firstChar)
        {
          currentCommand = C_COMMAND_NONE;
        }
        motors[selectedMotor]->setInterBeat(currentValue);
        currentValue = -1;
        break;
      // execute commands  / set mode
      case SEPARATOR:
      case EOL:
        if (firstChar)
        {
          currentCommand = C_COMMAND_NONE;
        }
        switch (currentCommand)
        {
        case C_COMMAND_SPEED:
          motors[selectedMotor]->setSpeed(currentValue);
          break;
        case C_COMMAND_DIR:
          motors[selectedMotor]->setDir(currentValue);
          break;
        case C_COMMAND_ROTATE:
          motors[selectedMotor]->setRotate(currentValue);
          break;
        case C_COMMAND_STOP:
          motors[selectedMotor]->setNextMode(MODE_STOP);
          motors[selectedMotor]->stop();
          break;
        case C_COMMAND_MOVE:
          motors[selectedMotor]->setMove(currentValue);
          break;
        case C_COMMAND_WAVE:
          motors[selectedMotor]->setWave(currentValue);
          break;
        case C_COMMAND_BEAT:
          motors[selectedMotor]->setBeat(currentValue);
          break;
        case C_COMMAND_SELECT:
        case C_COMMAND_NONE:
          break;
        }
        currentValue = -1;
        firstChar = true;
        break;
      }
    }*/
  for (int i = 0; i < N_MOTORS; i++)
  {
    motors[i]->action();
  }
}
