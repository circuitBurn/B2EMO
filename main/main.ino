#include <DFPlayerMini_Fast.h>
#include "sbus.h"
#include <SoftwareSerial.h>
#include "CountDown.h"
#include "DataRingState.h"

// SBUS RC values
constexpr int RC_HIGH = 1811;
constexpr int RC_MID = 992;
constexpr int RC_LOW = 172;

// Channels (Offset by 1 from transmitter: Channel 1 on TX is 0 here)
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
constexpr int DATA_RING_SPEED = 110; // 0 - 255

// Data ring timers used to randomize spin/stop
CountDown e1Timer;
CountDown e2Timer;

// DFRobot motor controller control values
bool e1Direction = LOW;
int e1Speed = 0;
bool e2Direction = LOW;
int e2Speed = 0;

// Randomized spin and delay values
int spinLow = 0;
int spinHigh = 0;
int delayLow = 0;
int delayHigh = 0;

// FrSky SBUS
bfs::SbusRx sbus_rx(&Serial1, SBUS_RX, SBUS_TX, true);
bfs::SbusData data;

// DFPlayer Mini
SoftwareSerial mySerial(MP3_RX, MP3_TX);
DFPlayerMini_Fast dfPlayer;

// Sound trigger button debounce
const int debounceDelay = 50;
int lastStableValue = -1;
unsigned long lastDebounceTime = 0;

void setup()
{
  pinMode(M1, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(E2, OUTPUT);

  mySerial.begin(9600);
  dfPlayer.begin(mySerial, true);
  dfPlayer.volume(8);

  sbus_rx.Begin();
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

void controlDataRingMotors()
{
  // Read the data ring mode selection switch
  DataRingState state = getDataRingSwitchState();
  switch (state)
  {
    case DataRingState::FAST:
      {
        // Set quicker spin and delay times (ms)
        spinLow = 100;
        spinHigh = 400;
        delayLow = 500;
        delayHigh = 500;
        break;
      }
    case DataRingState::SLOW:
      {
        // Set longer spin and delay times (ms)
        spinLow = 150;
        spinHigh = 750;
        delayLow = 800;
        delayHigh = 2500;
        break;
      }
    case DataRingState::IDLE:
      {
        // Stop all timers and turn off the motors
        // Returns immediately since we don't need to check anything else
        e1Speed = 0;
        e2Speed = 0;
        e1Timer.stop();
        e2Timer.stop();
        updateMotor(M1, E1, e1Direction, e1Speed);
        updateMotor(M2, E2, e2Direction, e2Speed);
        return;
      }
  }

  // Start upper ring
  if (e1Timer.isStopped() && e1Speed == 0)
  {
    // Set a randomized spin time
    e1Timer.start(random(spinLow, spinHigh));

    // Randomize direction
    e1Direction = random(0, 2);

    // TODO: randomize speed
    e1Speed = DATA_RING_SPEED;
  }

  // Start lower ring
  if (e2Timer.isStopped() && e2Speed == 0)
  {
    // Set a randomized spin time
    e2Timer.start(random(spinLow, spinHigh));

    // Randomize direction
    e2Direction = random(0, 2);

    // TODO: randomize speed
    e2Speed = DATA_RING_SPEED;
  }

  // Stop upper ring
  if (e1Timer.isStopped() && e1Speed == DATA_RING_SPEED)
  {
    e1Speed = 0;

    // Set a randomized delay before starting again
    e1Timer.start(random(delayLow, delayHigh));
  }

  // Stop lower ring
  if (e2Timer.isStopped() && e2Speed == DATA_RING_SPEED)
  {
    e2Speed = 0;

    // Set a randomized delay before starting again
    e2Timer.start(random(delayLow, delayHigh));
  }

  updateMotor(M1, E1, e1Direction, e1Speed);
  updateMotor(M2, E2, e2Direction, e2Speed);
}

/**
   Returns the state corresponding to the data ring mode selector switch on the transmitter
*/
DataRingState getDataRingSwitchState()
{
  if (data.ch[CH_DATA_RINGS] == RC_LOW)
  {
    return DataRingState::FAST;
  }
  else if (data.ch[CH_DATA_RINGS] == RC_MID)
  {
    return DataRingState::SLOW;
  }

  return DataRingState::IDLE;
}

/**
   Communicate with a single motor on the motor controller
*/
void updateMotor(int motor, int enable, bool direction, int speed)
{
  digitalWrite(motor, direction);
  analogWrite(enable, speed);
}

/**
   Read the sound button index from 1 to 10
*/
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
  {
    return -1;
  }
  return 10 - (lastStableValue - 172) / 180;
}

/**
 * Play the triggered sound immediately
 */
void playTriggeredSound()
{
  int soundIndex = getSoundIndex(data.ch[CH_MP3_TRIG]);
  if (soundIndex > 0)
  {
    dfPlayer.play(soundIndex);
  }
}
