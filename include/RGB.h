#include "Configuration.h"

enum rgbled { RED_LED, BLUE_LED ,GREEN_LED};
enum rgbstate { LED_ON, LED_OFF };
void RGBInit();
void RGBActivate(enum rgbled,enum rgbstate);
void RGBToggle(enum rgbled);
