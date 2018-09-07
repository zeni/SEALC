#include <Servo.h>

class Servomotor : public Motor
{
  Servo servo;
  int angleMin, angleMax;
  int pin;
  int angle; // current angle
  void RA();
  void SQ();
  void servoStep();
  void moveStep();
  void initRP();
  bool setRO(int v);
  String getType();
  void SD(int v);
  void ST();
  void setRA(int v);
  bool setRW(int v);
  bool setRP(int v);
  void goHome();
  void action();
  void columnRP(int v);

public:
  Servomotor();
  Servomotor(int p, int amin, int amax);
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
  angle = servo.read();
}

String Servomotor::getType()
{
  return " (servo)";
}

void Servomotor::SD(int v)
{
  v = (v > 0) ? 1 : 0;
  dir = (v < 0) ? (1 - dir) : v;
  if (mode == MODE_ST)
    currentDir = dir;
  Serial.print(">> dir: ");
  (dir > 0) ? Serial.println("CCW") : Serial.println("CW");
}

bool Servomotor::setRO(int v)
{
  Serial.println(">> servo has no RO command");
  return false;
}

bool Servomotor::setRP(int v)
{
  Serial.println(">> servo has no RP command");
  return false;
}

void Servomotor::initRP()
{
}

void Servomotor::columnRP(int v)
{
}

void Servomotor::ST()
{
  Serial.println(">> stop");
  switch (mode)
  {
  case MODE_SQ:
    if (currentDir == dir)
      currentDir = 1 - dir;
    currentSteps = steps - currentSteps;
    stepsHome = steps;
    mode = MODE_HOME;
    timeMS = millis();
    break;
  case MODE_RA:
    mode = MODE_HOME;
    stepsHome = steps;
    timeMS = millis();
    break;
  default:
    mode = nextMode;
    nextMode = MODE_ST;
    currentSteps = 0;
    angle = servo.read();
  }
}

void Servomotor::setRA(int v)
{
  v = (v <= 0) ? 0 : (v % (angleMax - angleMin));
  v = (dir > 0) ? -v : v;
  Serial.print(">> move ");
  Serial.print(v);
  Serial.println(" degrees");
  steps = abs(v) / 360.0 * nSteps;
  currentSteps = 0;
  if (mode == MODE_ST)
  {
    mode = MODE_RA;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_RA;
    ST();
  }
}

bool Servomotor::setRW(int v)
{
  Serial.println(">> servo has no RW command");
  return false;
}

// stop and back to init pos
void Servomotor::goHome()
{
  if ((millis() - timeMS) > speed)
  {
    if (currentSteps >= stepsHome)
    {
      currentSteps = 0;
      mode = MODE_ST;
      ST();
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
  Serial.println(angle);
  Serial.println(currentSteps);
  if (currentDir == 0)
  {
    angle++;
    if (angle > angleMax)
    {
      currentSteps = steps;
      angle = angleMax;
    }
  }
  else
  {
    angle--;
    if (angle < angleMin)
    {
      currentSteps = steps;
      angle = angleMin;
    }
  }
  Serial.println(angle);
  servo.write(angle);
}

// move one step
void Servomotor::moveStep()
{
  if (currentSteps >= steps)
  {
    mode = MODE_ST;
    ST();
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
  case MODE_ST:
    break;
  case MODE_RA:
    RA();
    break;
  case MODE_SQ:
    SQ();
    break;
  case MODE_HOME:
    goHome();
    break;
  }
}

// rotate a number of steps
void Servomotor::RA()
{
  if (speed > 0)
  {
    if ((millis() - timeMS) > speed)
      moveStep();
  }
  else
  {
    ST();
    Serial.println("Stopped: speed is 0.");
  }
}

// continuous hammer movement with pattern of angles
void Servomotor::SQ()
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
          newBeat = true;
        int a = floor(indexSeq / 2);
        if (a >= lengthSeq)
          indexSeq = 0;
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
    ST();
    Serial.println("Stopped: speed is 0.");
  }
}