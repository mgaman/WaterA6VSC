/*
 *  Water control uses EEPROM to store operational data
 *  Amount varies with model, we assume 1024 bytes which is avaliable on all models
 */
#ifndef WCEEPROM_H
#define ECEEPROM_H

 struct eEEPROMmap
 {
  unsigned Tpl0;          // 0
  unsigned Tpl1;          // 1
  unsigned LeakInSecs;    // 2
  char DefaultPhone[20];  // 3
  char SMTPServer[30];    // 4
  char EmailAccount[30];  // 5
  char EmailPassword[20]; // 6 yeah I know its unsafe
  char EmailRecipient[30];// 7
  char Apn[20];           // 8
  bool sendSMS;           // 9
  uint32_t reportRate;    // a send report every N seconds;
  bool polarity;          // b
  unsigned pulsewidth;    // c
  char HTTPServer[30];    // d
  bool useSSL;            // e
  int gprstimeout;        // f
  char wwwpath[20];       // g
  unsigned reboottime;    //h
 };

enum eEEPROMIndex {TPL0,TPL1,LIS,DP,SmS,EA,EP,ER,APN,SENDSMS,RR,POL,PW,HTTPs,useSSL,GPRSTO,WWWPATH,REBOOT};
void EEPROMSetIndex(enum eEEPROMIndex i,const char * s);
char *EEPROMGetIndex(enum eEEPROMIndex i);

void InitEepromData(void);
void PrintEepromData(void);
char * EEPROMGetIndex(enum eEEPROMIndex i);
void EEPROMSetIndex(enum eEEPROMIndex i,const char * s);
void eepromgetarray(int startaddress,uint8_t *target,int l);
void eepromsetarray(int startaddress,uint8_t *source,int l);
void eepromgetstring(int startaddress,char *target,int maxlength);
void eepromsetstring(int startaddress,const char *source,int maxlength);
#endif
