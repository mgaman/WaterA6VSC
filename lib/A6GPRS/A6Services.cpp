#include "Arduino.h"
#include "A6Services.h"
#include <ctype.h>

A6GPRS::A6GPRS(Stream &comm,unsigned cbs,unsigned maxmessagelength){
  comm_buf = new char[cbs];
  commbuffsize = cbs;
  CIPstatus = IP_STATUS_UNKNOWN;
  _comms = &comm;
  modemMessageLength = 0;
  modemmessage = new byte[maxmessagelength];
  maxMessageLength = maxmessagelength;
  ParseState = GETMM;
  nextLineSMS = false;
  smsmode = SMS_UNKNOWN;
}
A6GPRS::~A6GPRS(){}
static char TempBuf[100];
static const char *statusnames[] = {"IP INITIAL","IP START","IP CONFIG","IP IND","IP GPRSACT","IP STATUS","TCP/UDP CONNECTING","IP CLOSE","CONNECT OK"};
enum eCIPstatus A6GPRS::getCIPstatus()
{
  enum eCIPstatus es = IP_STATUS_UNKNOWN;
  RXFlush();
  print(F("AT+CIPSTATUS\r"));
  if (GetLineWithPrefix("+IPSTATUS:",TempBuf,50,1000))
  {
    char *s = TempBuf;  // skip over whitespace
    while (isspace(*s))
      s++;
    int i;
    for (i=0;i<9;i++)
      if (strncmp(s,statusnames[i],strlen(statusnames[i])) == 0)
      {
        es = (eCIPstatus)i;
        break;
      }
  }
  waitresp("OK\r\n",2000);  // trailing stuff
  CIPstatus = es;
  return es;
}

bool A6GPRS::begin(const char *apn,const char *user,const char *pwd)  // apn, username, password
{
  bool rc = false;
  RXFlush();
  CIPstatus = getCIPstatus();
  if (CIPstatus != IP_INITIAL)
  {
    print(F("AT+CGATT=0\r"));
    waitresp("OK\r\n",2000);
  }
  RXFlush();
  print(F("AT+CGATT=1\r"));
  if (waitresp("OK\r\n",20000))
  {
    RXFlush();
    strcpy(TempBuf,"AT+CGDCONT=1,\"IP\",\"");
    strcat(TempBuf,apn);
    strcat(TempBuf,"\"\r");
    print(TempBuf);
    if (waitresp("OK\r\n",1000))
    {
    RXFlush();
    print(F("AT+CGACT=1,1\r"));
      if (waitresp("OK\r\n",1000))
      {
        RXFlush();
        rc = true;
      }
    }
  }
  RXFlush();
  return rc;
}

bool A6GPRS::begin(const char *apn)  // apn
{
  return begin(apn,"","");
}

bool A6GPRS::getLocalIP(char ip[])
{
  int count = 15;
  RXFlush();
  ip[0] = 0;
  print(F("AT+CIFSR\r"));
  // skip empty lines
  while (count-- && !isdigit(ip[0]))
    GetLineWithPrefix(NULL,ip,50,2000);
  waitresp("OK\r\n",1000);
	strcpy(ip,"8.8.8.8");
	return true;
}
/*
 *  Serial input is a mixture of modem unsolicited messages e.g. +CGREG: 1, expected
 *  messages such as the precursor to data +CIPRCV:n, and data from the server
 *  We look for complete modem messages & react to +CIPRCV, transfer n bytes
 *  to the registered client parser
 *  wait for cr/lf cr , as terminators 
 */
