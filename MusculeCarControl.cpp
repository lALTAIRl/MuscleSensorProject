#define rightMove 3
#define leftMove 4
#define forwardMove 6
#define backwardMove 7
#define forwardLed 8
#define rightLed 9
#define leftLed 11
#define backwardLed 12

//управление:
//  первая  |     вторая 
//  запись  |     запись
// ________ |  ____________
//  вперёд  |     влево
// ________ |  ____________
//          | 
//  назад   |     вправо
// ________ |  ____________
//          |
// ничего   |     ничего
// ________ |  ____________

//для бегущего среднего:
//чем меньше коэфицент - тем больше отставание от реального графика
#define FILTER_COEF 0.1
//интервал опроса значения
#define FILTER_STEP 22 //ms
short sensorValue;
float filtredSensorValue;
unsigned long filter_timer;

const short plotPartsCount = 4;

const short plotMaxLevel = 1023;
short plotUpperZoneLevel = 600;
short plotMediumZoneLevel = 320;
const short plotMinimumZoneLevel = 0;

// 50ms - 16.8s 14.8809 - 5s, delay = timeOfMove/336;
// CHANGE TIME OF CAR MOTION HERE:
const float timeOfMove = 5;                                 //s
const float delayInPlayIteration = timeOfMove * 1000 / 336; //ms

const short arraySize = 350;
bool ForwardMovingArray[arraySize];
bool BackwardMovingArray[arraySize];
bool RightMovingArray[arraySize];
bool LeftMovingArray[arraySize];

void setup()
{
    pinMode(rightMove, OUTPUT);
    pinMode(leftMove, OUTPUT);
    pinMode(forwardMove, OUTPUT);
    pinMode(backwardMove, OUTPUT);

    pinMode(forwardLed, OUTPUT);
    pinMode(rightLed, OUTPUT);
    pinMode(leftLed, OUTPUT);
    pinMode(backwardLed, OUTPUT);
    Serial.begin(9600);

    RecordMusculeActivity();
    //RecordFakeValues();
    SetAllValuesToDefaults();
    //DoDelayTreeSecBeforePlay();
    delay(1000);
    PlayRecordedMusculeActivity();
    SetAllValuesToDefaults();
}

void CalibrateLevels()
{
    // Serial.println(F("Prepare to strain the muscle to the maximum on 3 seconds."));
    // Serial.println(F("Calibration begins in 3"));
    // Serial.println(F("Calibration begins in 2"));
    // Serial.println(F("Calibration begins in 1"));
    // Serial.print(F("Strain the muscle to the maximum on 3 s"));

    uint32_t now = millis();
    PrintPlot(0, 0, 550);
    while (millis() - now <= 3000)
    {
        if (millis() - now == 1000 || millis() - now == 2000)
        {
            PrintPlot(0, 0, 550);
        }
        PrintPlot(0, 0, 0);
    }
    PrintPlot(0, 0, 1023);

    long counter = 0;
    double average = 0;
    SetAllLedValuesToMax();
    now = millis();
    while (millis() - now < 3000)
    {
        sensorValue = analogRead(A7);
        PrintPlot(sensorValue, sensorValue, 0);
        average += sensorValue;
        counter++;
    }
    SetAllValuesToDefaults();
    PrintPlot(0, 0, 1023);
    average /= counter;
    Serial.println(F("Thanks, dude, you are really strong. Now we can start record. Your average is "));
    Serial.println(average);
    plotUpperZoneLevel = average - average / 2.75;
    plotMediumZoneLevel = average / 3;
    Serial.println(F("Your plotUpperZoneLevel is "));
    Serial.println(plotUpperZoneLevel);
    Serial.println(F("Your plotMediumZoneLevel is "));
    Serial.println(plotMediumZoneLevel);
}

void SetAllLedValuesToMax()
{
    digitalWrite(leftLed, HIGH);
    digitalWrite(rightLed, HIGH);
    digitalWrite(backwardLed, HIGH);
    digitalWrite(forwardLed, HIGH);
}

