/*
 *   Following functions are implemented on behalf of the A6GPRSDevice class
 *   None of them HAVE to be present but I would recommend that at least HWRESET is implemented
 *   else the modem may never get  going
 */
#include <Arduino.h>
#include <A6Services.h>
#include <A6Call.h>

#define TRANSISTOR_CONTROL 7  // connect to base of transistor
#define A6_RESET_TIME 50 // ms

//extern A6GPRS gsm;
//extern A6CALL phone;
extern bool smsArrived;

void A6Device::HWReset()
{
  /*
   * The A6 modem takes about 70mA throuh the reset pin which is too much for an Arduino GPIO pin
   * Instead connect an Arduino GPIO pin to the base of a transistor and toggle that.
   * Connect the emitter of the transistor to ground and the collector to the A6 reset pin
   * http://electronics.stackexchange.com/questions/82222/how-to-use-a-transistor-to-pull-a-pin-low
   */
  pinMode(TRANSISTOR_CONTROL,OUTPUT);
  digitalWrite(TRANSISTOR_CONTROL,LOW);
  digitalWrite(TRANSISTOR_CONTROL,HIGH);
  delay(A6_RESET_TIME);
  digitalWrite(TRANSISTOR_CONTROL,LOW);
}
void A6Device::debugWrite(unsigned long c)
{
  if (enableDebug)
    Serial.print(c);
}
void A6Device::debugWrite(uint16_t c)
{
  if (enableDebug)
    Serial.print(c);
}
void A6Device::debugWrite(int c)
{
  if (enableDebug)
    Serial.print(c);
}
void A6Device::debugWrite(char c)
{
  if (enableDebug)
    Serial.write(c);
}
void A6Device::debugWrite(char *s)
{
  if (enableDebug)
    Serial.print(s);
}void A6Device::debugWrite(const __FlashStringHelper*s)
{
  if (enableDebug)
    Serial.print(s);
}
/*
 *   Callback examples
 */
void A6GPRS::onServerDisconnect()
{
  Serial.println("TCP was disconnected by Server");
}

void A6Device::onException(eExceptions e,int i)
{
  Serial.print("Exception: ");
  Serial.print(e);
  Serial.print(", Value: ");
  Serial.println(i);
}

void A6CALL::OnDialin(char *cid)
{
  Serial.print(cid);
  Serial.println(" calling");
}

void A6CALL::OnPhoneEvent(enum ephoneEvent ev,int parm)
{
//  Serial.print("Event: ");
//  Serial.println(ev);
  switch(ev)
  {
    case SOUNDER_EVENT:
      break;
    case CALL_EVENT:
      break;
    case SMS_TEXT_ARRIVED_EVENT:
      smsArrived = true;
      break;
    case SMS_PDU_ARRIVED_EVENT:
      break;
    case CALL_DISCONNECT_EVENT:
      break;
    case CREG_ARRIVED_EVENT:
      break;
    case CGREG_ARRIVED_EVENT:
      break;
    case UNKNOWN_REPLY_ARRIVED:
      break;
    case MESSAGE_ARRIVED_EVENT:
      break;
    case RING_EVENT:
      break;
    case UNKNOWN_EVENT:
      break;
  }
}

