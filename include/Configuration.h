/*
 * Hardware Pins
*/
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/*
 * pins 0-1 Serial
 * 2-3 meter interrupts
 * 4 soft reset
 * 5-6 relays
 * 7 modem reset
 * 8 EEPROM reset bypass
 * 8,9 unused  (8 used by SIM800
 * 10,11,12 RGB
 * 13 led
 * 18-19 Serial1
 * 20 SDA
 * 21 SCL
 */
#define ANTENNA_BUG
//#define USE_BARO

// 0,1 UART
#define METER_DRDY_1 2 // alarm must be an interrupt pin
#define METER_DRDY_2 3 // alarm must be an interrupt pin
#define ARD_RESET_TRIGGER  4  // connect via 2K resistor to RESET
#define IN1  5 // relay pins
#define IN2  6
#define A6_RESET_TRIGGER  7 // hardware reset of modem
#define RESET_EEPROM 8 // if low reset EEPROM
#define RED_LED_PIN 10
#define GREEN_LED_PIN  11
#define BLUE_LED_PIN  12
#define ONBOARD_LED  13
// now need to use 20/21 for I2C so move to 18/19
// couldnt use 2/3 because pins already soldered in and are too short
// needs to move modem serial from serial1 to serial2
// water meter observer calibration

// MEGA MISO 50 MOSI 51  SCK 52
// UNO MISO 12 MOSI 11  SCK 13

// Mega 2560 interrupt pins are 2,3,18,19,20,21
/*
 *   ICSP   MISO (1)  VCC
 *          SCK       MOSI
 *          RESET     GND
 */

#ifdef MAIN_FILE
const char *DEFAULT_HOME_NUMBER = "+972545919886";
const char *DEFAULT_SMTP_SERVER = "smtp.mail.yahoo.com";
const char *DEFAULT_SENDER_ACCOUNT = "dhdh654321@yahoo.com";
const char *DEFAULT_SENDER_PASSWORD = "x";
const char *DEFAULT_RECEIVER_ACCOUNT = "mgadriver@gmail.com";
//const char *DEFAULT_APN = "uinternet";
const char *DEFAULT_APN = "net.hotm";
const char *DEFAULT_HTTP_SERVER = "henrylineage.com"; //"david-henry.dyndns.tv"
const char *DEFAULT_WWW_PATH = "/webdata/WC";
#else
extern const char *DEFAULT_HOME_NUMBER;
extern const char *DEFAULT_SMTP_SERVER;
extern const char *DEFAULT_SENDER_ACCOUNT;
extern const char *DEFAULT_SENDER_PASSWORD;
extern const char *DEFAULT_RECEIVER_ACCOUNT;
extern const char *DEFAULT_APN;
//const char *DEFAULT_APN = "net.hotm";
extern const char *DEFAULT_HTTP_SERVER; //"david-henry.dyndns.tv"
extern const char *DEFAULT_WWW_PATH;
#endif

#define DEFAULT_PULSE_WIDTH 100
#define DEFAULT_REPORT_RATE (600L)
#define DEFAULT_LEAK_IN_SECS 1800
#define DEFAULT_TICKS_PER_LITER 265
#define DEFAULT_GPRS_TIMEOUT 30
#define DEFAULT_REBOOT_TIME 7300

// Useful universal macros
#define SMS_LENGTH 160
#define TICKS_PER_SEC 1000
#define ONE_SECOND TICKS_PER_SEC
#define TEN_SECONDS (10*TICKS_PER_SEC)
#define TEN_MINUTES (600*TICKS_PER_SEC)

// GSM modem tcp/ip sessions
#define HTTPCID  1
#define SMTPCID  2
#define MQTTCID  3
#endif

// OLED stuff
#define Y_LINE0 10
#define Y_LINE1 21
#define Y_LINE2 31
/*
MySQL
ENUM ('ping','powerup','leak','eepromreport','tapchange','report',
# 'reset','calibrate') default 'report' ,
*/
