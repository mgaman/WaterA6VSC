/*
 * Mods:  make max mesage length parameter to constructor
*/
#ifndef A6_CALL_H_
#define A6_CALL_H_
enum ephoneEvent {SOUNDER_EVENT,CALL_EVENT,SMS_TEXT_ARRIVED_EVENT,SMS_PDU_ARRIVED_EVENT,CALL_DISCONNECT_EVENT,CREG_ARRIVED_EVENT,CGREG_ARRIVED_EVENT,
       UNKNOWN_REPLY_ARRIVED,MESSAGE_ARRIVED_EVENT,RING_EVENT,UNKNOWN_EVENT};
class A6CALL
{
  public:
    A6CALL(A6GPRS& a6gprs);
    ~A6CALL();
    // callbacks
    void OnPhoneEvent(enum ephoneEvent,int) __attribute__((weak));
    void OnDialin(char[]);
    void Parse(uint8_t[],unsigned);
    char smsbuffer[160];
    char smsSender[20];
    int getLineStatus();    // see +CREG?
    bool sendSMS(const char *,const char *);      // target, message
    bool sendSMS(const char *,const char *,enum eDCS); 
  private:
    A6GPRS *_a6gprs;
    bool nextLineSMS;
};
#endif
