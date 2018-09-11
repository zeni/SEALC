class Stepper : public Motor
{
  int pinSTP, pinDIR;
  int waveDir; // increasing / decreasing speed
  int turns;   // for rotate (0=continuous rotation)
  int realSteps;
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
      digitalWrite(pinDIR, currentDir);
    }
    if ((millis() - timeMS) > speed)
    {
      if (currentSteps >= steps)
      {
        currentDir = 1 - currentDir;
        digitalWrite(pinDIR, currentDir);
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