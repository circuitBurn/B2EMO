#include <DFPlayerMini_Fast.h>
#include "sbus.h"
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

// Configuration

#define RC_HIGH 1811
#define RC_LOW 172

#define SBUS_RX 16
#define SBUS_TX 17

#define MP3_RX 18
#define MP3_TX 19
#define MP3_BUSY 23
#define MP3_CHANNEL 15
#define MP3_NUM_FILES 4

#define PIXEL_PIN 5
#define PIXEL_COUNT 1

#define M1 32
#define E1 33
#define M2 25
#define E2 26

bfs::SbusRx sbus_rx(&Serial1, SBUS_RX, SBUS_TX, true);
bfs::SbusData data;

SoftwareSerial mySerial(MP3_RX, MP3_TX);

Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

DFPlayerMini_Fast dfPlayer;

void setup()
{
  Serial.begin(115200);

  // dfPlayer Busy Signal
  pinMode(MP3_BUSY, INPUT);

  // Data Ring Motors
  pinMode(M1, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(E2, OUTPUT);

  // Neopixels
  pixels.begin();

  // DF Player
  mySerial.begin(9600);
  dfPlayer.begin(mySerial, true);
  dfPlayer.volume(30);
  dfPlayer.play(1);

  // SBUS
  sbus_rx.Begin();
}

void loop()
{
  if (sbus_rx.Read()) {
    data = sbus_rx.data();
    audioTrigger();
    dataRingMotors();
  }
}

/**
   Spin the internal data rings
*/
void dataRingMotors()
{
  int motorSpeed = map(data.ch[13], RC_LOW, RC_HIGH, -255, 255);

  if (motorSpeed < 0)
  {
    digitalWrite(M1, LOW);
  }
  else
  {
    digitalWrite(M1, HIGH);
  }

  analogWrite(E1, abs(motorSpeed));
}

void audioTrigger()
{
  int soundIndex = getSoundIndex(data.ch[MP3_CHANNEL]);

  // Check dfPlayer status
  if (soundIndex > 0 && digitalRead(MP3_BUSY) == HIGH)
  {
    dfPlayer.play(soundIndex);
  }
}
