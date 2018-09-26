#include <Servo.h>

class Servomotor : public Motor
{
  Servo servo;
  int angleMin, angleMax;
  int pin;
  int angle;               // current angle
  int seq[MAX_SEQ];        // seq. of angles for beat
  int currentSeq[MAX_SEQ]; // seq. of angles for beat
  int angleSeq;            // angle value for seq.
  void RA();
  void SQ();
  void SS(int v);
  void servoStep();
  void moveStep();
  void setRO(int v);
  String getType();
  void setSD(int v);
  void SD();
  void ST();
  void setRA(int v);
  void setRR(int v);
  void setRW(int v);
  void setRP(int v);
  void WA();
  void action();
  void columnRP(int v);
  void initSQ();
  void setSQ(int v);
  void columnSQ(int v);

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
  for (int j = 0; j < MAX_SEQ; j++)
  {
    seq[j] = 0;
    currentSeq[j] = 0;
  }
  angleSeq = 0;
  speed = (speedRPM > 0) ? (floor(60.0 / (speedRPM * nSteps) * 1000)) : 0;
}

String Servomotor::getType()
{
  return " (servo)";
}

void Servomotor::SS(int v)
{
  speedRPM = (v > 60000.0 / nSteps) ? floor(60000.0 / nSteps) : v;
  speed = (speedRPM > 0) ? (floor(60000.0 / (speedRPM * nSteps))) : 0;
  Serial.print(">> speed: ");
  Serial.print(speedRPM);
  Serial.println(" RPM");
}

void Servomotor::setSD(int v)
{
  v = (v > 0) ? 1 : v;
  dir = (v < 0) ? (1 - dir) : v;
  mode = MODE_SD;
  Serial.print(">> dir: ");
  (dir > 0) ? Serial.println("CCW") : Serial.println("CW");
}

void Servomotor::setRO(int v)
{
  Serial.println(">> servo has no RO command");
  mode = MODE_RO;
}

void Servomotor::columnRP(int v)
{
}

void Servomotor::setRP(int v)
{
  Serial.println(">> servo has no RP command");
  mode = MODE_RP;
}

void Servomotor::setRR(int v)
{
  v = (v <= 0) ? 0 : (v % (angleMax - angleMin));
  v = (dir > 0) ? -v : v;
  Serial.print(">> move ");
  Serial.print(v);
  Serial.println(" degrees");
  steps = abs(v);
  currentSteps = 0;
  currentDir = dir;
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
    v -= angle;
    currentDir = 0;
  }
  else
  {
    v = angle - v;
    currentDir = 1;
  }
  steps = v;
  currentSteps = 0;
  mode = MODE_RA;
  timeMS = millis();
}

void Servomotor::setRW(int v)
{
  Serial.println(">> servo has no RW command");
  mode = MODE_RW;
}

void Servomotor::columnSQ(int v)
{
  v = (v <= 0) ? 0 : v;
  if (angleSeq == 0)
    angleSeq = v;
  else
    seq[indexSeq++] = v;
}

void Servomotor::setSQ(int v)
{
  Serial.print(">> sequence: ");
  newBeat = true;
  currentDir = dir;
  if (angleSeq == 0)
  {
    angleSeq = v;
    indexSeq = 0;
    seq[indexSeq] = 1;
    lengthSeq = 1;
  }
  else
  {
    seq[indexSeq++] = v;
    lengthSeq = indexSeq;
  }
  currentLengthSeq = lengthSeq;
  for (int i = 0; i < currentLengthSeq; i++)
  {
    currentSeq[i] = seq[i];
    Serial.print(currentSeq[i]);
    Serial.print("|");
  }
  Serial.println();
  indexSeq = 0;
  currentIndexSeq = 0;
  steps = angleSeq;
  angleSeq = 0;
  currentSteps = 0;
  mode = MODE_SQ;
  timeMS = millis();
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
  case MODE_IDLE:
    deQ();
    break;
  case MODE_SD:
    SD();
    break;
  case MODE_RO:
  case MODE_RP:
  case MODE_RW:
  case MODE_ST:
    ST();
    break;
  case MODE_RA:
  case MODE_RR:
    RA();
    break;
  case MODE_SQ:
    SQ();
    break;
  case MODE_WA:
    WA();
    break;
  }
}

void Servomotor::ST()
{
  Serial.println(">> stop");
  currentSteps = 0;
  angle = servo.read();
  mode = MODE_IDLE;
  deQ();
}

void Servomotor::SD()
{
  currentDir = dir;
  deQ();
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
      int a = floor(currentIndexSeq / 2);
      currentDir = (currentSeq[a] < 2) ? dir : (1 - dir);
    }
    if ((millis() - timeMS) > speed)
    {
      int a = floor(currentIndexSeq / 2);
      if (currentSteps >= steps)
      {
        currentSteps = 0;
        currentIndexSeq++;
        if (a >= currentLengthSeq)
          currentIndexSeq = 0;
        if ((currentIndexSeq % 2) == 0)
          newBeat = true;
        else
          currentDir = 1 - currentDir;
      }
      else
      {
        currentSteps++;
        if (currentSeq[a] > 0)
          servoStep();
      }
    }
    timeMS = millis();
  }
  else
  {
    ST();
    Serial.println("Stopped: speed is 0.");
  }
}

void Servomotor::WA()
{
  if (isPaused)
  {
    if ((millis() - timeMS) > pause)
    {
      isPaused = false;
      ST();
    }
  }
  else
    isPaused = true;
}