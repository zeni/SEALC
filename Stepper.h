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
  if (mode == MODE_ST)
  {
    currentDir = dir;
    digitalWrite(pinDIR, dir);
  }
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
  if (mode == MODE_ST)
  {
    mode = MODE_RO;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_RO;
    ST();
  }
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
  if (mode == MODE_ST)
  {
    mode = MODE_RP;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_RP;
    ST();
  }
  return true;
}

void Stepper::columnRP(int v)
{
  turns = (v <= 0) ? 1 : v;
}

void Stepper::ST()
{
  switch (mode)
  {
  case MODE_SQ:
  {
    int a = floor(indexSeq / 2);
    if (seq[a] > 0)
    {

      if (currentDir == dir)
        currentDir = 1 - dir;
      digitalWrite(pinDIR, currentDir);
      currentSteps = steps - currentSteps;
      stepsHome = steps;
      mode = MODE_HOME;
      timeMS = millis();
    }
    else
    {
      currentSteps = 0;
      mode = MODE_ST;
      ST();
    }
    break;
  }
  case MODE_RA:
    mode = MODE_HOME;
    stepsHome = steps;
    timeMS = millis();
    break;
  case MODE_RO:
    mode = MODE_HOME;
    stepsHome = nSteps;
    timeMS = millis();
    break;
  case MODE_RP:
    mode = MODE_HOME;
    stepsHome = isPaused ? 0 : nSteps;
    timeMS = millis();
    break;
  case MODE_RW:
    mode = MODE_HOME;
    currentSteps = realSteps;
    stepsHome = nSteps;
    timeMS = millis();
    break;
  default:
    Serial.println(">> stop");
    mode = nextMode;
    nextMode = MODE_ST;
    currentSteps = 0;
    digitalWrite(pinSTP, LOW);
    SD(dir);
    break;
  }
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
  if (mode == MODE_ST)
  {
    mode = MODE_RA;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_RA;
    ST();
  }
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
  if (mode == MODE_ST)
  {
    mode = MODE_RW;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_RW;
    ST();
  }
}

void Stepper::action()
{
  switch (mode)
  {
  case MODE_ST:
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
          currentSteps = 0;
          digitalWrite(pinSTP, LOW);
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