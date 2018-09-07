#define MAX_SEQ 10 // max length of sequence for beat
//  modes
#define MODE_ST 0
#define MODE_RO 1
#define MODE_RA 2
#define MODE_RW 4
#define MODE_SQ 5
#define MODE_HOME 6
#define MODE_SD 7
#define MODE_RP 8

class Motor
{
protected:
  int id;
  int nSteps;
  int mode;
  int nextMode;
  int steps;     // for move/hammer
  int stepsHome; // steps for homing
  int dir;
  int currentDir;
  int currentSteps;     // for move/hammer
  int seq[MAX_SEQ];     // seq. of angles for beat
  int indexSeq;         // current position in sequence
  int lengthSeq;        // length of seq.
  int angleSeq;         // angle value for seq.
  unsigned long timeMS; // for speed
  int speed;            // en ms
  int speedRPM;         //en RPM
  bool newBeat;
  //common
  virtual void RA();
  virtual void SQ();
  virtual void goHome();
  virtual void moveStep();

public:
  Motor();
  void initSQ();
  virtual void initRP();
  void SS(int v);
  virtual bool setRO(int v);
  virtual bool setRP(int v);
  virtual void setRA(int v);
  virtual bool setRW(int v);
  void setSQ(int v);
  void columnSQ(int v);
  virtual void columnRP(int v);
  virtual void ST();
  virtual void action();
  virtual void SD(int v);
  virtual String getType();
  void setNextMode(int m);
  void GS();
  void GD();
  void GM();
  void GI(int i);
};

Motor::Motor()
{
  mode = MODE_ST;
  nextMode = mode;
  speedRPM = 12;
  SS(speedRPM);
  currentSteps = 0;
  steps = 0;
  stepsHome = steps;
  dir = 0;
  currentDir = dir;
  for (int j = 0; j < MAX_SEQ; j++)
    seq[j] = 0;
  indexSeq = 0;
  lengthSeq = 0;
  angleSeq = 0;
  timeMS = millis();
}

String Motor::getType()
{
  return "unknown";
}

void Motor::GS()
{
  Serial.print(">> speed: ");
  Serial.print(speedRPM);
  Serial.println(" RPM");
}

void Motor::GI(int i)
{
  Serial.print(">> motor: " + String(i));
}

void Motor::GD()
{
  Serial.print(">> dir: ");
  (dir > 0) ? Serial.println("CCW") : Serial.println("CW");
}

void Motor::GM()
{
  Serial.print(">> mode: ");
  switch (mode)
  {
  case MODE_ST:
    Serial.println("ST");
    break;
  case MODE_RO:
    Serial.println("RO");
    break;
  case MODE_RA:
    Serial.println("RA");
    break;
  case MODE_RW:
    Serial.println("RW");
    break;
  case MODE_RP:
    Serial.println("RP");
    break;
  case MODE_HOME:
    Serial.println("HOME");
    break;
  case MODE_SQ:
    Serial.println("SQ");
    break;
  case MODE_SD:
    Serial.println("SD");
    break;
  }
}

void Motor::SD(int v)
{
}

void Motor::setNextMode(int m)
{
  nextMode = m;
}

void Motor::SS(int v)
{
  speedRPM = (v > 0) ? v : 0;
  speed = (speedRPM > 0) ? (floor(60.0 / (speedRPM * nSteps) * 1000)) : 0;
  Serial.print(">> speed: ");
  Serial.print(speedRPM);
  Serial.println(" RPM");
}

void Motor::initSQ()
{
  angleSeq = 0;
  indexSeq = 0;
  lengthSeq = 0;
  newBeat = true;
}

void Motor::initRP()
{
}

void Motor::columnSQ(int v)
{
  v = (v <= 0) ? 0 : v;
  if (angleSeq == 0)
    angleSeq = v;
  else
  {
    seq[indexSeq] = v;
    indexSeq++;
  }
}

void Motor::columnRP(int v)
{
}

bool Motor::setRP(int v)
{
  return false;
}

bool Motor::setRO(int v)
{
  return false;
}

void Motor::ST()
{
}

// stop and back to init pos
void Motor::goHome()
{
}

void Motor::setRA(int v)
{
}

bool Motor::setRW(int v)
{
  return false;
}

void Motor::setSQ(int v)
{
  Serial.print(">> sequence: ");
  currentDir = dir;
  if (angleSeq == 0)
  {
    angleSeq = v;
    seq[indexSeq] = 1;
    lengthSeq = 1;
  }
  else
  {
    seq[indexSeq] = v;
    indexSeq++;
    lengthSeq = indexSeq;
  }
  angleSeq = angleSeq / 360.0 * nSteps;
  indexSeq = 0;
  steps = angleSeq;
  angleSeq = 0;
  currentSteps = 0;
  for (int i = 0; i < lengthSeq; i++)
  {
    Serial.print(seq[i]);
    Serial.print("|");
  }
  Serial.println();
  if (mode == MODE_ST)
  {
    mode = MODE_SQ;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_SQ;
    ST();
  }
}

void Motor::action()
{
}

// move one step
void Motor::moveStep()
{
}

// rotate a number of steps
void Motor::RA()
{
}

// continuous hammer movement with pattern of angles
void Motor::SQ()
{
}