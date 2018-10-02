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

class Stepper
{
  int id;
  int nSteps;
  int mode;
  int steps;     // for move/hammer
  int stepsHome; // steps for homing
  int dir;
  int currentDir;
  int currentSteps;     // for move/hammer
  int indexSeq;         // current position in sequence
  int lengthSeq;        // length of seq.
  int currentIndexSeq;  // current position in sequence
  int currentLengthSeq; // length of seq.
  unsigned long timeMS; // for speed
  int speed;            // en ms
  int speedRPM;         //en RPM
  bool newBeat;
  int modesQ[MAX_QUEUE];
  int valuesQ[MAX_QUEUE];
  int sizeQ;
  int pause;
  bool isPaused;
  int pinSTP, pinDIR;
  int waveDir; // increasing / decreasing speed
  int turns;   // for rotate (0=continuous rotation)
  int realSteps;
  int seq[MAX_SEQ];        // seq. of angles for beat
  int currentSeq[MAX_SEQ]; // seq. of angles for beat
  int angleSeq;            // angle value for seq.
  void setWA(int v);
  void deQ();
  void RA();
  void SQ();
  void RW();
  void stepperStep();
  void moveStep();
  void SD();
  void setSD(int v);
  void setRA(int v);
  void setRR(int v);
  void setRW(int v);
  void WA();
  void setSQ(int v);

public:
  Stepper();
  Stepper(int n, int stp, int dir);
  void action();
  void SS(int v);
  void initSQ();
  void columnSQ(int v);
  void fillQ(int m, int v);
  void ST();
};

Stepper::Stepper()
{
}

Stepper::Stepper(int n, int pin_stp, int pin_dir)
{
  mode = MODE_IDLE;
  speedRPM = 12;
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
  currentIndexSeq = 0;
  currentLengthSeq = 0;
  pause = 1000;
  isPaused = false;
  waveDir = 0;
  pinSTP = pin_stp;
  pinDIR = pin_dir;
  pinMode(pinDIR, OUTPUT);
  pinMode(pinSTP, OUTPUT);
  nSteps = n;
  realSteps = currentSteps;
  for (int j = 0; j < MAX_SEQ; j++)
  {
    seq[j] = 0;
    currentSeq[j] = 0;
  }
  angleSeq = 0;
  speed = (speedRPM > 0) ? (floor(60.0 / (speedRPM * nSteps) * 1000)) : 0;
  timeMS = millis();
}

void Stepper::SS(int v)
{
  speedRPM = (v > 60000.0 / nSteps) ? floor(60000.0 / nSteps) : v;
  speed = (speedRPM > 0) ? (floor(60000.0 / (speedRPM * nSteps))) : 0;
  Serial.print(">> speed: ");
  Serial.print(speedRPM);
  Serial.println(" RPM");
}

void Stepper::setSD(int v)
{
  v = (v > 0) ? 1 : v;
  dir = (v < 0) ? (1 - dir) : v;
  mode = MODE_SD;
  Serial.print(">> dir: ");
  (dir > 0) ? Serial.println("CCW") : Serial.println("CW");
}

void Stepper::setWA(int v)
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

void Stepper::setRR(int v)
{
  setRA(v);
}

void Stepper::setRA(int v)
{
  v = (v <= 0) ? 0 : (v % 360);
  steps = v / 360.0 * nSteps;
  Serial.print(">> move ");
  Serial.print(steps * 360.0 / nSteps);
  Serial.println(" degrees");
  currentSteps = 0;
  mode = MODE_RA;
  timeMS = millis();
}

void Stepper::setRW(int v)
{
  v = (v <= 0) ? 1 : v;
  Serial.print(">> speed cycle: ");
  Serial.print(v);
  Serial.println(" per turn");
  steps = nSteps / (2.0 * v);
  currentSteps = 0;
  realSteps = currentSteps;
  waveDir = 0;
  mode = MODE_RW;
  timeMS = millis();
}

