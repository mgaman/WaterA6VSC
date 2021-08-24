#include <time.h>
#include <string.h>
//#include <math.h>
#include <Arduino.h>
#include <avr/wdt.h>
#include "Configuration.h"
#include "A6Services.h"
#include "HTTP.h"
#include "WCEEPROM.h"
#include "Relay.h"
#include "Utilities.h"
#include "Display.h"
#include "RGB.h"

//inline int max(int a,int b) {return ((a)>(b)?(a):(b)); }

extern A6GPRS gsm;
extern unsigned long totalMeterCount[],CurrentMeterCount[],PreviousMeterCount[]; //,REPORT_RATE;
char tempbuf[300];  // for sprintf stuff, longest string is E?
extern char IMEI[];  // why ? *IMEI causes problems
extern char cooked[];
//extern bool tcpActive;
//extern uint32_t last200;
extern bool urlSent;
extern int rssi;
#ifdef ANTENNA_BUG
extern bool ignoreMeter;     // set true while sending emails
#endif
char taptemp[10];

// EEPROM message long so send in parts
void SendLongMessage ()
{
//  Serial.println(tempbuf);
  urlencode(tempbuf,cooked);
  strcpy(tempbuf,EEPROMGetIndex(WWWPATH));
  strcat(tempbuf,"/eeprom.php?");  // do not do url encode on this bit
  strcat(tempbuf,cooked);
//  Serial.println(tempbuf);
  urlSent = HTTPGET(tempbuf);
}
/*\
 *   Possible messages
 *   b boot
 *   c close tap
 *   d debug messages
 *   e set value in eeprom en,vvv  n index vvv value
 *   f debug on/off
 *   o open tap
 *   p ping
 *   r reset values to default
 *   s stop
 */