void PlayRecordedMusculeActivity()
{
    for (short i = 0; i < arraySize; i++)
    {
        // ↙
        if (BackwardMovingArray[i] == true && LeftMovingArray[i] == true)
        {
            AccelerateBackward();
            TurnLeft();
        }

        // ↘
        else if (BackwardMovingArray[i] == true && RightMovingArray[i] == true)
        {
            AccelerateBackward();
            TurnRight();
        }

        // ↖
        else if (ForwardMovingArray[i] == true && LeftMovingArray[i] == true)
        {
            AccelerateForward();
            TurnLeft();
        }
        else if (ForwardMovingArray[i] == true && RightMovingArray[i] == true)
        {
            AccelerateForward();
            TurnRight();
        }

        // ↓
        else if (BackwardMovingArray[i] == true && RightMovingArray[i] == LeftMovingArray[i])
        {
            SetDefaultLedValues(true);
            SetDefaultMoveValues(true);
            AccelerateBackward();
        }

        // ↑
        else if (ForwardMovingArray[i] == true && RightMovingArray[i] == LeftMovingArray[i])
        {
            SetDefaultLedValues(true);
            SetDefaultMoveValues(true);
            AccelerateForward();
        }

        // ←
        else if (ForwardMovingArray[i] == BackwardMovingArray[i] && LeftMovingArray[i] == true)
        {
            TurnLeft();
            SetDefaultLedValues(false);
            SetDefaultMoveValues(false);
        }
        // →
        else if (ForwardMovingArray[i] == BackwardMovingArray[i] && RightMovingArray[i] == true)
        {
            TurnRight();
            SetDefaultLedValues(false);
            SetDefaultMoveValues(false);
        }
        delay(delayInPlayIteration);
    }
}

void SetAllValuesToDefaults()
{
    SetDefaultLedValues(true);
    SetDefaultLedValues(false);
    SetDefaultMoveValues(true);
    SetDefaultMoveValues(false);
}

void RecordMusculeActivity()
{
    CalibrateLevels();

    DoDelayTreeSecBeforeRecord();

    Serial.println(F("Recording Acceleration..."));
    RecordAcceleration();
    Serial.println(F("End Recording Acceleration"));

    DoDelayTreeSecBeforeRecord();

    Serial.println(F("Recording Turns..."));
    RecordTurns();
    Serial.println(F("End Recording Turns"));
}

void DoDelayTreeSecBeforeRecord()
{
    Serial.println(F("Recording starts in 3s"));
    delay(1000);
    Serial.println(F("Recording starts in 2s"));
    delay(1000);
    Serial.println(F("Recording starts in 1s"));
    delay(1000);
}

void DoDelayTreeSecBeforePlay()
{
    Serial.println(F("Play starts in 3s"));
    delay(1000);
    Serial.println(F("Play starts in 2s"));
    delay(1000);
    Serial.println(F("Play starts in 1s"));
    delay(1000);
}

void RecordAcceleration()
{
    short currentPlotPartsCount = 1;
    for (short i = 0; i < arraySize; i++)
    {
        short sensorValue = 0;
        if (millis() - filter_timer > FILTER_STEP)
        {
            filter_timer = millis(); // просто таймер
            // читаем значение (не обязательно с аналога, это может быть ЛЮБОЙ датчик)
            sensorValue = analogRead(A7);
            // основной алгоритм фильтрации. Внимательно прокрутите его в голове, чтобы понять, как он работает
            filtredSensorValue += (sensorValue - filtredSensorValue) * FILTER_COEF;
        }

        if (i == arraySize / plotPartsCount * currentPlotPartsCount)
        {
            PrintPlot(sensorValue, filtredSensorValue, plotMaxLevel);
            currentPlotPartsCount++;
        }
        else
        {
            PrintPlot(sensorValue, filtredSensorValue, plotMinimumZoneLevel);
        }

        if (filtredSensorValue < plotMediumZoneLevel)
        {
            ForwardMovingArray[i] = false;
            BackwardMovingArray[i] = false;
            SetDefaultLedValues(false);
        }
        else if (filtredSensorValue >= plotUpperZoneLevel)
        {
            ForwardMovingArray[i] = true;
            BackwardMovingArray[i] = false;
            SetForwardLedValues();
        }
        else
        {
            ForwardMovingArray[i] = false;
            BackwardMovingArray[i] = true;
            SetBackwardLedValues();
        }
    }
    SetAllValuesToDefaults();
}

