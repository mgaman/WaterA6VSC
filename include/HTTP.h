bool HTTPGET(char *);
bool HTTPGET(char *,char *);
void HTTPResponse(uint8_t *,unsigned);
// callback when code was discovered
void HTTPCallback(int) __attribute__((weak));
