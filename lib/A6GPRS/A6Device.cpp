#include "Arduino.h"
#include "A6Device.h"
//#include <TimerOne.h>

A6Device::A6Device(){
}
A6Device::~A6Device(){
}
bool A6Device::reset()
{
  bool rc = false;
  inlevel = outlevel = 0;
  HWReset();
  if (waitresp("+CREG: 1\r\n",40000))
  {
    _comms->print(F("ATE0\r"));
    rc = waitresp("OK\r\n",1000);
    if (rc)
    {
      _comms->print(F("AT+CMGF=1\r")); // SMS text mode
      rc = waitresp("OK\r\n",1000);   
    }
  }
  return rc;
}

/*
 * Check incoming buffer for desired string, return true if found in time else false
 * Only use this version during setup time as serialEvent not yet available
 */
bool A6Device::waitresp(char const *response_string,int32_t timeout)
{
  int lengthtotest = strlen(response_string);
  const char *nextChar = response_string;
  bool started = false;
  uint32_t TimeOut = millis() + timeout;
  while (TimeOut > millis() && lengthtotest>0)
  {
    readAll();
// get next char from buffer, if no match discard, if match decrement lengthtotest & get next character
    char c = pop();
    if (c != -1)
    {
      if (c == *nextChar)
      {
        lengthtotest--;
        nextChar++;
        started = true;
      }
      else if (started)
      {
        lengthtotest = strlen(response_string);
        nextChar = response_string;
        started = false;
      }
    }
  }
  return TimeOut > millis();  // finished before time is up
}

void A6Device::push(char c)
{
  comm_buf[inlevel++] = c;
  if (inlevel == commbuffsize)  // handle wrap around
  {
    inlevel = 0;
    onException(CIRC_BUFFER_OVERFLOW,0);
  }
}

char A6Device::pop()
{
  char c;
  if (inlevel == outlevel)
    c = -1;
  else
  {
    c = comm_buf[outlevel++];
    debugWrite(c);
    if (outlevel == commbuffsize)  // handle wrap around
      outlevel = 0;
    if (inlevel == outlevel)
      inlevel = outlevel = 0;
  }
  return c;
}

// suck up all incoming data on UART, call this frequently
void A6Device::readAll()
{
  while (_comms->available())
    push((char)_comms->read());
}

void A6Device::RXFlush()
{
//  volatile char c;
//  while (_comms->available())
//    c = _comms->read();
  _comms->flush();
  inlevel = outlevel = 0;
}

void A6Device::TXFlush()
{
  _comms->flush();
}

bool A6Device::GetLineWithPrefix(char const *px,char *outbuf, int bufsize,int32_t timeout)
{
  int lengthtotest = strlen(px);
  const char *nextChar = px;
  uint32_t TimeOut = millis() + timeout;
  enum epx {BEFORE_PREFIX,IN_PREFIX,AFTER_PREFIX} eepx = BEFORE_PREFIX;
  bool alldone = false;
  if (px == NULL)
  {
    eepx = AFTER_PREFIX;
  }
  while (TimeOut > millis() && !alldone)
  {
    // serialEvent only works when explicitly called ot between loops
    while (_comms->available())
      push((char)_comms->read());
    // get next char from buffer, if no match discard, if match decrement lengthtotest & get next character
    char c = pop();
    if (c!= -1)
    {
      switch (eepx)
      {
        case BEFORE_PREFIX:
          if (c == *nextChar)
          {
            eepx = IN_PREFIX;
            lengthtotest--;
            nextChar++;
          }
          break;
        case IN_PREFIX:
          if (c == *nextChar)
          {
            lengthtotest--;
            nextChar++;
            if (lengthtotest == 0)
              eepx = AFTER_PREFIX;
          }
          else
          {
            eepx = BEFORE_PREFIX;
            nextChar = px;
            lengthtotest = strlen(px);
          }
          break;
        case AFTER_PREFIX:
          *outbuf++ = c;
          bufsize--;
          if (/*c == 0x0d ||*/ c == 0x0a || /*c == 0 ||*/ bufsize == 0)
          {
            *outbuf = 0; // add end marker
            alldone = true;
          }
          break;
      }
    }
  } 
  return TimeOut > millis();  // still have time left 
}

int A6Device::peek()
{
  if (inlevel == outlevel)
    return -1;
  else
    return comm_buf[outlevel];
}

size_t A6Device::write(uint8_t c)
{
  _comms->write(c);
  return 1;
}

size_t A6Device:: write(const uint8_t *buf, size_t size)
{
  size_t j;
  for (j=0; j<size; j++)
    write(buf[j]);
  return j;
}

