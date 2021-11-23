/*
 *  Modifications for Linux environment, ensure include file names
 *  are case sensitive
 *  
 *  BUGS REBOOT
 *       watchdog compilation
 *       not getting IP address
 *       not sending http
 */
#include <Arduino.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <string.h>
//#include <Time.h>
#include <TimeLib.h>
#include <limits.h>
#include <A6Services.h>
#include <A6Client.h>
#include <A6Call.h>
#define MAIN_FILE
#include "Configuration.h"
#include "RGB.h"
#include "Relay.h"
#include "WCEEPROM.h"
#include "Utilities.h"
#include "Display.h"
#include "HTTP.h"

extern HardwareSerial Serial1;
void serialEvent1();

A6GPRS gsm(Serial1, 200, 2000);  //200 bytes for unsolicited, 2000 bytes for http response
A6CLIENT client(gsm);
A6CALL phone(gsm);

enum eWaterFlow {
	FLOW_UNKNOWN, FLOWING, NOT_FLOWING, WATER_LEAK
};
void SMSParse(const char * s);

// Global Data
#define BIG_BUFF_SIZE 150
static char bigbuff[BIG_BUFF_SIZE];
bool ledstate = false;
bool SMSout = false;  // save on sending SMS just print to Serial
unsigned MAX_FLOW_TIME = DEFAULT_LEAK_IN_SECS;
unsigned long REPORT_RATE = DEFAULT_REPORT_RATE;
unsigned reboottime = DEFAULT_REBOOT_TIME;
unsigned PulseWidth = 100;
bool polaritySwitch = true;
int gprstimeout = DEFAULT_GPRS_TIMEOUT;
char IMEI[20];
time_t ep;
unsigned long totalMeterCount[2] = { 0, 0 }, prevReportTime = 0;
unsigned long StartCountingPeriod = UINT_MAX, CurrentMeterCount[2] = { 0, 0 },
		PreviousMeterCount[2] = { 0, 0 };
bool tcpActive = false;
bool ignoreMeter = false;
bool smsArrived = false;
uint32_t httptimeout = UINT_MAX;
bool doReport = false;
//int reports[2] = {0,0};  // reports sent, succ completion
//uint32_t last200 = 0;
//int sprintfsize = -1;
uint32_t prevsampletime = 0, prevanalyzetime = 0;
bool sampleFlag;
long lastblink = -1;
unsigned long nextSample;
bool smsSent = false;
uint32_t lastLeakSentSecs = ULONG_MAX;
bool urlSent = false;
int rssi;

/*
 *   Check both meters
 */
enum eWaterFlow FlowAnalyze(int meter) {
	enum eWaterFlow wf = FLOW_UNKNOWN;
	if (CurrentMeterCount[meter] == 0) // not flowing
	{
		wf = NOT_FLOWING;
	} else  // is flowing
	{
		if (PreviousMeterCount[meter] == 0) // just started
		{
			// If water is flowing and tap supposed to be closed, change the state
			lasttap = TAP_OPEN;
			wf = FLOWING;
			StartCountingPeriod = millis() / ONE_SECOND;
		} else {
			if ((millis() / ONE_SECOND - StartCountingPeriod) >= MAX_FLOW_TIME) // running for a minute or more)
			{
				wf = WATER_LEAK;
			}
		}
	}
	PreviousMeterCount[meter] = CurrentMeterCount[meter];
	CurrentMeterCount[meter] = 0;
	return wf;
}

void MeterCount1() {
	totalMeterCount[0]++;
	CurrentMeterCount[0]++;
}

void MeterCount2() {
	totalMeterCount[1]++;
	CurrentMeterCount[1]++;
}

