#include <Servo.h>

class Servomotor : public Motor
{
  Servo servo;
  int angleMin, angleMax;
  int pin;
  int angle; // current angle
  void move();
  void beat();
  void servoStep();
  void moveStep();

public:
  Servomotor();
  Servomotor(int p, int amin, int amax);
  void setRotate(int v);
  String getType();
  void setDir(int v);
  void stop();
  void setMove(int v);
  void setWave(int v);
  void goHome();
  void action();
};

Servomotor::Servomotor() : Motor()
{
}

Servomotor::Servomotor(int p, int amin, int amax) : Motor()
{
  angleMin = amin;
  angleMax = amax;
  pin = p;
  nSteps = 360;
  servo.attach(pin);
  servo.write(angleMin);
}

String Servomotor::getType()
{
  return " (servo)";
}

void Servomotor::setDir(int v)
{
  if (v > 0)
    v = 1;
  switch (mode)
  {
  case MODE_STOP:
    if (v < 0)
      dir = 1 - dir;
    else
      dir = v;
    currentDir = dir;
    break;
  default:
    if (v < 0)
      dir = 1 - dir;
    else
      dir = v;
    break;
  }
  Serial.print(">> dir: ");
  if (dir > 0)
  {
    Serial.println("CCW");
  }
  else
    Serial.println("CW");
}

void Servomotor::setRotate(int v)
{
  Serial.println(">> servo has no rotate command");
}

void Servomotor::stop()
{
  Serial.println(">> stop");
  switch (mode)
  {
  case MODE_BEAT:
    if (currentDir == dir)
    {
      currentDir = 1 - dir;
    }
    currentSteps = steps - currentSteps;
    stepsHome = steps;
    mode = MODE_HOME;
    timeMS = millis();
    break;
  case MODE_MOVE:
    mode = MODE_HOME;
    stepsHome = steps;
    timeMS = millis();
    break;
  case MODE_ROTATE:
  case MODE_WAVE:
    mode = MODE_HOME;
    stepsHome = nSteps;
    timeMS = millis();
    break;
  default:
    mode = nextMode;
    nextMode = MODE_STOP;
    currentSteps = 0;
    angle = servo.read();
  }
}

void Servomotor::setMove(int v)
{
  if (v < 0)
    v = 0;
  v = v % 180;
  if (dir > 0)
    v = -v;
  Serial.print(">> move ");
  Serial.print(v);
  Serial.println(" degrees");
  steps = abs(v) / 360.0 * nSteps;
  currentSteps = 0;
  if (mode == MODE_STOP)
  {
    mode = MODE_MOVE;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_MOVE;
    stop();
  }
}

void Servomotor::setWave(int v)
{
  Serial.println(">> servo has no wave command");
}

// stop and back to init pos
void Servomotor::goHome()
{
  if ((millis() - timeMS) > speed)
  {
    if (currentSteps >= stepsHome)
    {
      currentSteps = 0;
      mode = MODE_STOP;
      stop();
    }
    else
    {
      currentSteps++;
      servoStep();
      timeMS = millis();
    }
  }
}

// one step servo
void Servomotor::servoStep()
{
  if (currentDir == 0)
  {
    angle += currentSteps;
    if (angle > angleMax)
    {
      currentSteps = steps;
      angle = angleMax;
    }
  }
  else
  {
    angle -= currentSteps;
    if (angle < angleMin)
    {
      currentSteps = steps;
      angle = angleMin;
    }
  }
  servo.write(angle);
}

// move one step
void Servomotor::moveStep()
{
  if (currentSteps >= steps)
  {
    mode = MODE_STOP;
    stop();
  }
  else
  {
    currentSteps++;
    servoStep();
    timeMS = millis();
  }
}

void Servomotor::action()
{
  switch (mode)
  {
  case MODE_STOP:
    break;
  case MODE_MOVE:
    move();
    break;
  case MODE_BEAT:
    beat();
    break;
  case MODE_HOME:
    goHome();
    break;
  }
}

// rotate a number of steps
void Servomotor::move()
{
  if (speed > 0)
  {
    if ((millis() - timeMS) > speed)
    {
      moveStep();
    }
  }
  else
  {
    stop();
    Serial.println("Stopped: speed is 0.");
  }
}

// continuous hammer movement with pattern of angles
void Servomotor::beat()
{
  if (speed > 0)
  {
    if (newBeat)
    {
      newBeat = false;
      int a = floor(indexSeq / 2);
      switch (seq[a])
      {
      case 2:
        currentDir = 1 - dir;
        break;
      case 1:
        currentDir = dir;
        break;
      }
    }
    if ((millis() - timeMS) > speed)
    {
      if (currentSteps >= steps)
      {
        currentDir = 1 - currentDir;
        currentSteps = 0;
        indexSeq++;
        if ((indexSeq % 2) == 0)
        {
          newBeat = true;
        }
        int a = floor(indexSeq / 2);
        if (a >= lengthSeq)
        {
          indexSeq = 0;
        }
      }
      else
      {
        int a = floor(indexSeq / 2);
        if (seq[a] > 0)
        {
          currentSteps++;
          servoStep();
        }
        timeMS = millis();
      }
    }
  }
  else
  {
    stop();
    Serial.println("Stopped: speed is 0.");
  }
}