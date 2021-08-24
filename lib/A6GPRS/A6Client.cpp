#include "Arduino.h"
#include "A6Services.h"
#include "A6Client.h"

static char TempBuf[100];

A6CLIENT::A6CLIENT(A6GPRS &device)
{
  a6instance = &device;
  a6instance->connectedToServer = false;
  payloadReadIndex = 0;
  payloadSize = 0;
}
A6CLIENT::~A6CLIENT(){}

int A6CLIENT::connect(const char *path, uint16_t port)
{
  bool rc = false;
  char tt[10];
  a6instance->CIPstatus = a6instance->getCIPstatus();
  if (a6instance->CIPstatus == CONNECT_OK)
  {
	  a6instance->debugWrite(F("Close stale connection\n"));
	  stop();
	  a6instance->CIPstatus = a6instance->getCIPstatus();	  
  }
  if ( a6instance->CIPstatus == IP_CLOSE ||  a6instance->CIPstatus == IP_GPRSACT||  a6instance->CIPstatus == IP_INITIAL)
  {
    strcpy(TempBuf,"AT+CIPSTART=\"TCP\",\"");
    strcat(TempBuf,path);
    strcat(TempBuf,"\",");
    itoa(port,tt,10);
    strcat(TempBuf,tt); 
    strcat(TempBuf,"\r");
//    Serial.println(TempBuf);
    a6instance->RXFlush();  
    a6instance->print(TempBuf);
    if ( a6instance->waitresp("CONNECT OK",7500))  // must be less than 8 secs as in watchdog
    {
      a6instance->connectedToServer = true;
      rc = a6instance->waitresp("OK\r\n",500);
    }
    else {
      // might have been a false negative, stopping wont harm, may help
      stop();
    }
  } 
  return rc;
}

bool A6CLIENT::TxBegin()
{
  a6instance->RXFlush();
  a6instance->print(F("AT+CIPSEND\r"));
  return a6instance->waitresp(">",10000);
}

bool A6CLIENT::TxEnd()
{
  a6instance->write(0x1a);
  return a6instance->waitresp("OK\r\n",5000);
}

int A6CLIENT::connect(IPAddress ip, uint16_t port)
{
  // convert IP address to string
  char ipaddress[20];
 // uint8_t b[4];
  sprintf(ipaddress,"%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);
  return connect(ipaddress,port);
}

uint8_t A6CLIENT::connected(void)
{
  return a6instance->connectedToServer | (available()>0);
}
size_t A6CLIENT::write(uint8_t c)
{
  a6instance->write(c);
//  Serial.print('+');
  return 1;
}
size_t A6CLIENT::write(const void *buf, uint16_t len, uint32_t flags)
{
  uint8_t *pb = (uint8_t *)buf;
  Serial.print('?');
  for (uint16_t i = 0; i<len; i++)
    write(*pb++);
  return len;
}
int A6CLIENT::read(void *buf, uint16_t len, uint32_t flags)
{
  int data,count=0;
  unsigned j;
  while (j++ < len)
  {
    data = read();
    if (data == -1)
      break;
    else
    {
      *(uint8_t *)buf++ = data;
      count++;
    }
  }
  return count;
}

int A6CLIENT::read(void)
{
  if (payloadReadIndex < payloadSize)
    return (int)(pB[payloadReadIndex++]);
  else
    return -1;
}

int A6CLIENT::available(void)
{
  return payloadSize - payloadReadIndex;
}
int A6CLIENT::read(uint8_t *buf, size_t size)
{
  return read(buf,size,0);
}
size_t A6CLIENT::write(const uint8_t *buf, size_t size)
{
//  Serial.println('[');
  for (size_t i=0;i<size;i++)
    write(buf[i]);
  return size;
}
int A6CLIENT::peek()
{
  return a6instance->peek();
}

void A6CLIENT::flush()
{
  a6instance->TXFlush();
}

void A6CLIENT::stop()
{
  bool rc;
  a6instance->RXFlush();
  a6instance->print(F("AT+CIPCLOSE\r"));
  rc = a6instance->waitresp("OK\r\n",2000);
  a6instance->connectedToServer = !rc;
}
A6CLIENT::operator bool()
{
  return connected();
}

void A6CLIENT::payloadSetup(uint8_t *buf,unsigned sz)
{
  pB = buf;
  payloadReadIndex = 0;
  payloadSize = sz;
}

