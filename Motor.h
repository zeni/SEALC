#define MAX_SEQ 10 // max length of sequence for beat
//  modes
#define MODE_STOP 0
#define MODE_ROTATE 1
#define MODE_MOVE 2
#define MODE_WAVE 4
#define MODE_BEAT 5
#define MODE_HOME 6
#define MODE_DIR 7

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
  virtual void move();
  virtual void beat();
  virtual void goHome();
  virtual void moveStep();

public:
  // common
  Motor();
  void initBeat();
  void setSpeed(int v);
  virtual void setRotate(int v);
  virtual void setMove(int v);
  virtual void setWave(int v);
  void setBeat(int v);
  void setInterBeat(int v);
  virtual void stop();
  virtual void action();
  virtual void setDir(int v);
  virtual String getType();
  void setMode(int m);
  void setNextMode(int m);
};

Motor::Motor()
{
  mode = MODE_STOP;
  nextMode = mode;
  speed = 0;
  currentSteps = 0;
  steps = 0;
  stepsHome = steps;
  dir = 0;
  currentDir = dir;
  for (int j = 0; j < MAX_SEQ; j++)
  {
    seq[j] = 0;
  }
  indexSeq = 0;
  lengthSeq = 0;
  angleSeq = 0;
  timeMS = millis();
}

String Motor::getType()
{
  return "unknown";
}

void Motor::setDir(int v)
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

void Motor::setSpeed(int v)
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

void Motor::initBeat()
{
  angleSeq = 0;
  indexSeq = 0;
  lengthSeq = 0;
  newBeat = true;
}

void Motor::setInterBeat(int v)
{
  if (v < 0)
    v = 0;
  if (angleSeq == 0)
  {
    angleSeq = v;
  }
  else
  {
    seq[indexSeq] = v;
    indexSeq++;
  }
}

void Motor::setRotate(int v)
{
}

void Motor::stop()
{
}

// stop and back to init pos
void Motor::goHome()
{
}

void Motor::setMove(int v)
{
}

void Motor::setWave(int v)
{
}

void Motor::setBeat(int v)
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
  if (mode == MODE_STOP)
  {
    mode = MODE_BEAT;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_BEAT;
    stop();
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
void Motor::move()
{
}

// continuous hammer movement with pattern of angles
void Motor::beat()
{
}