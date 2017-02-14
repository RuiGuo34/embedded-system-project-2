// Section 2: Energy-efficient real time scheduler
// Important! DO NOT MODIFY this file. You will not submit this file.
// This file is provided for your implementation of the program procedure.
// For more details, please see the instructions in the class website.

#include "governor.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SETFREQ_SYSCALL_NUM 391

static int maxFreq;
static int minFreq;
static int curFreq;

void init_userspace_governor() {
	char buf[32];

	FILE* fp;
	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "w");
	fprintf(fp, "userspace");
	fclose(fp);


	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");
	fscanf(fp, "%d", &maxFreq);
	fclose(fp);

	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", "r");
	fscanf(fp, "%d", &minFreq);
	fclose(fp);

	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", "r");
	fscanf(fp, "%d", &curFreq);
	fclose(fp);

	// Custom syscall test:
	// Instead of File IO, we use a custom system call to minimize FILE IO time.
	// Here, we test if it correct works in the custom kernel.
	set_by_min_freq();

	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
	fscanf(fp, "%d", &curFreq);
	fclose(fp);

	printf("MIN frequency change test: %d == %d\n", curFreq, minFreq);


	set_by_max_freq();

	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
	fscanf(fp, "%d", &curFreq);
	fclose(fp);

	printf("MAX frequency change test: %d == %d\n", curFreq, maxFreq);
}

void set_by_max_freq() {
	// Instead of File IO, we use a custom system call to minimize FILE IO time.
	syscall(SETFREQ_SYSCALL_NUM, 1);
	curFreq = maxFreq;
}


void set_by_min_freq() {
	// Instead of File IO, we use a custom system call to minimize FILE IO time.
	syscall(SETFREQ_SYSCALL_NUM, 0);
	curFreq = minFreq;
}

int get_cur_freq() { // We're assuming that there'e no other frequency change other than this governor control.
	return curFreq;
}
