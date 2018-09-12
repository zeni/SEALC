class Vibro : public Motor
{
    int pin;
    int duration;
    bool isOn;
    int durationSeq[MAX_SEQ];
    int stateSeq[MAX_SEQ];
    String getType();
    void RO();
    void RP();
    void RA();
    void SQ();
    void RW();
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
    }
}

String Vibro::getType()
{
    return " (vibro)";
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
        Serial.println(String(duration) + " ms");
    }
    mode = MODE_RO;
    timeMS = millis();
}

void Vibro::initRP()
{
    duration = 1000;
    pause = 1000;
    isPaused = false;
}

void Vibro::columnRP(int v)
{
    duration = (v <= 0) ? 1 : v;
}

void Vibro::setRP(int v)
{
    isPaused = false;
    pause = (v <= 0) ? 1000 : v;
    duration = (duration <= 0) ? 1000 : duration;
    Serial.println(">> rotate " + String(duration) + "ms and pause " + String(pause) + "ms");
    mode = MODE_RP;
    timeMS = millis();
}

void Vibro::initSQ()
{
    indexSeq = 0;
    lengthSeq = 0;
    newBeat = true;
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
    v = (v <= 0) ? 0 : 1;
    stateSeq[(indexSeq - 1) / 2] = v;
    indexSeq++;
    lengthSeq = indexSeq / 2;
    indexSeq = 0;
    for (int i = 0; i < lengthSeq; i++)
    {
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
}

void Vibro::setRA(int v)
{
    Serial.println(">> vibro has no RA command");
}

void Vibro::setRW(int v)
{
    Serial.println(">> vibro has no RW command");
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

// rotate a number of steps
void Vibro::RA()
{
}

// continuous wave movement (like rotate but with changing speed)
void Vibro::RW()
{
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
        isOn = stateSeq[indexSeq];
        if (isOn)
            digitalWrite(pin, HIGH);
        else
            digitalWrite(pin, LOW);
        newBeat = false;
    }
    if ((millis() - timeMS) > durationSeq[indexSeq])
    {
        newBeat = true;
        indexSeq++;
        if (indexSeq >= lengthSeq)
            indexSeq = 0;
        timeMS = millis();
    }
}