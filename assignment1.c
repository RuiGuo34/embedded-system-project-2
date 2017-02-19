#include "assignment1.h"
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <stdint.h>

void init_shared_variable(SharedVariable* sv) {
	// You can initialize the shared variable if needed.
	sv->bProgramExit = 0;
	sv->state = 0; //initial state
	sv->temp = 0;
	sv->touch = 0;
	sv->track = 0;
	sv->prev = 0;
	sv->touch_buz = 0;
}

void init_sensors(SharedVariable* sv) {
	pinMode(PIN_YELLOW,OUTPUT);
	pinMode(PIN_BLUE,OUTPUT);
	pinMode(PIN_ALED,OUTPUT);
	pinMode(PIN_BUZZER,OUTPUT);
	
	pinMode(PIN_BUTTON,INPUT);
	pinMode(PIN_TEMP,INPUT);
	pinMode(PIN_TRACK,INPUT);
	pinMode(PIN_TOUCH,INPUT);
	
	softPwmCreate(PIN_BLUE,0,0xff);
	softPwmCreate(PIN_RED,0,0xff);
	softPwmCreate(PIN_GREEN,0,0xff);
}

void body_button(SharedVariable* sv) {
	int button = digitalRead(PIN_BUTTON);
	
	if (button == 1 && sv->prev == 0) {
		if (sv->state == 1) init_shared_variable(sv);
		else sv->state = 1;
	}
	sv->prev = button;
}

void body_twocolor(SharedVariable* sv) {
	
	if (sv->state == 1) digitalWrite(PIN_YELLOW,1);
	else digitalWrite(PIN_YELLOW,0);
}

void body_temp(SharedVariable* sv) {
	int temperature = digitalRead(PIN_TEMP);
	
	if (sv->state == 1) {
		if (temperature && sv->touch == 0 && sv->track == 0) {
			sv->temp = 1;
		}
		else {
			sv->temp = 0;
		}
	}
}

void body_track(SharedVariable* sv) {
	int track = digitalRead(PIN_TRACK);
	
	if (sv->state == 1) {
		if (track == 0 && sv->touch == 0) {
			sv->track = 1;
		}
	}
}

void body_touch(SharedVariable* sv) {
	int touch = digitalRead(PIN_TOUCH);
	
	if (sv->state == 1) {
		if (touch == 1 && sv->track == 0) {
			sv->touch = 1;
			sv->touch_buz = 1;
		}
		else {
			sv->touch_buz = 0;
		}
	}
}

void body_rgbcolor(SharedVariable* sv) {
	if (sv->state == 0) {
		softPwmWrite(PIN_RED,0);
		softPwmWrite(PIN_GREEN,0);
		softPwmWrite(PIN_BLUE,0xff);
		return;
	}
	if (sv->touch) {
		softPwmWrite(PIN_RED,0xc8);
		softPwmWrite(PIN_GREEN,0x3b);
		softPwmWrite(PIN_BLUE,0xff);
		return;
	}
	if (sv->track) {
		softPwmWrite(PIN_RED,0xff);
		softPwmWrite(PIN_GREEN,0);
		softPwmWrite(PIN_BLUE,0);
		return;
	}
	softPwmWrite(PIN_RED,0xff);
	softPwmWrite(PIN_GREEN,0xff);
	softPwmWrite(PIN_BLUE,0);
	return;
}

void body_aled(SharedVariable* sv) {
	
	if (sv->temp == 1) {
		digitalWrite(PIN_ALED,1);
	}
	else {
		digitalWrite(PIN_ALED,0);
	}
}

void body_buzzer(SharedVariable* sv) {
	if (sv->touch_buz == 1 && sv->track == 0) {
		digitalWrite(PIN_BUZZER,1);
	}
	else {
		digitalWrite(PIN_BUZZER,0);
	}
}