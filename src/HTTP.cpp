#include <Arduino.h>
#include <A6Services.h>
#include <A6Client.h>
#include "WCEEPROM.h"
#include "HTTP.h"
#include "Display.h"

extern A6GPRS gsm;
extern A6CLIENT client;

//  if (client.connect(EEPROMGetIndex(HTTPs),80))

bool HTTPGET(char *srv,char *url)
{
  bool rc = false;
  gsm.debugWrite(F("Connecting to "));
  gsm.debugWrite(srv);
  if (client.connect(srv,80))
  {
    if (client.connected())
    {
      gsm.debugWrite(F("TCP connected\n"));
      if (client.TxBegin())
      {
        gsm.debugWrite(F("Got >\n"));
        client.print(F("GET "));
        client.print(url);
        client.println(F(" HTTP/1.0"));
        client.println();
        if (client.TxEnd())
        {
          rc = true;
          gsm.debugWrite(F("get sent\n"));
        }
        else
        {
          gsm.debugWrite(F("get not sent\n"));
        }
      }
      else
      {
        gsm.debugWrite(F("Cannot send data\n"));
      }
    }
    else
    {
      gsm.debugWrite(F("connection failed\n"));
    }
  }
  else
  {
    gsm.debugWrite(F("connection failed\n"));
  }
  gsm.debugWrite(F("Force Stop\n"));
  client.stop();  // stop by force 
  return rc;
}

char server[50];
bool HTTPGET(char *url)
{
  strcpy(server,EEPROMGetIndex(HTTPs));
  return HTTPGET(server,url);
}

// accumulate line up to CR.LF
// scan line for HTTP/1. then look for code
// stage 0 waiting for LF
static char linebuf[100];
static unsigned offset = 0;
//static int stage = 0;
// scan response for HTTP/1.n nnn OK
// message may come in several bursts
static int httpparse(uint8_t c)
{
  int rc = -1;
  linebuf[offset++] = (char)c;
  if (offset >= sizeof(linebuf))
    offset=0;
  if (c==0x0a)
  {
    linebuf[offset] = 0;
    gsm.debugWrite(F("Analysing "));
    gsm.debugWrite(linebuf);
		gsm.debugWrite(F("\n"));
    offset=0;
    if (strncmp(linebuf,"HTTP/1.",7) == 0)
      rc = atoi(&linebuf[9]);
  }
  return rc;
}
void HTTPResponse(uint8_t *buf,unsigned len)
{
  unsigned j;
  int k;
  gsm.debugWrite(F("http length "));
  gsm.debugWrite((uint16_t)len);
  gsm.debugWrite(F("\n"));
  for (j=0;j<len;j++)
  {
    k = httpparse(buf[j]);
    if (k> -1)
      HTTPCallback(k);
  }
}

void HTTPCallback(int c)
{
  Serial.print(F("HTTP response: "));
  Serial.println(c);
  sprintf(server,"HTTP: %d",c);
  displayAddLine(2,server);
  displayShow();
}
