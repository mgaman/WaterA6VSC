#include <avr/pgmspace.h>
#include "Arduino.h"
#include "Relay.h"

enum tapState lasttap = TAP_UNKNOWN;
extern unsigned PulseWidth;   // pulse to solenoid
extern bool polaritySwitch;

void RelaySet(int in1,int in2)
{
  digitalWrite(IN1,in1);
  digitalWrite(IN2,in2);
}

void TapChangeState(enum tapState t)
{
  lasttap = t;
  switch(t)
  {
    case TAP_OPEN:
      if (polaritySwitch)
      {
        RelaySet(HIGH,HIGH);
        delay(PulseWidth);
        RelaySet(LOW,HIGH);
      }
      else
      {
        RelaySet(LOW,LOW);
        delay(PulseWidth);
        RelaySet(HIGH,LOW);
      }
      break;
    case TAP_CLOSE:
      if (polaritySwitch)
      {
        RelaySet(LOW,LOW);
        delay(PulseWidth);
        RelaySet(HIGH,LOW);
      }
      else
      {
        RelaySet(HIGH,HIGH);
        delay(PulseWidth);
        RelaySet(LOW,HIGH);
      }
      break;
    case TAP_UNKNOWN:
      break;
  }
}

#if 0
const char *tapstatenames[] = {"unknown","tapopen","tapclose"};
const char *TapToText()
{
  return tapstatenames[lasttap];
}
#endif



