#include "Configuration.h"

void RelaySet(int l1,int l2);

enum tapState {TAP_UNKNOWN=1,TAP_OPEN,TAP_CLOSE};  // make these correspond to ENUM in SQL create table
extern enum tapState lasttap;

void TapChangeState(enum tapState t);
const char *TapToText();
