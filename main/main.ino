#include <DFPlayerMini_Fast.h>
#include "sbus.h"
#include <SoftwareSerial.h>
#include "CountDown.h"

// SBUS RC values
constexpr int RC_HIGH = 1811;
constexpr int RC_MID = 992;
constexpr int RC_LOW = 172;

// Channels (Start at 0, on Transmitter will start at 1)sss
constexpr int CH_UPPER_LIFT = 5;
constexpr int CH_LOWER_LIFT = 6;
constexpr int CH_DATA_RINGS = 14;
constexpr int CH_MP3_TRIG = 15;

// SBUS communication pins
constexpr int SBUS_RX = 16;
constexpr int SBUS_TX = 17;

// DFPlayer module
constexpr int MP3_RX = 18;
constexpr int MP3_TX = 19;
constexpr int MP3_BUSY = 23;

// Data ring motor control pins
constexpr int M1 = 32;
constexpr int E1 = 33;
constexpr int M2 = 25;
constexpr int E2 = 26;
constexpr int DATARING_SPEED = 110; // 0- 255

CountDown bodyLiftCountdown;
CountDown e1Countdown;
CountDown e2Countdown;

bool e1Direction = LOW;
int e1Speed = 0;
bool e2Direction = LOW;
int e2Speed = 0;

bfs::SbusRx sbus_rx(&Serial1, SBUS_RX, SBUS_TX, true);
bfs::SbusData data;

SoftwareSerial mySerial(MP3_RX, MP3_TX);
DFPlayerMini_Fast dfPlayer;

int prevLowerLift;
int prevUpperLift;
bool bodyLiftInProgress = false;

void setup()
{
  pinMode(MP3_BUSY, INPUT);
  pinMode(M1, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(E2, OUTPUT);

  mySerial.begin(9600);
  dfPlayer.begin(mySerial, true);
  dfPlayer.volume(8);

  sbus_rx.Begin();
  if (sbus_rx.Read())
  {
    data = sbus_rx.data();
    prevLowerLift = data.ch[CH_LOWER_LIFT];
    prevUpperLift = data.ch[CH_UPPER_LIFT];
  }
}

void loop()
{
  if (sbus_rx.Read())
  {
    data = sbus_rx.data();
    playTriggeredSound();
    controlDataRingMotors();
  }
}

void stopDataRings()
{
  e1Speed = 0;
  e1Countdown.stop();
  e2Speed = 0;
  e2Countdown.stop();
  bodyLiftCountdown.stop();
  updateMotor(M1, E1, e1Direction, e1Speed);
  updateMotor(M2, E2, e2Direction, e2Speed);
}

void controlDataRingMotors()
{
  if (isBodyLiftActive() && !bodyLiftInProgress)
  {
    e1Direction = LOW;
    e1Speed = DATARING_SPEED;
    e1Countdown.stop();
    e2Direction = LOW;
    e2Speed = DATARING_SPEED;
    e2Countdown.stop();
    bodyLiftInProgress = true;
  }
  else if (!isBodyLiftActive() && bodyLiftInProgress)
  {
    stopDataRings();
    bodyLiftInProgress = false;
  }
  else if (areDataRingsActive())
  {
    int spinLow = 150, spinHigh = 750, delayLow = 800, delayHigh = 2500;
    if (data.ch[CH_DATA_RINGS] == RC_MID)
    {
      spinLow = 100;
      spinHigh = 400;
      delayLow = 500;
      delayHigh = 500;
    }

    if (e1Countdown.isStopped() && e1Speed == 0)
    {
      e1Countdown.start(random(spinLow, spinHigh));
      e1Direction = random(0, 2);
      e1Speed = DATARING_SPEED;
    }
    if (e2Countdown.isStopped() && e2Speed == 0)
    {
      e2Countdown.start(random(spinLow, spinHigh));
      e2Direction = random(0, 2);
      e2Speed = DATARING_SPEED;
    }
    if (e1Countdown.isStopped() && e1Speed == DATARING_SPEED)
    {
      e1Speed = 0;
      e1Countdown.start(random(delayLow, delayHigh));
    }
    if (e2Countdown.isStopped() && e2Speed == DATARING_SPEED)
    {
      e2Speed = 0;
      e2Countdown.start(random(delayLow, delayHigh));
    }
  }
  else if (!bodyLiftInProgress)
  {
    stopDataRings();
  }

  updateMotor(M1, E1, e1Direction, e1Speed);
  updateMotor(M2, E2, e2Direction, e2Speed);
}

void playTriggeredSound()
{
  int soundIndex = getSoundIndex(data.ch[CH_MP3_TRIG]);
  if (soundIndex > 0)
    dfPlayer.play(soundIndex);
}

bool isBodyLiftActive()
{
  if (data.ch[CH_LOWER_LIFT] != prevLowerLift || data.ch[CH_UPPER_LIFT] != prevUpperLift)
  {
    if (bodyLiftCountdown.isStopped())
    {
      bodyLiftCountdown.start(250);
      prevLowerLift = data.ch[CH_LOWER_LIFT];
      prevUpperLift = data.ch[CH_UPPER_LIFT];
      return true;
    }
  }
  return bodyLiftCountdown.remaining() > 0;
}

bool areDataRingsActive()
{
  return data.ch[CH_DATA_RINGS] == RC_LOW || data.ch[CH_DATA_RINGS] == RC_MID;
}

const int debounceDelay = 50; // Adjust debounce delay as needed
int lastStableValue = -1;
unsigned long lastDebounceTime = 0;

int getSoundIndex(int triggerValue)
{
  static int lastValue = -1;
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();

  // If the value is unstable, reset debounce timer
  if (triggerValue != lastValue)
  {
    lastTime = currentTime;
  }

  // If stable for debounceDelay, accept new value
  if ((currentTime - lastTime) > debounceDelay)
  {
    lastStableValue = triggerValue;
  }

  lastValue = triggerValue;

  // Process only the stable value
  if (lastStableValue < 172 || lastStableValue > 1680)
    return -1;
  return 10 - (lastStableValue - 172) / 180;
}


//int getSoundIndex(int triggerValue)
//{
//    if (triggerValue < 172 || triggerValue > 1680)
//        return -1;
//    return 10 - (triggerValue - 172) / 180;
//}

void updateMotor(int motor, int enable, bool direction, int speed)
{
  digitalWrite(motor, direction);
  analogWrite(enable, speed);
}
