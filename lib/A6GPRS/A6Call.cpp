#include "Arduino.h"
#include "A6Services.h"
#include "A6Call.h"

A6CALL::A6CALL(A6GPRS& a6gprs)
{
  _a6gprs = &a6gprs;
}
A6CALL::~A6CALL(){}
void A6CALL::Parse(uint8_t buffer[],unsigned length)
{
  int parm;
  char *start;
  if (length == 1)
    return; // single trailing LF
  if (strncmp((const char *)buffer,"RING",4) == 0)
  {
    _a6gprs->callState = CALLER_RINGING;
    OnPhoneEvent(RING_EVENT,parm);    
  }
  else if (strncmp((const char *)buffer,"ERROR",5) == 0)
  {
    _a6gprs->callState = CALL_DISCONNECTED;
    OnPhoneEvent(CALL_DISCONNECT_EVENT,0);
  }
  else if (strncmp((const char *)buffer,"+CIEV:",6) == 0)
  {
    start = strchr((const char *)buffer,'"'); // point to begining of string
    if (start)
    {
      start++;
      if (strncmp(start,"SOUNDER\",",9) == 0)
      {
        start += 9;
        parm = atoi(start);
        if (parm == 0)
          _a6gprs->callState = SPEAKING;
        OnPhoneEvent(SOUNDER_EVENT,parm);
      }
      else if (strncmp(start,"CALL\",",6) == 0)
      {
        start += 6;
        parm = atoi(start);
        if (parm == 0)
          _a6gprs->callState = PHONE_IDLE;
        OnPhoneEvent(CALL_EVENT,parm);
      }
      else if (strncmp(start,"MESSAGE\",",9) == 0)
      {
        start += 9;
        parm = atoi(start);
        if (parm == 0)
          _a6gprs->callState = PHONE_IDLE;
        OnPhoneEvent(MESSAGE_ARRIVED_EVENT,parm);
      }
      else
      {
        strcpy(smsbuffer,start);
        OnPhoneEvent(UNKNOWN_EVENT,strlen(start));
      }
    }
    else
    {
      strcpy(smsbuffer,(const char *)buffer);
      OnPhoneEvent(UNKNOWN_EVENT,strlen((const char *)buffer));
    }
  }
  else if (strncmp((const char *)buffer,"+CLIP:",6) == 0)
  {
    start = strchr((const char *)buffer,'"'); // point to begining of string
    if (start)
    {
      start++;
      char *end = strchr(start,'"');  // point to end of number
      if (end)
        *end = 0;
      _a6gprs->callState = CALLERID;
      OnDialin(start);
    }
  }
  else if (strncmp((const char *)buffer,"+CREG:",6) == 0)
  {
    start = strchr((const char *)buffer,' '); // point to begining of string
    parm = atoi(start);
    OnPhoneEvent(CREG_ARRIVED_EVENT,parm);
  }
  else if (strncmp((const char *)buffer,"+CGREG:",7) == 0)
  {
    start = strchr((const char *)buffer,' '); // point to begining of string
    parm = atoi(start);
    OnPhoneEvent(CGREG_ARRIVED_EVENT,parm);
  }
  else if (strncmp((const char *)buffer,"+CMT:",5) == 0)
  {
    // extract sender
    start = strchr((const char *)buffer,'"');
    if (start)  // found if this is a text message
    {
      start++;    // in text mode +CMT: "sender",,"toimestamp", message in next line
      char *end = strchr(start,'"');
      *end = 0;
      strcpy(smsSender,start);
    }
    else
    {
      // in PDU mode +CMT:  ,length   pdu block on next line
      start = strchr((const char *)buffer,',');
      *smsSender = 0;   // will be taken from PDU block
    }
    nextLineSMS = true;
  }
  else if (nextLineSMS)
  {
    strcpy(smsbuffer,(const char *)buffer);
    nextLineSMS = false;
    if (_a6gprs->smsmode == SMS_TEXT)
      OnPhoneEvent(SMS_TEXT_ARRIVED_EVENT,strlen(smsbuffer));
    else
      OnPhoneEvent(SMS_PDU_ARRIVED_EVENT,strlen(smsbuffer));      
  }
  else
  {
    // use SMS buffer to temporarily save unknown string
    strcpy(smsbuffer,(const char *)buffer);
    OnPhoneEvent(UNKNOWN_REPLY_ARRIVED,strlen(smsbuffer));    
  }
}

bool A6CALL::sendSMS(const char *addr,const char *text,enum eDCS alphabet)
{
  bool rc = false;
//  int i,pdulength;
//  char tt[8];
  switch (_a6gprs->smsmode)
  {
    case SMS_TEXT:
      _a6gprs->print(F("AT+CMGS=\""));
      _a6gprs->print(addr);
      _a6gprs->print(F("\"\r"));
      _a6gprs->waitresp(">",1000); 
      _a6gprs->print(text);
      _a6gprs->write(0x1a);
      rc = _a6gprs->waitresp("+CMGS:",5000);
      break;
#if 0
    case SMS_PDU:
      pdulength = encodePDU(pdubuffer,addr,text,alphabet);
//      Serial.println(pdulength);
      if (pdulength > 0)
      {
        _a6gprs->print(F("AT+CMGS="));
        itoa(tt,pdulength-1,10);
        _a6gprs->print(tt);
        _a6gprs->print(F("\r"));
        rc = _a6gprs->waitresp("> ",1000);
        // pdu is binary, send as text  
        if (rc)
        {
          for (i=0;i<pdulength;i++)
          {
            sprintf(TempBuf,"%02X",pdubuffer[i]);
            _a6gprs->print(TempBuf);
          }
          // finally send ctrl-Z
          delay(10);
          _a6gprs->print((unsigned char)0x1a);
          rc =_a6gprs->waitresp("OK\r\n",7000);
        }
      }
      break;
#endif
    default:
      break;
  }
  return rc;
}

bool A6CALL::sendSMS(const char *addr,const char *text)
{
  return sendSMS(addr,text,ALPHABET_7BIT);
}

