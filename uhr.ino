#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

#include "RTClib.h"

RTC_DS3231 rtc;

enum Mode {
	normal,
	set_hour,
	set_minute,
};

#define NEXT 20;

static const char *mode_str[] = {'n','h','m'};

char* daysOfTheWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


//Vcc - Vcc
//Gnd - Gnd
//Din - Mosi (Pin 11)
//Cs  - SS (Pin 10)
//Clk - Sck (Pin 13)
const int pinLED = 14;
const int pinBTN = 15;

int i = 0;
int mode = Mode::normal;
int pressBtnDuration = 0;
int newHour = 0;
int newMinute = 0;
int nextCounter = NEXT;
bool justSwitched = false;
bool toggle = true;

char newTime[2];
String newTime_str = "00";

const int pinCS = 10;
const int numberOfHorizontalDisplays = 8;
const int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
const int wait = 50; // Velocidad a la que realiza el scroll
const int spacer = 1;
const int width = 5 + spacer; // Ancho de la fuente a 5 pixeles

void setup(){
   Serial.begin(9600);
   matrix. setIntensity ( 0 ) ;  // Adjust the brightness between 0 and 15
   matrix. setPosition ( 0 ,  0 ,  0 ) ;  // The first display is at <0, 0>
   matrix. setPosition ( 1 ,  1 ,  0 ) ;  // The second display is at <1, 0>
   matrix. setPosition ( 2 ,  2 ,  0 ) ;  // The third display is in <2, 0>
   matrix. setPosition ( 3 ,  3 ,  0 ) ;  // The fourth display is at <3, 0>
   matrix. setPosition ( 4 ,  4 ,  0 ) ;  // The fifth display is at <4, 0>
   matrix. setPosition ( 5 ,  5 ,  0 ) ;  // The sixth display is at <5, 0>
   matrix. setPosition ( 6 ,  6 ,  0 ) ;  // The seventh display is at <6, 0>
   matrix. setPosition ( 7 ,  7 ,  0 ) ;  // The eighth display is in <7, 0>
   matrix. setPosition ( 8 ,  8 ,  0 ) ;  // The ninth display is at <8, 0>
   matrix. setRotation ( 0 ,  1 ) ;		// Display position
   matrix. setRotation ( 1 ,  1 ) ;		// Display position
   matrix. setRotation ( 2 ,  1 ) ;		// Display position
   matrix. setRotation ( 3 ,  1 ) ;		// Display position
   matrix. setRotation ( 4 ,  1 ) ;		// Display position
   matrix. setRotation ( 5 ,  1 ) ;		// Display position
   matrix. setRotation ( 6 ,  1 ) ;		// Display position
   matrix. setRotation ( 7 ,  1 ) ;		// Display position
   matrix. setRotation ( 8 ,  1 ) ;		// Display position

   if (!rtc.begin()) {
  Serial.println("Couldn't find RTC");
  Serial.flush();
  abort();
}
	if (rtc.lostPower()) {
		Serial.println("RTC lost power, let's set the time!");

		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}
	pinMode(pinLED, OUTPUT);
	pinMode(pinBTN, INPUT_PULLUP);

}



void loop(){
	DateTime now = rtc.now();
	String a = "";
	char timenow[5];
	int second = now.second();

	newTime_str = String(newTime);

	if(now.hour() < 10){
		a += "0";
		a += String(now.hour());
	} else {
		a += String(now.hour());
	}

	if(now.minute() < 10){
		a += "0";
		a += String(now.minute());
	} else {
		a += String(now.minute());
	}
	matrix.fillScreen(LOW);

	switch (mode) {
		case Mode::normal:
			matrix.drawChar(0,0,a[0],1,1,1);
			matrix.drawChar(6,0,a[1],1,1,1);
			matrix.drawChar(14,0,a[2],1,1,1);
			matrix.drawChar(20,0,a[3],1,1,1);
			break;
		case Mode::set_hour:
			matrix.drawChar(0,0,newTime_str[0],1,1,1);
			matrix.drawChar(6,0,newTime_str[1],1,1,1);
			matrix.drawChar(14,0,a[2],1,1,1);
			matrix.drawChar(20,0,a[3],1,1,1);
			for (int i=0; i < 12; i++)
				matrix.drawPixel(i,7,1);
			break;
		case Mode::set_minute:
			matrix.drawChar(0,0,a[0],1,1,1);
			matrix.drawChar(6,0,a[1],1,1,1);
			matrix.drawChar(14,0,newTime_str[0],1,1,1);
			matrix.drawChar(20,0,newTime_str[1],1,1,1);
			for (int i=0; i < 12; i++)
				matrix.drawPixel(i+14,7,1);
			break;
	}

	if (second <= 30)
		matrix.drawPixel(second,7,1);
	else
		matrix.drawPixel(30 + (30-second),7,1);

	if(toggle)
		matrix.drawChar(10,0,':',1,1,1);
	matrix.write();

	if(i == 5)
		toggle == true ? toggle = false : toggle = true;

	int currentState = digitalRead(pinBTN);
	if(currentState == HIGH) {
		pressBtnDuration = 0;
		if (justSwitched == true)
			justSwitched = false;
	} else {

		nextCounter = NEXT;

		pressBtnDuration++;

		if (justSwitched == false) {
			switch (mode) {
				case Mode::set_hour:
					sprintf(newTime, "%02d", ++newHour);
					if (newHour == 23)
						newHour = -1;
					break;
				case Mode::set_minute:
					sprintf(newTime, "%02d", ++newMinute);
					if (newMinute == 59)
						newMinute = -1;
					break;
			}
		}
	}

	if (mode == Mode::normal && pressBtnDuration == 20) {
		justSwitched = true;
		nextCounter = NEXT;
		mode++;
		newHour = now.hour();
		sprintf(newTime, "%02d", newHour);
	}

	if (mode != Mode::normal && --nextCounter == 0) {

		switch (mode) {
			case Mode::set_hour:
				rtc.adjust(DateTime(now.year(), now.month(), now.day(), newHour, now.minute(), now.second()));
				newMinute = now.minute();
				sprintf(newTime, "%02d", newMinute);
				break;
			case Mode::set_minute:
				rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), newMinute, now.second()));
				break;
		}
		mode++;
		nextCounter = NEXT;
		if(mode > 2)
			mode=Mode::normal;
	}

Serial.println(nextCounter);

	if(i % 10 == 0)
		i=0;

	i++;
	delay(100);
}