void RecordTurns()
{
    short currentPlotPartsCount = 1;
    for (short i = 0; i < arraySize; i++)
    {
        short sensorValue = 0;
        if (millis() - filter_timer > FILTER_STEP)
        {
            filter_timer = millis(); // просто таймер
            // читаем значение (не обязательно с аналога, это может быть ЛЮБОЙ датчик)
            sensorValue = analogRead(A7);
            // основной алгоритм фильтрации. Внимательно прокрутите его в голове, чтобы понять, как он работает
            filtredSensorValue = sensorValue * FILTER_COEF + filtredSensorValue * (1 - FILTER_COEF);
        }
        if (i == arraySize / plotPartsCount * currentPlotPartsCount)
        {
            PrintPlot(sensorValue, filtredSensorValue, plotMaxLevel);
            currentPlotPartsCount++;
        }
        else
        {
            PrintPlot(sensorValue, filtredSensorValue, 0);
        }

        if (filtredSensorValue < plotMediumZoneLevel)
        {
            RightMovingArray[i] = false;
            LeftMovingArray[i] = false;
            SetDefaultLedValues(true);
        }
        else if (filtredSensorValue >= plotUpperZoneLevel)
        {
            RightMovingArray[i] = false;
            LeftMovingArray[i] = true;
            SetLeftTurnLedValues();
        }
        else
        {
            RightMovingArray[i] = true;
            LeftMovingArray[i] = false;
            SetRightTurnLedValues();
        }
    }
    SetAllValuesToDefaults();
}

void RecordFakeValues()
{
    short currentPlotPartsCount = 1;
    for (short i = 0; i < arraySize; i++)
    {
        short sensorValue = 1023;
        filtredSensorValue = sensorValue;

        if (i == arraySize / plotPartsCount * currentPlotPartsCount)
        {
            PrintPlot(sensorValue, filtredSensorValue, plotMaxLevel);
            currentPlotPartsCount++;
        }
        else
        {
            PrintPlot(sensorValue, filtredSensorValue, plotMinimumZoneLevel);
        }

        if (sensorValue < plotMediumZoneLevel)
        {
            ForwardMovingArray[i] = false;
            BackwardMovingArray[i] = false;
            SetDefaultLedValues(false);
        }
        else if (sensorValue >= plotUpperZoneLevel)
        {
            ForwardMovingArray[i] = true;
            BackwardMovingArray[i] = false;
            SetForwardLedValues();
        }
        else
        {
            ForwardMovingArray[i] = false;
            BackwardMovingArray[i] = true;
            SetBackwardLedValues();
        }
    }
    SetAllValuesToDefaults();
    currentPlotPartsCount = 1;
    for (short i = 0; i < arraySize; i++)
    {
        short sensorValue;
        if (i < 75)
            sensorValue = plotMediumZoneLevel + 1;
        else if (i < 150)
            sensorValue = 0;
        else if (i < 195)
            sensorValue = 1023;
        else if (i < 255) //235
            sensorValue = plotMediumZoneLevel + 1;
        else
            sensorValue = 0;
        // sensorValue = 0;
        filtredSensorValue = sensorValue;
        if (i == arraySize / plotPartsCount * currentPlotPartsCount)
        {
            PrintPlot(sensorValue, filtredSensorValue, plotMaxLevel);
            currentPlotPartsCount++;
        }
        else
        {
            PrintPlot(sensorValue, filtredSensorValue, 0);
        }

        if (sensorValue < plotMediumZoneLevel)
        {
            RightMovingArray[i] = false;
            LeftMovingArray[i] = false;
            SetDefaultLedValues(true);
        }
        else if (sensorValue >= plotUpperZoneLevel)
        {
            RightMovingArray[i] = false;
            LeftMovingArray[i] = true;
            SetLeftTurnLedValues();
        }
        else
        {
            RightMovingArray[i] = true;
            LeftMovingArray[i] = false;
            SetRightTurnLedValues();
        }
    }
    SetAllValuesToDefaults();
}