void SMSParse(const char * s)
{
//  unsigned long secs;
//  int l;
  char rtcbuff[25];
  char tpltemp[10];
  char rbtemp[10];
  strcpy(tempbuf,"Parse: ");
  strcat(tempbuf,s);
  Serial.println(tempbuf);
  displayAddLine(0,tempbuf);
  displayShow();
  gsm.getRTC(rtcbuff);
  time_t ep = GSMToEpoch(rtcbuff);
  switch(toupper(*s))
  {
    case 'B':
      // soft_restart();
  //    digitalWrite(ARD_RESET_TRIGGER,LOW);
      while (true) {}  // closed loop will eventually trigger the watchdog
      break;
    case 'C':
      TapChangeState(TAP_CLOSE);
      sprintf(tempbuf,"%s/RawData.php?AC=tapchange&imei=%s&UT=%lu&tap=3&epoch=%lu&m0=%lu&m1=%lu",EEPROMGetIndex(WWWPATH),
        IMEI,millis()/1000,ep,totalMeterCount[0],totalMeterCount[1]);
  //    Serial.println(tempbuf);
      urlSent = HTTPGET(tempbuf);
      break;
    case 'O':
      TapChangeState(TAP_OPEN);
      sprintf(tempbuf,"%s/RawData.php?AC=tapchange&imei=%s&UT=%lu&tap=2&epoch=%lu&m0=%lu&m1=%lu",EEPROMGetIndex(WWWPATH),
        IMEI,millis()/1000,ep,totalMeterCount[0],totalMeterCount[1]);
      urlSent = HTTPGET(tempbuf);
      break;
    case 'R':
      PreviousMeterCount[0] = CurrentMeterCount[0] = PreviousMeterCount[1] = CurrentMeterCount[1]= totalMeterCount[0] = totalMeterCount[1] = 0;
      InitEepromData();
#ifdef ANTENNA_BUG
      ignoreMeter = false;
#endif
      sprintf(tempbuf,"%s/RawData.php?AC=reset&imei=%s&UT=%lu&epoch=%lu",EEPROMGetIndex(WWWPATH),IMEI,millis()/1000,ep);
      urlSent = HTTPGET(tempbuf);
      break;
      /*
       * unsigned long secs = millis()/1000;
  secs *= 1000;  // do a day in 86.4 secs
  unsigned long days = secs / (24*3600L);
  unsigned long hours = (secs / 3600)%24;
  unsigned long mins = (secs/60)%60;
  unsigned long s = secs%60;
       */
    case 'P':
      // this string about 170 long
      sprintf(tempbuf,"%s/RawData.php?AC=ping&imei=%s&UT=%lu&m0=%lu&m1=%lu&tap=%d&epoch=%lu&ss=%d",EEPROMGetIndex(WWWPATH),
        IMEI,millis()/1000,totalMeterCount[0],totalMeterCount[1],lasttap,ep,rssi);
 //     Serial.println(tempbuf);
      urlSent = HTTPGET(tempbuf);
      break;
    case 'E':
    // E?
    // En,v   n 0-9 v value
      switch(*++s)
      {
        case '*':  // truncated EEPROM data
          // bug fix, cannot call EEPROMGetIndex twice as result overidden
          strcpy(tpltemp,EEPROMGetIndex(TPL0));
        //  strcpy(rbtemp,EEPROMGetIndex(REBOOT));
          strcpy(rbtemp,"7300");
          sprintf(tempbuf,"imei=%s&epoch=%lu&tpl0=%s&tpl1=%s&reboot=%s",IMEI,ep,tpltemp,EEPROMGetIndex(TPL1),rbtemp);
          SendLongMessage();
          break;
        case '?':   // full EEPROM data
          sprintf(tempbuf,"imei=%s",IMEI);
          sprintf(tempbuf,"%s&epoch=%lu",tempbuf,ep);
          sprintf(tempbuf,"%s&tpl0=%s",tempbuf,EEPROMGetIndex(TPL0));
          sprintf(tempbuf,"%s&tpl1=%s",tempbuf,EEPROMGetIndex(TPL1));
          sprintf(tempbuf,"%s&lis=%s",tempbuf,EEPROMGetIndex(LIS));
          sprintf(tempbuf,"%s&ph=%s",tempbuf,EEPROMGetIndex(DP));
          SendLongMessage();
          sprintf(tempbuf,"imei=%s",IMEI);
          sprintf(tempbuf,"%s&epoch=%lu",tempbuf,ep);
          sprintf(tempbuf,"%s&smtps=%s",tempbuf,EEPROMGetIndex(SmS));
          sprintf(tempbuf,"%s&ea=%s",tempbuf,EEPROMGetIndex(EA));
          SendLongMessage();
          sprintf(tempbuf,"imei=%s",IMEI);
          sprintf(tempbuf,"%s&epoch=%lu",tempbuf,ep);
          sprintf(tempbuf,"%s&pword=%s",tempbuf,EEPROMGetIndex(EP));
          sprintf(tempbuf,"%s&rcvr=%s",tempbuf,EEPROMGetIndex(ER));
          sprintf(tempbuf,"%s&apn=%s",tempbuf,EEPROMGetIndex(APN));
          sprintf(tempbuf,"%s&smsflag=%s",tempbuf,EEPROMGetIndex(SENDSMS));
          SendLongMessage();
          sprintf(tempbuf,"imei=%s",IMEI);
          sprintf(tempbuf,"%s&epoch=%lu",tempbuf,ep);
          sprintf(tempbuf,"%s&rr=%s",tempbuf,EEPROMGetIndex(RR));
          sprintf(tempbuf,"%s&pol=%s",tempbuf,EEPROMGetIndex(POL));
          sprintf(tempbuf,"%s&plw=%s",tempbuf,EEPROMGetIndex(PW));
          sprintf(tempbuf,"%s&https=%s",tempbuf,EEPROMGetIndex(HTTPs));
         sprintf(tempbuf,"%s&gpto=%s",tempbuf,EEPROMGetIndex(GPRSTO));
          SendLongMessage();
          break;
        case '0': // '0' -> 0
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          EEPROMSetIndex((eEEPROMIndex)(*s-'0'),s+2);
          break;
        case 'a': // 'a'->10
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
          EEPROMSetIndex((eEEPROMIndex)(*s-0x57),s+2);
          break;
        case 'A': // 'A'->10
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
          EEPROMSetIndex((eEEPROMIndex)(*s-0x37),s+2);
          break;
      }
      break;
    case 'F':  // gsm debug
      switch(*++s)
      {
        case '0':
          gsm.enableDebug = false;
          break;
        case '1':
          gsm.enableDebug = true;
          break;
      }
      break;
    case 'S':  // stop
      // this cannot be undone
      RGBInit();  // RGB off
      wdt_disable();
      while (true) {}
  }
}
