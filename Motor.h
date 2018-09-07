#define MAX_SEQ 10 // max length of sequence for beat
//  modes
#define MODE_ST 0
#define MODE_RO 1
#define MODE_RA 2
#define MODE_RW 4
#define MODE_SQ 5
#define MODE_HOME 6
#define MODE_SD 7

class Motor
{
protected:
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
  int speed;            // en RPM
  bool newBeat;

  //common
  virtual void RA();
  virtual void SQ();
  virtual void goHome();
  virtual void moveStep();

public:
  // common
  Motor();
  void initSQ();
  void SS(int v);
  virtual void setRO(int v);
  virtual void setRA(int v);
  virtual void setRW(int v);
  void setSQ(int v);
  void columnSQ(int v);
  //void columnRP(int v);
  virtual void ST();
  virtual void action();
  virtual void SD(int v);
  virtual String getType();
  void setMode(int m);
  void setNextMode(int m);
};

Motor::Motor()
{
  mode = MODE_ST;
  nextMode = mode;
  speed = 12;
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

void Motor::SD(int v)
{
}

void Motor::setMode(int m)
{
  mode = m;
}

void Motor::setNextMode(int m)
{
  nextMode = m;
}

void Motor::SS(int v)
{
  if (v > 0)
    speed = floor(60.0 / (v * nSteps) * 1000);
  else
  {
    speed = 0;
    v = 0;
  }
  Serial.print(">> speed: ");
  Serial.print(v);
  Serial.println(" RPM");
}

void Motor::initSQ()
{
  angleSeq = 0;
  indexSeq = 0;
  lengthSeq = 0;
  newBeat = true;
}

void Motor::columnSQ(int v)
{
  if (v < 0)
    v = 0;
  if (angleSeq == 0)
    angleSeq = v;
  else
  {
    seq[indexSeq] = v;
    indexSeq++;
  }
}

void Motor::setRO(int v)
{
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

void Motor::setRW(int v)
{
}

void Motor::setSQ(int v)
{
  Serial.print(">> beat pattern: ");
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