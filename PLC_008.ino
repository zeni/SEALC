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
*******************************************************************/

#include "Motor.h"
#include "Stepper.h"
#include "Servomotor.h"

#define N_MOTORS 3
// pins
#define EN 8 // stepper motor enable, low level effective
// serial commands
#define COMMAND_SPEED 's'
#define COMMAND_DIR 'd'
#define COMMAND_ROTATE 'r'
#define COMMAND_STOP 'x'
#define COMMAND_MOVE 'm'
#define COMMAND_WAVE 'w'
#define COMMAND_BEAT 'b'
#define SEPARATOR ','
#define EOL '\n'
#define COLUMN ':'
// commands
#define C_COMMAND_SELECT 0
#define C_COMMAND_SPEED 1
#define C_COMMAND_DIR 2
#define C_COMMAND_ROTATE 3
#define C_COMMAND_STOP 4
#define C_COMMAND_MOVE 5
#define C_COMMAND_NONE 6
#define C_COMMAND_WAVE 7
#define C_COMMAND_BEAT 8

// vars
Motor *motors[N_MOTORS];
int selectedMotor;
int currentCommand, currentValue;
bool firstChar = true;

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
  currentValue = 0;
  currentCommand = C_COMMAND_NONE;
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
        currentCommand = C_COMMAND_SELECT;
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
      switch (a)
      {
      case COMMAND_ROTATE:
        if (firstChar)
          firstChar = false;
        currentCommand = C_COMMAND_ROTATE;
        break;
      case COMMAND_STOP:
        if (firstChar)
          firstChar = false;
        currentCommand = C_COMMAND_STOP;
        break;
      case COMMAND_SPEED:
        if (firstChar)
          firstChar = false;
        currentCommand = C_COMMAND_SPEED;
        break;
      case COMMAND_DIR:
        if (firstChar)
          firstChar = false;
        currentCommand = C_COMMAND_DIR;
        break;
      case COMMAND_MOVE:
        if (firstChar)
          firstChar = false;
        currentCommand = C_COMMAND_MOVE;
        break;
      case COMMAND_WAVE:
        if (firstChar)
          firstChar = false;
        currentCommand = C_COMMAND_WAVE;
        break;
      case COMMAND_BEAT:
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
    }
  }
  for (int i = 0; i < N_MOTORS; i++)
  {
    motors[i]->action();
  }
}
