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
#define COMMAND_ROTATEPAUSE 10

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

void processCommand(char a)
{
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
      currentCommand = COMMAND_NONE;
      updateValue(a);
    }
  }
  else
  {
    command[iCommand++] = a;
  }
  if (iCommand == 1)
  {
    switch (command[0])
    {
    case COLUMN:
      if (firstChar)
      {
        currentCommand = COMMAND_ERROR;
      }
      else
      {
        switch (currentCommand)
        {
        case COMMAND_BEAT:
          motors[selectedMotor]->columnSQ(currentValue);
          break;
        case COMMAND_ROTATEPAUSE:
          //motors[selectedMotor]->columnRP(currentValue);
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
      {
        currentCommand = COMMAND_NONE;
      }
      switch (currentCommand)
      {
      case COMMAND_SPEED:
        motors[selectedMotor]->SS(currentValue);
        break;
      case COMMAND_DIR:
        motors[selectedMotor]->SD(currentValue);
        break;
      case COMMAND_ROTATE:
        motors[selectedMotor]->setRO(currentValue);
        break;
      case COMMAND_STOP:
        motors[selectedMotor]->setNextMode(MODE_ST);
        motors[selectedMotor]->ST();
        break;
      case COMMAND_MOVE:
        motors[selectedMotor]->setRA(currentValue);
        break;
      case COMMAND_WAVE:
        motors[selectedMotor]->setRW(currentValue);
        break;
      case COMMAND_BEAT:
        motors[selectedMotor]->setSQ(currentValue);
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
    case 's':
    case 'S':
      switch (command[1])
      {
      case 's':
      case 'S':
        currentCommand = COMMAND_SPEED; //SS
        break;
      case 'd':
      case 'D':
        currentCommand = COMMAND_DIR; //SD
        break;
      case 't':
      case 'T':
        currentCommand = COMMAND_STOP; //ST
        break;
      case 'q':
      case 'Q':
        currentCommand = COMMAND_BEAT; //SQ
        motors[selectedMotor]->initBeat();
        break;
      }
      break;
    case 'r':
    case 'R':
      switch (command[1])
      {
      case 'o':
      case 'O':
        currentCommand = COMMAND_ROTATE; //RO
        break;
      case 'W':
      case 'w':
        currentCommand = COMMAND_WAVE; //RW
        break;
      case 'a':
      case 'A':
        currentCommand = COMMAND_MOVE; //RA
        break;
      case 'p':
      case 'P':
        currentCommand = COMMAND_ROTATEPAUSE; //RP
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
    processCommand(a);
  }
  for (int i = 0; i < N_MOTORS; i++)
  {
    motors[i]->action();
  }
}
