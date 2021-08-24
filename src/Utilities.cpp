#include <avr/pgmspace.h>
//#include <Time.h>
#include <TimeLib.h>
#include "Arduino.h"
#include <A6Services.h>
#include <A6Client.h>
#include <A6Call.h>
#include "WCEEPROM.h"

char cooked[300];
extern A6CALL phone;

 // convert output string from GSM modem to unix type
time_t GSMToEpoch(char *s)
{
  int tt[7];
  byte delim[6] = {'/','/',',',':',':','+'};
  char *f;
//  Serial.print("..");Serial.println(s);
  // skip over quote mark
  char *tdstring;
  tdstring = strchr(s,0x22);
  if (tdstring != NULL)
    tdstring++;
  else
    tdstring = s;
//  Serial.print("++");Serial.println(tdstring);
  for (int i=0;i<6;i++)
  {
    f = strchr(tdstring,delim[i]);
    *f++ = 0; // isolate field
    tt[i] = atoi(tdstring);
 //   Serial.println(tt[i]); // field
    tdstring += strlen(tdstring)+1;   // next field
  }
  tt[0] += 2000;
  setTime(tt[3],tt[4],tt[5],tt[2],tt[1],tt[0]);  // note order h/m/s/d/m/y
  return now(); // + (tt[6]*900); // add diff from GMT
}
int urlencode(char *src, char *tgt)
{
  int offset = 0;
//  const char rfc3986[] = " !*'();:@&=+$,/?#[]";
  const char rfc3986[] = " !*'();:@+$,/?#[]";  // without & =
  while (*src)
  {
    if (strchr(rfc3986,*src))
    {
      sprintf(&tgt[offset],"%%%02X",*src);
      offset += 3;
    }
    else
      tgt[offset++] = *src;
    src++;
  }
  tgt[offset] = 0;
  return offset;
}

void errorprint(const char * file,int line)
{
//  Serial.print("Error: ");
//  Serial.print(file);
//  Serial.print(", Line:: ");
//  Serial.println(line);
  sprintf(cooked,"Error: %s Line: %d",file,line);
  Serial.println(cooked);
  phone.sendSMS(EEPROMGetIndex(DP),cooked);
}
