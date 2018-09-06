#define MAX_SEQ 10 // max length of sequence for beat
// stepper modes
#define MODE_STOP 0
#define MODE_ROTATE 1
#define MODE_MOVE 2
#define MODE_WAVE 4
#define MODE_BEAT 5
#define MODE_HOME 6
#define MODE_DIR 7

class Stepper : public Motor
{
  int pinSTP, pinDIR;
  int waveDir; // increasing / decreasing speed
  int turns;   // for rotate (0=continuous rotation)
  int realSteps;
  void rotate();
  void move();
  void beat();
  void wave();
  void goHome();
  void stepperStep();
  void moveStep();

public:
  Stepper();
  Stepper(int n, int stp, int dir);
  void setDir(int v);
  void setRotate(int v);
  void setMove(int v);
  void setWave(int v);
  void stop();
  void action();
  String getType();
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

void Stepper::setDir(int v)
{
  if (v > 0)
    v = 1;
  switch (mode)
  {
  case MODE_STOP:
    if (v < 0)
      dir = 1 - dir;
    else
      dir = v;
    currentDir = dir;
    digitalWrite(pinDIR, dir);
    break;
  default:
    if (v < 0)
      dir = 1 - dir;
    else
      dir = v;
    break;
  }
  Serial.print(">> dir: ");
  if (dir > 0)
  {
    Serial.println("CCW");
  }
  else
    Serial.println("CW");
}

void Stepper::setRotate(int v)
{
  Serial.print(">> rotate ");
  if (v <= 0)
  {
    v = 0;
    Serial.println("continuously");
  }
  else
    Serial.println(String(v) + " turn(s)");
  turns = v;
  steps = turns * nSteps;
  if (mode == MODE_STOP)
  {
    mode = MODE_ROTATE;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_ROTATE;
    stop();
  }
}

void Stepper::stop()
{
  Serial.println(">> stop");
  switch (mode)
  {
  case MODE_BEAT:
  {
    int a = floor(indexSeq / 2);
    if (seq[a] > 0)
    {

      if (currentDir == dir)
      {
        currentDir = 1 - dir;
      }
      digitalWrite(pinDIR, currentDir);
      currentSteps = steps - currentSteps;
      stepsHome = steps;
      mode = MODE_HOME;
      timeMS = millis();
    }
    else
    {
      currentSteps = 0;
      mode = MODE_STOP;
      stop();
    }
    break;
  }
  case MODE_MOVE:
    mode = MODE_HOME;
    stepsHome = steps;
    timeMS = millis();
    break;
  case MODE_ROTATE:
    mode = MODE_HOME;
    stepsHome = nSteps;
    timeMS = millis();
    break;
  case MODE_WAVE:
    mode = MODE_HOME;
    currentSteps = realSteps;
    stepsHome = nSteps;
    timeMS = millis();
    break;
  default:
    mode = nextMode;
    nextMode = MODE_STOP;
    currentSteps = 0;
    digitalWrite(pinSTP, LOW);
    setDir(dir);
    break;
  }
}

// stop and back to init pos
void Stepper::goHome()
{
  Serial.println(stepsHome);
  Serial.println(currentSteps);
  Serial.println(currentDir);
  if ((millis() - timeMS) > speed)
  {
    if (currentSteps >= stepsHome)
    {
      currentSteps = 0;
      mode = MODE_STOP;
      stop();
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
    mode = MODE_STOP;
    stop();
  }
  else
  {
    currentSteps++;
    stepperStep();
    timeMS = millis();
  }
}

void Stepper::setMove(int v)
{
  if (v < 0)
    v = 0;
  v = v % 360;
  Serial.print(">> move ");
  Serial.print(v);
  Serial.println(" degrees");
  steps = v / 360.0 * nSteps;
  currentSteps = 0;
  if (mode == MODE_STOP)
  {
    mode = MODE_MOVE;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_MOVE;
    Stepper::stop();
  }
}

void Stepper::setWave(int v)
{
  if (v <= 0)
    v = 1;
  Serial.print(">> speed cycle: ");
  Serial.print(v);
  Serial.println(" per turn");
  steps = nSteps / (2.0 * v);
  currentSteps = 0;
  realSteps = currentSteps;
  waveDir = 0;
  if (mode == MODE_STOP)
  {
    mode = MODE_WAVE;
    timeMS = millis();
  }
  else
  {
    nextMode = MODE_WAVE;
    Stepper::stop();
  }
}

void Stepper::action()
{
  switch (mode)
  {
  case MODE_STOP:
    break;
  case MODE_ROTATE:
    rotate();
    break;
  case MODE_MOVE:
    move();
    break;
  case MODE_WAVE:
    wave();
    break;
  case MODE_BEAT:
    beat();
    break;
  case MODE_HOME:
    goHome();
    break;
  }
}

// rotation
void Stepper::rotate()
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
      {
        moveStep();
      }
    }
  }
  else
  {
    stop();
    Serial.println("Stopped: speed is 0.");
  }
}

// rotate a number of steps
void Stepper::move()
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
    stop();
    Serial.println("Stopped: speed is 0.");
  }
}

// continuous wave movement (like rotate but with changing speed)
void Stepper::wave()
{
  if (speed > 0)
  {
    int s;
    if (waveDir == 0)
    {
      s = speed * (steps - currentSteps);
    }
    else
    {
      s = speed * currentSteps;
    }
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
        stepperStep();
        currentSteps++;
        timeMS = millis();
      }
    }
  }
  else
  {
    stop();
    Serial.println("Stopped: speed is 0.");
  }
}

// continuous hammer movement with pattern of angles
void Stepper::beat()
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
        {
          newBeat = true;
        }
        int a = floor(indexSeq / 2);
        if (a >= lengthSeq)
        {
          indexSeq = 0;
        }
      }
      else
      {
        int a = floor(indexSeq / 2);
        currentSteps++;
        if (seq[a] > 0)
        {
          stepperStep();
        }
        timeMS = millis();
      }
    }
  }
  else
  {
    stop();
    Serial.println("Stopped: speed is 0.");
  }
}