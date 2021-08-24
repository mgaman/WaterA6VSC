#ifndef A6SERVICE_H
#define A6SERVICE_H
#include "A6Device.h"
enum eCIPstatus {IP_INITIAL,IP_START,IP_CONFIG,IP_IND,IP_GPRSACT,IP_STATUS,TCPUDP_CONNECTING,IP_CLOSE,CONNECT_OK,IP_STATUS_UNKNOWN };
enum ePSstate { DETACHED,ATTACHED,PS_UNKNOWN };
enum ecallState {PHONE_IDLE,CALLER_RINGING,SPEAKING,DIALLING_OUT,CALLERID,CALL_DISCONNECTED};
enum eParseState {GETMM,GETLENGTH,GETDATA,GETTELEVENT};
enum eSMSmode {SMS_PDU,SMS_TEXT,SMS_UNKNOWN};
enum eDCS {ALPHABET_7BIT,ALPHABET_8BIT,ALPHABET_16BIT};


class A6GPRS: public virtual A6Device
{
  public:
    A6GPRS(Stream &comm,unsigned,unsigned); // constructor uartstream, circular buffer size, max message size
    ~A6GPRS(); // destructor
    enum eCIPstatus getCIPstatus();
    bool begin(const char *);  // apn
    bool begin(const char *,const char*,const char *);  // apn, username, password
    bool getLocalIP(char []);
    eCIPstatus CIPstatus;
    uint8_t *Parse(unsigned *,bool *);
    bool getRTC(char[]);
    bool getIMEI(char[]);
    bool connectedToServer;    // TCP connection to server
    bool setSmsMode(eSMSmode);
    bool dial(char[]);
    bool answer();
    bool hangup();
    bool callerID(bool);
    enum ecallState callState;
	int getRSSI();
    eSMSmode smsmode;
  private:
    void onServerDisconnect();
    unsigned maxMessageLength;
    volatile unsigned modemMessageLength;
    byte *modemmessage;
    eParseState ParseState;
    unsigned clientMsgLength;
    bool nextLineSMS;
};
#endif