void setup() {
	bool stillDown = true;
	// Serial terminal and modem
	Serial.begin(115200);
	Serial1.begin(115200);
	pinMode(RESET_EEPROM,INPUT_PULLUP);
	if (digitalRead(RESET_EEPROM) == HIGH) {
		Serial.println("Reset EEPROM");
		InitEepromData();   // if jumper from 8 to earth disconnected
	}
	else 
		Serial.println("Do not reset EEPROM");
	// set up the various GPIO hardware
	// LEDS
	REPORT_RATE = atoi(EEPROMGetIndex(RR));
	SMSout = atoi(EEPROMGetIndex(SENDSMS));
	RGBInit();
	pinMode(ONBOARD_LED, OUTPUT);
	digitalWrite(ONBOARD_LED, LOW);
	// Timeout trigger
	pinMode(ARD_RESET_TRIGGER, OUTPUT);
	digitalWrite(ARD_RESET_TRIGGER, HIGH);
	// RELAY
// force relay to power off position
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	RelaySet(HIGH, LOW);
	TapChangeState(TAP_OPEN);
	// Hall Effect Meters
	pinMode(METER_DRDY_1, INPUT);
	pinMode(METER_DRDY_2, INPUT);
	// response for water meter pulses
	attachInterrupt(digitalPinToInterrupt(METER_DRDY_1), MeterCount1, RISING);
	attachInterrupt(digitalPinToInterrupt(METER_DRDY_2), MeterCount2, RISING);
	// Timer0 is already used for millis() - we'll just interrupt somewhere
	// in the middle and call the "Compare A" function below
	OCR0A = 0xAF;
	TIMSK0 |= _BV(OCIE0A);
	PrintEepromData();
	gsm.enableDebug = false;
	displayInit();
	displayAddLine(0, F("Water Control"));
	displayShow();
	gsm.enableDebug = true;
	if (gsm.reset()) {
		Serial.println(F("gsm up"));
		gsm.debugWrite(F("Current Time: "));
		if (gsm.getRTC(bigbuff))
			ep = GSMToEpoch(bigbuff);
		else
			ep = 0;
		gsm.debugWrite((unsigned long)ep);
		gsm.debugWrite(F("\n"));
		if (!gsm.setSmsMode(SMS_TEXT))
			Serial.println(F("SMS text mode not set"));
		if (!gsm.getIMEI(IMEI))
			strcpy(IMEI, "fakeimei");
		if (gsm.begin(EEPROMGetIndex(APN))) {
      		Serial.println("Connected to GPRS");
			// get our ip address
			if (gsm.getLocalIP(bigbuff)) {
        		gsm.debugWrite(F("IP address: "));
        		gsm.debugWrite(bigbuff);
				gsm.debugWrite(F("\n"));
				displayAddLine(0, bigbuff);
				displayShow();
				// announce to the world we're up
				if (sprintf(bigbuff,"%s/RawData.php?AC=powerup&imei=%s&UT=%lu&m0=%lu&m1=%lu&epoch=%lu",
					EEPROMGetIndex(WWWPATH), IMEI, millis() / 1000,
					totalMeterCount[0], totalMeterCount[1], ep) > BIG_BUFF_SIZE)
						errorprint(__FILE__, __LINE__);
				Serial.println(bigbuff);
				urlSent = HTTPGET(bigbuff);
				Serial.println(urlSent ? F("powerup sent") : F("powerup not sent"));
//				if (SMSout)
//					phone.sendSMS(EEPROMGetIndex(DP), "Powerup");
				if (urlSent) {
					stillDown = false;
					if (SMSout)  // only send if have gprs connection
						phone.sendSMS(EEPROMGetIndex(DP), "Powerup");
				}
			} else
				Serial.println(F("Couldn't get IP address"));
		} else
			Serial.println(F("Connect GPRS failed"));
	} else
		Serial.println(F("gsm down"));
	wdt_enable(WDTO_8S);
	if (stillDown)  // force restart if not online
		while (true) {}
}