void loop()
{
}

void PrintPlot(short sensorValue, float filtredSensorValue, short iterationDivider)
{
    Serial.print(0); // To freeze the lower limit
    Serial.print(F(" "));
    Serial.print(plotMaxLevel); // To freeze the upper limit
    Serial.print(F(" "));
    Serial.print(plotMediumZoneLevel);
    Serial.print(F(" "));
    Serial.print(plotUpperZoneLevel);
    Serial.print(F(" "));
    Serial.print(sensorValue);
    Serial.print(F(" "));
    Serial.print(iterationDivider);
    Serial.print(F(" "));
    Serial.println(filtredSensorValue);
}

void PrintCalibrationCountdoun(short dividerValue)
{
    Serial.print(0); // To freeze the lower limit
    Serial.print(F(" "));
    Serial.print(plotMaxLevel); // To freeze the upper limit
    Serial.print(F(" "));
    Serial.print(dividerValue); // To freeze the lower limit
    Serial.print(F(" "));
}

void SetDefaultMoveValues(bool isTurns)
{
    if (isTurns)
    {
        digitalWrite(leftMove, LOW);
        digitalWrite(rightMove, LOW);
    }
    else
    {
        digitalWrite(forwardMove, LOW);
        digitalWrite(backwardMove, LOW);
    }
}

void SetDefaultLedValues(bool isTurns)
{
    if (isTurns)
    {
        digitalWrite(leftLed, LOW);
        digitalWrite(rightLed, LOW);
    }
    else
    {
        digitalWrite(forwardLed, LOW);
        digitalWrite(backwardLed, LOW);
    }
}

void SetLeftTurnLedValues()
{
    digitalWrite(leftLed, HIGH);
    digitalWrite(rightLed, LOW);
}
void SetRightTurnLedValues()
{
    digitalWrite(rightLed, HIGH);
    digitalWrite(leftLed, LOW);
}
void SetLeftTurnMoveValues()
{
    digitalWrite(leftMove, HIGH);
    digitalWrite(rightMove, LOW);
}
void SetRightTurnMoveValues()
{
    digitalWrite(rightMove, HIGH);
    digitalWrite(leftMove, LOW);
}

void SetForwardLedValues()
{
    digitalWrite(forwardLed, HIGH);
    digitalWrite(backwardLed, LOW);
}
void SetBackwardLedValues()
{
    digitalWrite(backwardLed, HIGH);
    digitalWrite(forwardLed, LOW);
}
void SetForwardMoveValues()
{
    digitalWrite(forwardMove, HIGH);
    digitalWrite(backwardMove, LOW);
}
void SetBackwardMoveValues()
{
    digitalWrite(backwardMove, HIGH);
    digitalWrite(forwardMove, LOW);
}

void TurnLeft()
{
    SetLeftTurnLedValues();
    SetLeftTurnMoveValues();
}

void TurnRight()
{
    SetRightTurnLedValues();
    SetRightTurnMoveValues();
}

void AccelerateForward()
{
    SetForwardLedValues();
    SetForwardMoveValues();
}

void AccelerateBackward()
{
    SetBackwardLedValues();
    SetBackwardMoveValues();
}