void Stepper::initSQ()
{
  indexSeq = 0;
  lengthSeq = 0;
}

void Stepper::columnSQ(int v)
{
  v = (v <= 0) ? 0 : v;
  if (angleSeq == 0)
    angleSeq = v;
  else
    seq[indexSeq++] = v;
}

void Stepper::setSQ(int v)
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
  angleSeq = angleSeq / 360.0 * nSteps;
  currentIndexSeq = 0;
  steps = angleSeq;
  angleSeq = 0;
  currentSteps = 0;
  mode = MODE_SQ;
  timeMS = millis();
}

// one step stepper
void Stepper::stepperStep()
{
  digitalWrite(pinSTP, LOW);
  digitalWrite(pinSTP, HIGH);
}

// move one step
void Stepper::moveStep()
{
  if (currentSteps >= steps)
    ST();
  else
  {
    currentSteps++;
    stepperStep();
    timeMS = millis();
  }
}

void Stepper::action()
{
  switch (mode)
  {
  case MODE_IDLE:
    deQ();
    break;
  case MODE_ST:
    ST();
    break;
  case MODE_SD:
    SD();
    break;
  case MODE_RA:
  case MODE_RR:
    RA();
    break;
  case MODE_RW:
    RW();
    break;
  case MODE_SQ:
    SQ();
    break;
  case MODE_WA:
    WA();
    break;
  }
}

void Stepper::ST()
{
  Serial.println(">> stop");
  currentSteps = 0;
  digitalWrite(pinSTP, LOW);
  mode = MODE_IDLE;
  deQ();
}

void Stepper::SD()
{
  currentDir = dir;
  digitalWrite(pinDIR, dir);
  deQ();
}

// rotate a number of steps
void Stepper::RA()
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

// continuous wave movement (like rotate but with changing speed)
void Stepper::RW()
{
  if (speed > 0)
  {
    int s = (waveDir == 0) ? (speed * (steps - currentSteps)) : (speed * currentSteps);
    if ((millis() - timeMS) > s)
    {
      if (currentSteps >= steps)
      {
        waveDir = 1 - waveDir;
        currentSteps = 0;
      }
      else
      {
        realSteps++;
        realSteps %= nSteps;
        if (realSteps == 0)
          deQ();
        currentSteps++;
        stepperStep();
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

void Stepper::WA()
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
  {
    isPaused = true;
    digitalWrite(pinSTP, LOW);
  }
}

// continuous hammer movement with pattern of angles
void Stepper::SQ()
{
  if (speed > 0)
  {
    if (newBeat)
    {
      newBeat = false;
      int a = floor(currentIndexSeq / 2);
      currentDir = (currentSeq[a] < 2) ? dir : (1 - dir);
      digitalWrite(pinDIR, currentDir);
      deQ();
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
        digitalWrite(pinDIR, currentDir);
      }
      else
      {
        currentSteps++;
        if (currentSeq[a] > 0)
          stepperStep();
        else
          digitalWrite(pinSTP, LOW);
      }
      timeMS = millis();
    }
  }
  else
  {
    ST();
    Serial.println("Stopped: speed is 0.");
  }
}

void Stepper::deQ()
{
  switch (modesQ[0])
  {
  case MODE_IDLE:
    break;
  case MODE_ST:
    mode = modesQ[0];
    break;
  case MODE_RA:
    setRA(valuesQ[0]);
    break;
  case MODE_RR:
    setRR(valuesQ[0]);
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

void Stepper::fillQ(int m, int v)
{
  if (sizeQ >= MAX_QUEUE)
  {
    sizeQ--;
    for (int i = 0; i < sizeQ; i++)
    {
      modesQ[i] = modesQ[i + 1];
      valuesQ[i] = valuesQ[i + 1];
    }
  }
  modesQ[sizeQ] = m;
  valuesQ[sizeQ++] = v;
}