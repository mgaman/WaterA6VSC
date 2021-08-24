/*
 * Implements the generic Arduino client class
 */
#ifndef A6CLIENT_H
#define A6CLIENT_H
#include "Arduino.h"  
#include "Client.h"

class A6CLIENT: public Client
{
  public:
    A6CLIENT(A6GPRS& D);
    ~A6CLIENT(); // destructor
    void operator=(const A6CLIENT& other);
    // these are clients methods that we must implement
    int connect(IPAddress ip, uint16_t port);
    int connect(const char *host, uint16_t port);    
    uint8_t connected(void);
    size_t write(uint8_t);
    size_t write(const uint8_t *buf, size_t size);
    size_t write(const void *buf, uint16_t len, uint32_t flags = 0);
    int read(void *buf, uint16_t len, uint32_t flags = 0);
    int read(void);
    int available(void);
    int read(uint8_t *buf, size_t size);
    int peek();
    void flush();
    void stop();
    operator bool();
    bool TxBegin();
    bool TxEnd();
//    using Print::write;
    void payloadSetup(uint8_t *,unsigned);
  private:
    A6GPRS *a6instance;
    int payloadReadIndex,payloadSize;
    uint8_t *pB;  // pointer to payload buffer
};
#endif
