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
  void setRO(int v);
  String getType();
  void setSD(int v);
  void SD();
  void ST();
  void setRA(int v);
  void setRR(int v);
  void setRW(int v);
  void setRP(int v);
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

void Servomotor::setSD(int v)
{
  v = (v > 0) ? 1 : v;
  dir = (v < 0) ? (1 - dir) : v;
  mode = MODE_SD;
  Serial.print(">> dir: ");
  (dir > 0) ? Serial.println("CCW") : Serial.println("CW");
}

void Servomotor::SD()
{
  currentDir = dir;
  deQ();
}

void Servomotor::setRO(int v)
{
  Serial.println(">> servo has no RO command");
  mode = MODE_RO;
}

void Servomotor::setRP(int v)
{
  Serial.println(">> servo has no RP command");
  mode = MODE_RP;
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
  currentSteps = 0;
  angle = servo.read();
  mode = MODE_ST;
  deQ();
}

void Servomotor::setRR(int v)
{
  v = (v <= 0) ? 0 : (v % (angleMax - angleMin));
  v = (dir > 0) ? -v : v;
  Serial.print(">> move ");
  Serial.print(v);
  Serial.println(" degrees");
  steps = abs(v) / 360.0 * nSteps;
  currentSteps = 0;
  mode = MODE_RR;
  timeMS = millis();
}

void Servomotor::setRA(int v)
{
  v = (v < angleMin) ? angleMin : ((v > angleMax) ? angleMax : v);
  Serial.print(">> move to ");
  Serial.print(v);
  Serial.println(" degrees");
  if (v > angle)
  {
    v = v - angle;
    currentDir = 1;
  }
  else
  {
    v = angle - v;
    currentDir = 0;
  }
  steps = v / 360.0 * nSteps;
  currentSteps = 0;
  mode = MODE_RA;
  timeMS = millis();
}

void Servomotor::setRW(int v)
{
  Serial.println(">> servo has no RW command");
  mode = MODE_RW;
}

// one step servo
void Servomotor::servoStep()
{
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
  servo.write(angle);
}

// move one step
void Servomotor::moveStep()
{
  if (currentSteps >= steps)
  {
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
  case MODE_RO:
  case MODE_RP:
  case MODE_RW:
    ST();
    break;
  case MODE_IDLE:
  case MODE_ST:
    break;
  case MODE_RA:
  case MODE_RR:
    RA();
    break;
  case MODE_SQ:
    SQ();
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
      deQ();
      newBeat = false;
      int a = floor(indexSeq / 2);
      switch (seq[a])
      {
      case 2:
        currentDir = 1 - dir;
        break;
      case 1:
      case 0:
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
        currentSteps++;
        if (seq[a] > 0)
        {
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