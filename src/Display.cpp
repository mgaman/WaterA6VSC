#include "Arduino.h"
#include <U8glib.h>
#include "Display.h"
#include <avr/pgmspace.h>

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI
char displaybuf[SCREEN_ROWS+1][SCREEN_COLS];  // allow for string end marker

void displayInit()
{
  if ( u8g.getMode() == U8G_MODE_R3G3B2 )
  {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT )
  {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW )
  {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR )
  {
    u8g.setHiColorByRGB(255,255,255);
  }
//  u8g.setFont(u8g_font_unifont);
//  u8g.setFont(u8g_font_6x10);
  u8g.setFont(u8g_font_courB10);
  memset(displaybuf,0,sizeof(displaybuf));
}

void displayAddLine(int line,const char *s)
{
  // strip trailing cr/lf if any
  char *l = strchr(s,0x0d);
  if (l)
    *l = 0;
  strcpy(displaybuf[line],s);
}

void displayAddLine(int line,const __FlashStringHelper *s)
{
  strcpy_PF(displaybuf[line],s);
}

void displayShow()
{
  u8g.firstPage();
  do {
    u8g.drawStr(0,10,displaybuf[0]);
    u8g.drawStr(0,21,displaybuf[1]);
    u8g.drawStr(0,31,displaybuf[2]);
  } while (u8g.nextPage());
}
