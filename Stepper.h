class Stepper : public Motor
{
  int pinSTP, pinDIR;
  int waveDir; // increasing / decreasing speed
  int turns;   // for rotate (0=continuous rotation)
  int realSteps;
  int pause;     // for RP in ms
  bool isPaused; // for RP, status
  void RO();
  void RP();
  void RA();
  void SQ();
  void RW();
  void goHome();
  void stepperStep();
  void moveStep();
  void initRP();
  bool setRP(int v);
  void SD(int v);
  bool setRO(int v);
  void setRA(int v);
  void setRR(int v);
  bool setRW(int v);
  void columnRP(int v);
  void ST();
  void action();
  String getType();
  void deQ();
  void fillQ(int m, int v);

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
  pause = 1000;
  isPaused = false;
}

String Stepper::getType()
{
  return " (stepper)";
}

void Stepper::SD(int v)
{
  v = (v > 0) ? 1 : v;
  dir = (v < 0) ? (1 - dir) : v;
  currentDir = dir;
  digitalWrite(pinDIR, dir);
  Serial.print(">> dir: ");
  (dir > 0) ? Serial.println("CCW") : Serial.println("CW");
}

bool Stepper::setRO(int v)
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
  return true;
}

void Stepper::initRP()
{
  turns = 1;
  pause = 1000;
  isPaused = false;
}

bool Stepper::setRP(int v)
{
  pause = (v <= 0) ? 1000 : v;
  turns = (turns <= 0) ? 1 : turns;
  steps = turns * nSteps;
  Serial.print(">> rotate (with " + String(pause) + "ms pause) ");
  Serial.println(String(turns) + " turn(s)");
  mode = MODE_RP;
  timeMS = millis();
  return true;
}

void Stepper::columnRP(int v)
{
  turns = (v <= 0) ? 1 : v;
}

void Stepper::ST()
{
  Serial.println(">> stop");
  currentSteps = 0;
  digitalWrite(pinSTP, LOW);
  commandDone = true;
  mode = MODE_IDLE;
}

// stop and back to init pos
void Stepper::goHome()
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
      stepperStep();
      timeMS = millis();
    }
  }
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
    commandDone = true;
  }
  else
  {
    commandDone = false;
    currentSteps++;
    stepperStep();
    timeMS = millis();
  }
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

bool Stepper::setRW(int v)
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

void Stepper::deQ()
{
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
    setRP(valuesQ[0]);
    break;
  case MODE_SQ:
    setSQ(valuesQ[0]);
    break;
  case MODE_SD:
    SD(valuesQ[0]);
    break;
  }
  commandDone = false;
  for (int i = 1; i < sizeQ; i++)
  {
    modesQ[i - 1] = modesQ[i];
    valuesQ[i - 1] = valuesQ[i];
  }
  modesQ[MAX_QUEUE - 1] = MODE_IDLE;
  valuesQ[MAX_QUEUE - 1] = -1;
  sizeQ--;
  sizeQ = (sizeQ < 0) ? 0 : sizeQ;
}

void Stepper::fillQ(int m, int v)
{
  modesQ[sizeQ] = m;
  valuesQ[sizeQ] = v;
  sizeQ++;
  sizeQ = (sizeQ > MAX_QUEUE) ? MAX_QUEUE : sizeQ;
}

void Stepper::action()
{
  if (commandDone)
    deQ();
  switch (mode)
  {
  case MODE_IDLE:
  case MODE_SD:
    break;
  case MODE_ST:
    ST();
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
  case MODE_HOME:
    goHome();
    break;
  }
  return true;
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
        if (currentSteps == 0)
          commandDone = true;
        else
          commandDone = false;
        stepperStep();
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
  Serial.println(isPaused);
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
          commandDone = true;
          currentSteps = 0;
          digitalWrite(pinSTP, LOW);
        }
        else
        {
          commandDone = false;
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
          commandDone = true;
        else
          commandDone = false;
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

// continuous hammer movement with pattern of angles
void Stepper::SQ()
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