uint8_t httpbuff[2000];
void loop() {
	uint8_t *pM; // points to a message
	unsigned payloadLength;
	bool ispayload;
	char rtctime[30];
	serialEvent1();
	pM = gsm.Parse(&payloadLength, &ispayload); // check if unsolicited message is TCP payload or not
	if (payloadLength > 0) {
		if (ispayload)
			// data event
			client.payloadSetup(pM, payloadLength);
		else
			// phone event
			phone.Parse(pM, payloadLength);
	}
	if (client.available()) {
		int k = client.available();
		client.read(httpbuff, k); // get it all
		HTTPResponse((uint8_t *)httpbuff, (unsigned)k); // now analyse
	}
	if (urlSent && !client.connected()) {
		Serial.println();
		Serial.println(F("Stopping."));
		wdt_reset(); // reset the watchdog
		client.stop();
		urlSent = false;
	}
	// check if time to sample
	if (sampleFlag) {
		sampleFlag = false;
		enum eWaterFlow wf[2];
		rssi = gsm.getRSSI();
		sprintf(bigbuff, "Up %lu SS %d", millis() / 1000, rssi);
		displayAddLine(0, bigbuff);
		sprintf(bigbuff, "0:%lu,1:%lu", CurrentMeterCount[0],
				CurrentMeterCount[1]);
		displayAddLine(1, bigbuff);
		displayShow();
		// check both meters and decide how to proceed
		wf[0] = FlowAnalyze(0);
		wf[1] = FlowAnalyze(1);
		if (wf[0] == NOT_FLOWING && wf[1] == NOT_FLOWING) // both report not flowing
				{
			RGBActivate(RED_LED, LED_OFF);
			RGBActivate(BLUE_LED, LED_OFF);
			RGBActivate(GREEN_LED, LED_ON);
			//smsSent = false;    // dont stop next leak message
		} else if (wf[0] == FLOWING && wf[1] == FLOWING)  // both report flowing
				{
			RGBActivate(RED_LED, LED_OFF);
			RGBActivate(BLUE_LED, LED_ON);
			RGBActivate(GREEN_LED, LED_OFF);
			//smsSent = false;    // dont stop next leak message
		} else if (wf[0] == WATER_LEAK || wf[1] == WATER_LEAK) { // either report a leak
			RGBActivate(RED_LED, LED_ON);
			RGBActivate(BLUE_LED, LED_OFF);
			RGBActivate(GREEN_LED, LED_OFF);
			// send message
			if (gsm.getRTC(rtctime))
				ep = GSMToEpoch(rtctime);
			else
				ep = 0;
			if (sprintf(bigbuff,
					"%s/RawData.php?AC=leak&imei=%s&UT=%lu&m0=%lu&m1=%lu&tap=%d&epoch=%lu",
					EEPROMGetIndex(WWWPATH), IMEI, millis() / 1000,
					totalMeterCount[0], totalMeterCount[1], lasttap,
					ep) > BIG_BUFF_SIZE)
				errorprint(__FILE__, __LINE__);
			TapChangeState(TAP_CLOSE);
			urlSent = HTTPGET(bigbuff);
			Serial.println(urlSent ? F("leak sent") : F("leak not sent"));
			wdt_reset();  // reset the watchdog
			uint32_t achshav = millis() / 1000;
			if ((!smsSent) || (achshav > (lastLeakSentSecs + 600))) // send SMS for sustained leak every 10 minutes
					{
				sprintf(bigbuff, "leak uptime=%lu ts=%s", achshav, rtctime);
				phone.sendSMS(EEPROMGetIndex(DP), bigbuff);
				smsSent = true;
				lastLeakSentSecs = achshav;  // rolls around every 250 days
			}
		} else {
			RGBActivate(BLUE_LED, LED_ON);
			RGBActivate(GREEN_LED, LED_ON);
			// what to do if meters dont agree ?
		}
	}
	if (millis() / ONE_SECOND > (prevReportTime + REPORT_RATE)) {
		// time for a regular report
		// send message
		if (gsm.getRTC(rtctime))
			ep = GSMToEpoch(rtctime);
		else
			ep = 0;
		prevReportTime = millis() / ONE_SECOND;
		if (sprintf(bigbuff,
				"%s/RawData.php?AC=report&imei=%s&UT=%lu&m0=%lu&m1=%lu&epoch=%lu&tap=%d&ss=%d",
				EEPROMGetIndex(WWWPATH), IMEI, millis() / 1000,
				totalMeterCount[0], totalMeterCount[1], ep, lasttap,
				rssi) > BIG_BUFF_SIZE)
			errorprint(__FILE__, __LINE__);
		int retries = 2;	// add a simple retry meachanism
		gsm.debugWrite(bigbuff);
//		Serial.println(bigbuff);
		while (retries > 0) {
//			urlSent = HTTPGET(bigbuff);
			if (HTTPGET(bigbuff)) {
				Serial.println(F("report sent"));
				displayAddLine(2, "report sent");
				retries = 0;	// force exit
			} else {
				Serial.println(F("report NOT sent"));
				displayAddLine(2, "report NOT sent");
				wdt_reset();
				retries--;
			}
			displayShow();
		}
	}

	if (smsArrived) {
		smsArrived = false;
		SMSParse(phone.smsbuffer);
	}
	wdt_reset(); // reset the watchdog
}

/*
 * TIMER0 is used to generate the millis counter. Lets use its interrupt
 */

SIGNAL(TIMER0_COMPA_vect) {
	long ms = millis() / 1000;
	if (ms != lastblink) {
		lastblink = ms;
		digitalWrite(ONBOARD_LED, ledstate ? LOW : HIGH);
		ledstate = !ledstate;
#if 0
    // fake some data
    MeterCount1();
    MeterCount2();
#endif
	}
	if (millis() > nextSample) {
		sampleFlag = true;
		nextSample = millis() + TEN_SECONDS;  // every ten secs
	}
	// force poweron every 2 hours
#ifdef REBOOT
	if (ms > reboottime)
	{
		digitalWrite(ARD_RESET_TRIGGER,LOW);
		while (true) {}
	}
#endif
	// devise a watchdog like mechanism using TIMER0 -
//  watchdogcounter++;
//  if (watchdogcounter > (TEN_SECONDS*6))
//  {
//    Serial.println("watchdog");
//    digitalWrite(ARD_RESET_TRIGGER,LOW);
	//   while (true){}
//  }
}

/*
 *  You MUST provide the appropriate serialEvent function for the hardware serial
 *  port defined in the A6GPRS constructor.
 *  It simply copied all incoming traffic to the libraries circular buffer for later processing
 */
void serialEvent1() {
	// this gets called between loops, does no harm
	while (Serial1.available())
		gsm.push((char) Serial1.read());
}