class Stepper : public Motor
{
  int pinSTP, pinDIR;
  int waveDir; // increasing / decreasing speed
  int turns;   // for rotate (0=continuous rotation)
  int realSteps;
  int seq[MAX_SEQ];        // seq. of angles for beat
  int currentSeq[MAX_SEQ]; // seq. of angles for beat
  int angleSeq;            // angle value for seq.
  void RO();
  void RP();
  void RA();
  void SQ();
  void RW();
  void stepperStep();
  void moveStep();
  void initRP();
  void setRP(int v);
  void SD();
  void setSD(int v);
  void setRO(int v);
  void setRA(int v);
  void setRR(int v);
  void setRW(int v);
  void columnRP(int v);
  void ST();
  void WA();
  void action();
  String getType();
  void initSQ();
  void setSQ(int v);
  void columnSQ(int v);

public:
  Stepper();
  Stepper(int n, int stp, int dir);
};

Stepper::Stepper() : Motor()
{
}

Stepper::Stepper(int n, int pin_stp, int pin_dir) : Motor()
{
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
}

String Stepper::getType()
{
  return " (stepper)";
}

void Stepper::setSD(int v)
{
  v = (v > 0) ? 1 : v;
  dir = (v < 0) ? (1 - dir) : v;
  mode = MODE_SD;
  Serial.print(">> dir: ");
  (dir > 0) ? Serial.println("CCW") : Serial.println("CW");
}

void Stepper::setRO(int v)
{
  Serial.print(">> rotate ");
  if (v <= 0)
  {
    turns = 0;
    Serial.println("continuously");
  }
  else
  {
    turns = v;
    Serial.println(String(turns) + " turn(s)");
  }
  steps = turns * nSteps;
  mode = MODE_RO;
  timeMS = millis();
}

void Stepper::initRP()
{
  turns = 1;
  pause = 1000;
  isPaused = false;
}

void Stepper::columnRP(int v)
{
  turns = (v <= 0) ? 1 : v;
}

void Stepper::setRP(int v)
{
  isPaused = false;
  pause = (v <= 0) ? 1000 : v;
  turns = (turns <= 0) ? 1 : turns;
  steps = turns * nSteps;
  Serial.print(">> rotate (with " + String(pause) + "ms pause) ");
  Serial.println(String(turns) + " turn(s)");
  mode = MODE_RP;
  timeMS = millis();
}

void Stepper::setRR(int v)
{
  setRA(v);
}

void Stepper::setRA(int v)
{
  v = (v <= 0) ? 0 : (v % 360);
  Serial.print(">> move ");
  Serial.print(v);
  Serial.println(" degrees");
  steps = v / 360.0 * nSteps;
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
  angleSeq = 0;
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
    currentSeq[i] = seq[i];
  angleSeq = angleSeq / 360.0 * nSteps;
  currentIndexSeq = 0;
  steps = angleSeq;
  angleSeq = 0;
  currentSteps = 0;
  for (int i = 0; i < currentLengthSeq; i++)
  {
    Serial.print(currentSeq[i]);
    Serial.print("|");
  }
  Serial.println();
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
  {
    ST();
  }
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
  case MODE_RO:
    RO();
    break;
  case MODE_RP:
    RP();
    break;
  case MODE_RA:
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

// rotation
void Stepper::RO()
{
  if (speed > 0)
  {
    if ((millis() - timeMS) > speed)
    {
      if (turns == 0)
      {
        currentSteps++;
        currentSteps %= nSteps;
        stepperStep();
        if (currentSteps == 0)
          deQ();
        timeMS = millis();
      }
      else
        moveStep();
    }
  }
  else
  {
    ST();
    Serial.println("Stopped: speed is 0.");
  }
}

// rotation with pause
void Stepper::RP()
{
  if (speed > 0)
  {
    if (isPaused)
    {
      if ((millis() - timeMS) > pause)
      {
        isPaused = false;
        currentSteps = 0;
      }
    }
    else
    {
      if ((millis() - timeMS) > speed)
      {
        if (currentSteps >= steps)
        {
          isPaused = true;
          currentSteps = 0;
          digitalWrite(pinSTP, LOW);
          deQ();
        }
        else
        {
          currentSteps++;
          stepperStep();
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

// rotate a number of steps
void Stepper::RA()
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
      deQ();
      newBeat = false;
      int a = floor(currentIndexSeq / 2);
      switch (currentSeq[a])
      {
      case 2:
        currentDir = 1 - dir;
        break;
      case 1:
      case 0:
        currentDir = dir;
        break;
      }
      digitalWrite(pinDIR, currentDir);
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