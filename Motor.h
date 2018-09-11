#define MAX_SEQ 10 // max length of sequence for beat
#define MAX_QUEUE 10
//  modes
#define MODE_ST 0
#define MODE_RO 1
#define MODE_RA 2
#define MODE_RR 3
#define MODE_RW 4
#define MODE_SQ 5
#define MODE_WA 6
#define MODE_SD 7
#define MODE_RP 8
#define MODE_IDLE 9

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
  int indexSeq;         // current position in sequence
  int lengthSeq;        // length of seq.
  unsigned long timeMS; // for speed
  int speed;            // en ms
  int speedRPM;         //en RPM
  bool newBeat;
  int modesQ[MAX_QUEUE];
  int valuesQ[MAX_QUEUE];
  int sizeQ;
  int pause;
  bool isPaused;

public:
  Motor();
  virtual void initRP();
  void SS(int v);
  virtual void setRO(int v);
  virtual void setRP(int v);
  virtual void setRA(int v);
  virtual void setRR(int v);
  virtual void setRW(int v);
  void setWA(int v);
  virtual void VA();
  virtual void initSQ();
  virtual void setSQ(int v);
  virtual void columnSQ(int v);
  virtual void columnRP(int v);
  virtual void ST();
  virtual void action();
  virtual void setSD(int v);
  virtual String getType();
  void setNextMode(int m);
  void GS();
  void GD();
  void GM();
  void GI(int i);
  void fillQ(int m, int v);
  void deQ();
  void initWA();
};

Motor::Motor()
{
  mode = MODE_IDLE;
  nextMode = mode;
  speedRPM = 12;
  SS(speedRPM);
  currentSteps = 0;
  steps = 0;
  stepsHome = steps;
  dir = 0;
  currentDir = dir;
  for (int j = 0; j < MAX_QUEUE; j++)
    modesQ[j] = MODE_IDLE;
  sizeQ = 0;
  indexSeq = 0;
  lengthSeq = 0;
  timeMS = millis();
  pause = 1000;
  isPaused = false;
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
  case MODE_RR:
    Serial.println("RR");
    break;
  case MODE_WA:
    Serial.println("WA");
    break;
  case MODE_RW:
    Serial.println("RW");
    break;
  case MODE_RP:
    Serial.println("RP");
    break;
  case MODE_SQ:
    Serial.println("SQ");
    break;
  case MODE_SD:
    Serial.println("SD");
    break;
  case MODE_IDLE:
    Serial.println("IDLE");
    break;
  }
}

void Motor::setSD(int v)
{
}

void Motor::initWA()
{
  pause = 1000;
  isPaused = false;
}

void Motor::setWA(int v)
{
  isPaused = false;
  v = (v < 0) ? 1000 : v;
  pause = v;
  Serial.print(">> wait ");
  Serial.print(pause);
  Serial.println(" ms");
  pause = v;
  mode = MODE_WA;
  timeMS = millis();
}

void Motor::setRA(int v)
{
}

void Motor::setRO(int v)
{
}

void Motor::setRR(int v)
{
}

void Motor::setRW(int v)
{
}

void Motor::SS(int v)
{
  speedRPM = (v > 0) ? v : 0;
  speed = (speedRPM > 0) ? (floor(60.0 / (speedRPM * nSteps) * 1000)) : 0;
  Serial.print(">> speed: ");
  Serial.print(speedRPM);
  Serial.println(" RPM");
}

void Motor::ST()
{
}

void Motor::initSQ()
{
}

void Motor::columnSQ(int v)
{
}

void Motor::setSQ(int v)
{
}

void Motor::initRP()
{
}

void Motor::columnRP(int v)
{
}

void Motor::setRP(int v)
{
}

void Motor::VA()
{
}

void Motor::action()
{
}

void Motor::deQ()
{
  //Serial.println(modesQ[0]);
  switch (modesQ[0])
  {
  case MODE_IDLE:
    break;
  case MODE_ST:
    mode = modesQ[0];
    break;
  case MODE_RO:
    setRO(valuesQ[0]);
    break;
  case MODE_RP:
    setRP(valuesQ[0]);
    break;
  case MODE_RA:
  case MODE_RR:
    setRA(valuesQ[0]);
    break;
  case MODE_RW:
    setRW(valuesQ[0]);
    break;
  case MODE_SQ:
    setSQ(valuesQ[0]);
    break;
  case MODE_SD:
    setSD(valuesQ[0]);
    break;
  case MODE_WA:
    setWA(valuesQ[0]);
    break;
  }
  if (modesQ[0] != MODE_IDLE)
  {
    for (int i = 1; i < MAX_QUEUE; i++)
    {
      modesQ[i - 1] = modesQ[i];
      valuesQ[i - 1] = valuesQ[i];
    }
    modesQ[MAX_QUEUE - 1] = MODE_IDLE;
    valuesQ[MAX_QUEUE - 1] = -1;
    sizeQ--;
    sizeQ = (sizeQ < 0) ? 0 : sizeQ;
  }
}

void Motor::fillQ(int m, int v)
{
  modesQ[sizeQ] = m;
  valuesQ[sizeQ] = v;
  sizeQ++;
  sizeQ = (sizeQ > MAX_QUEUE) ? MAX_QUEUE : sizeQ;
}