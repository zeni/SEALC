class Vibro : public Motor
{
    int pin;
    int duration;
    bool isOn;
    int durationSeq[MAX_SEQ];
    int stateSeq[MAX_SEQ];
    int currentDurationSeq[MAX_SEQ];
    int currentStateSeq[MAX_SEQ];
    String getType();
    void RO();
    void RP();
    void SQ();
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
    void initSQ();
    void setSQ(int v);
    void columnSQ(int v);

  public:
    Vibro();
    Vibro(int p);
};

Vibro::Vibro() : Motor()
{
}

Vibro::Vibro(int p) : Motor()
{
    pin = p;
    speed = 0;
    isOn = false;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    for (int j = 0; j < MAX_SEQ; j++)
    {
        durationSeq[j] = 0;
        stateSeq[j] = 0;
        currentDurationSeq[j] = 0;
        currentStateSeq[j] = 0;
    }
}

String Vibro::getType()
{
    return " (vibro)";
}

void Vibro::SS(int v)
{
    Serial.println(">> vibro has no SS command");
}

void Vibro::setSD(int v)
{
    Serial.println(">> vibro has no SD command");
}

void Vibro::setRO(int v)
{
    Serial.print(">> rotate ");
    if (v <= 0)
    {
        duration = 0;
        Serial.println("continuously");
    }
    else
    {
        duration = v;
        Serial.print(duration);
        Serial.println(" ms");
    }
    mode = MODE_RO;
    timeMS = millis();
}

void Vibro::columnRP(int v)
{
    duration = (v <= 0) ? 1000 : v;
}

void Vibro::setRP(int v)
{
    isPaused = false;
    pause = (v <= 0) ? 1000 : v;
    duration = (duration <= 0) ? 1000 : duration;
    Serial.print(">> rotate ");
    Serial.print(duration);
    Serial.print("ms and pause ");
    Serial.print(pause);
    Serial.println("ms");
    mode = MODE_RP;
    timeMS = millis();
}

void Vibro::columnSQ(int v)
{
    if (indexSeq % 2 == 0)
    {
        v = (v <= 0) ? 0 : v;
        durationSeq[indexSeq / 2] = v;
    }
    else
    {
        v = (v <= 0) ? 0 : 1;
        stateSeq[(indexSeq - 1) / 2] = v;
    }
    indexSeq++;
}

void Vibro::setSQ(int v)
{
    Serial.print(">> sequence: ");
    newBeat = true;
    v = (v <= 0) ? 0 : 1;
    stateSeq[(indexSeq - 1) / 2] = v;
    indexSeq++;
    lengthSeq = indexSeq / 2;
    indexSeq = 0;
    currentLengthSeq = lengthSeq;
    currentIndexSeq = 0;
    for (int i = 0; i < currentLengthSeq; i++)
    {
        currentDurationSeq[i] = durationSeq[i];
        currentStateSeq[i] = stateSeq[i];
        Serial.print(durationSeq[i]);
        Serial.print(" (");
        Serial.print(stateSeq[i]);
        Serial.print(")");
        Serial.print("|");
    }
    Serial.println();
    mode = MODE_SQ;
    timeMS = millis();
}

void Vibro::setRR(int v)
{
    Serial.println(">> vibro has no RR command");
    mode = MODE_IDLE;
}

void Vibro::setRA(int v)
{
    Serial.println(">> vibro has no RA command");
    mode = MODE_IDLE;
}

void Vibro::setRW(int v)
{
    Serial.println(">> vibro has no RW command");
    mode = MODE_IDLE;
}

void Vibro::action()
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
    case MODE_RA:
    case MODE_RR:
    case MODE_RW:
        break;
    case MODE_RO:
        RO();
        break;
    case MODE_RP:
        RP();
        break;
    case MODE_SQ:
        SQ();
        break;
    case MODE_WA:
        WA();
        break;
    }
}

void Vibro::ST()
{
    Serial.println(">> stop");
    digitalWrite(pin, LOW);
    isOn = false;
    mode = MODE_IDLE;
    deQ();
}

void Vibro::SD()
{
}

// rotation
void Vibro::RO()
{
    if (!isOn)
    {
        isOn = true;
        digitalWrite(pin, HIGH);
    }
    if (duration == 0)
        deQ();
    else
    {
        if ((millis() - timeMS) > duration)
            ST();
    }
}

// rotation with pause
void Vibro::RP()
{
    if (isPaused)
    {
        if ((millis() - timeMS) > pause)
        {
            isPaused = false;
            isOn = true;
            digitalWrite(pin, HIGH);
            timeMS = millis();
        }
        else
            deQ();
    }
    else
    {
        if (!isOn)
        {
            digitalWrite(pin, HIGH);
            isOn = true;
        }
        if ((millis() - timeMS) > duration)
        {
            isPaused = true;
            isOn = false;
            digitalWrite(pin, LOW);
            deQ();
            timeMS = millis();
        }
    }
}

void Vibro::WA()
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
        isOn = false;
        digitalWrite(pin, LOW);
    }
}

// continuous hammer movement with pattern of angles
void Vibro::SQ()
{
    if (newBeat)
    {
        deQ();
        isOn = currentStateSeq[currentIndexSeq] > 0;
        if (isOn)
            digitalWrite(pin, HIGH);
        else
            digitalWrite(pin, LOW);
        newBeat = false;
    }
    if ((millis() - timeMS) > currentDurationSeq[currentIndexSeq])
    {
        newBeat = true;
        currentIndexSeq++;
        if (currentIndexSeq >= currentLengthSeq)
            currentIndexSeq = 0;
        timeMS = millis();
    }
}