uint8_t *A6GPRS::Parse(unsigned *length,bool *payload)
{
  uint8_t *mm = 0;
  *length = 0;
  bool alldone = false;
  char c = pop();
  *payload = false;
  while (!alldone && c != -1)
  {
    switch (ParseState)
    {
      case GETMM:
        modemmessage[modemMessageLength++] = c;
        // first check if we got an unsolicited message
        if (c == 0x0a)
        {
          *length = modemMessageLength;
          alldone = true;
          modemmessage[modemMessageLength]=0; // end marker
          mm = modemmessage;
          modemMessageLength = 0;
        }
        // then check if there is data coming in from an TCP connection
        else if (modemMessageLength == 8 && strncmp((const char *)modemmessage,"+CIPRCV:",8) == 0)
        {
            ParseState = GETLENGTH;
            modemMessageLength = 0;
        }
        else if (modemMessageLength == 11 && strncmp((const char *)modemmessage,"+TCPCLOSED:",11) == 0)
        {
            debugWrite(F("Server closed connection\r\n"));
            connectedToServer = false;
            alldone = true;   // bail out here
            ParseState = GETMM;
            modemMessageLength = 0;
            onServerDisconnect();   // let app know
        }
        break;
      case GETLENGTH:
        modemmessage[modemMessageLength++] = c;
        if (c == ',')
        {
          modemmessage[modemMessageLength] = 0;
          clientMsgLength = atoi((const char *)modemmessage);
          debugWrite(F("Payload length "));
          debugWrite((int)clientMsgLength);
          modemMessageLength = 0;
          if (clientMsgLength > maxMessageLength)
            debugWrite(F("Item is too large"));
          ParseState = GETDATA;
        }
        break;
      case GETDATA:
        if (clientMsgLength <= maxMessageLength)   // only copy data if there is space
          modemmessage[modemMessageLength++] = c;
        else
          modemMessageLength++;   // else just discard
        if (modemMessageLength == clientMsgLength)
        {
          modemmessage[modemMessageLength] = 0;  // add a text end marker
          ParseState = GETMM;
          modemMessageLength = 0;
          alldone = true;
          mm = modemmessage;
          *length = clientMsgLength;
          *payload = true;
        }
        break;
      case GETTELEVENT:
        break;
    }
  if (!alldone)
    c = pop();
  }
  return mm;
}

bool A6GPRS::getRTC(char rtc[])
{
  bool rc;
  RXFlush();
  print(F("AT+CCLK\r"));
  waitresp("\r\n",500);
  rc = GetLineWithPrefix("+CCLK:",rtc,30,500);
  waitresp("OK\r\n",500);
  return rc;
}

bool A6GPRS::getIMEI(char imei[])
{
  bool rc = false;
  RXFlush();
  print(F("AT+EGMR=2,7\r"));
  rc = GetLineWithPrefix("+EGMR:",imei,20,500);
  if (rc)
	  rc = waitresp("OK\r\n",500);
  return rc;
}

bool A6GPRS::setSmsMode(eSMSmode m)
{
  sprintf(TempBuf,"AT+CMGF=%d\r",m);
  print(TempBuf);
  smsmode = m;
  return waitresp("OK\r\n",1000); 
}

bool A6GPRS::dial(char number[])
{
  bool rc = false;
  print(F("ATD"));
  print(number);
  print(F("\r"));
  if (waitresp("OK",1000))
  {
    callState = DIALLING_OUT;
    rc = true;
  }
  return rc;
}
bool A6GPRS::answer()
{
  print(F("ATA\r"));
  callState = SPEAKING;
  return waitresp("OK",1000);
}
bool A6GPRS::hangup()
{
  print(F("ATH\r"));
  callState = PHONE_IDLE;
  return waitresp("OK",1000);
}
bool A6GPRS::callerID(bool enable)
{
  print(F("AT+CLIP="));
  print(enable);
  print(F("\r"));
  return waitresp("OK",1000); 
}

int A6GPRS::getRSSI()
{
  int rssi = -1;
  char response[20],*cp;
  RXFlush();
  print(F("AT+CSQ\r"));
  if (GetLineWithPrefix("+CSQ:",response,20,500))
  {
	if (waitresp("OK\r\n",500))
	{
		cp = strchr(response,' ');
		rssi = atoi(++cp);
	}
  }
  return rssi;	
}
