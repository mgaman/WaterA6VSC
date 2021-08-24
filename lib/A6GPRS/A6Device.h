#ifndef A6DEVICE_H
#define A6DEVICE_H
enum eExceptions {BUFFER_OVERFLOW,URL_TOO_LONG,CIRC_BUFFER_OVERFLOW};

class A6Device: public Print
{
  public:
    A6Device();
    ~A6Device();
    virtual bool reset();
//    size_t write(uint8_t c);
    bool waitresp(char const *response_string,int32_t);
    void HWReset() __attribute__((weak));   // may be implemented by the caller 
    bool enableDebug;
    char *comm_buf;  // communication buffer +1 for 0x00 termination
    unsigned commbuffsize,inlevel, outlevel;  // data in comm_buf
    int dataAvailable;
    Stream *_comms;
    void onException(eExceptions,int) __attribute__((weak));
    char pop();       // get data from buffer, -1 if none available
    bool GetLineWithPrefix(char const *px,char *outbuf, int l,int32_t timeout);
    void push(char);   // add incoming data to buffer
    void debugWrite(unsigned long) __attribute__((weak));
    void debugWrite(int) __attribute__((weak));
    void debugWrite(uint16_t) __attribute__((weak));
    void debugWrite(char) __attribute__((weak));
    void debugWrite(char[]) __attribute__((weak));
    void debugWrite(const __FlashStringHelper*) __attribute__((weak));
    void readAll();
    void RXFlush();  // clear all data in input buffer
    void TXFlush();  // clear all data in input buffer
    size_t write(uint8_t);
    size_t write(const uint8_t *buf, size_t size);
    using Print::write;
    int peek();
};
#endif
