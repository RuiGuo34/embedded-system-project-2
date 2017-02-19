#include "assignment1.h"
#include "assignment2.h"
#include "workload.h"
#include "scheduler.h"
#include "governor.h"
#include <limits.h>
#include <float.h>

// Note: Deadline of each workload is defined in the "workloadDeadlines" variable.
// i.e., You can access the dealine of the BUTTON thread using workloadDeadlines[BUTTON]
// See also deadlines.c and workload.h

// Assignment: You need to implement the following two functions.

// learn_workloads(SharedVariable* v):
// This function is called at the start part of the program before actual scheduling
// - Parameters
// sv: The variable which is shared for every function over all threads

typedef void* (*thread_function_t)(void*);

int optimized_freq[] = {1,1,1,1,1,1,1,1};

long long currentDeadlines[] = {0,0,0,0,0,0,0,0};

thread_function_t functions[] = {&thread_button, &thread_twocolor, &thread_temp,
&thread_track, &thread_touch, &thread_rgbcolor,&thread_aled, &thread_buzzer };

long long energy = 0;

int P_WORK[] = {450,1050};

int sum(int *a) {
	unsigned int i = 0, s = 0;
	for (;i<NUM_TASKS;i++) {
		s+=a[i];
	}
	return s;
}

void printFreq(int *freq){
    int i = 0;
    printDBG("Frequency Preference: ");
    for(;i<NUM_TASKS;i++){
        printDBG("%d ", freq[i]);        
    }
    printDBG("\n");    
}

float calculate_utilization(int *optimized_freq, long long *w_1200, long long *w_600, long long *deadlines) {
	float sum = 0.0;
	int i = 0;
	for (;i<NUM_TASKS;i++) {
		if (optimized_freq[i] == 1) {
			sum += ((float)w_1200[i])/deadlines[i];
		}
		else {
			sum += ((float)w_600[i])/deadlines[i];
		}
	}
	return sum;
}

void learn_workloads(SharedVariable* sv) {
	long long start, end;
	long long workloads_1200[] = {0,0,0,0,0,0,0,0};
	long long workloads_600[] = {0,0,0,0,0,0,0,0};
	int Power_1200 = 1050;
	int Power_600 = 450;

	for (unsigned int i = 0; i < NUM_TASKS; i++) {
		//first test with max cpu freq
		set_by_max_freq();
		start = get_current_time_us();
		(*(functions[i]))(sv); 
		end = get_current_time_us();

		workloads_1200[i] = end-start;

		//then test with min cpu freq
		set_by_min_freq();
		start = get_current_time_us(); 
		(*(functions[i]))(sv);
		end = get_current_time_us();
		workloads_600[i] = end-start;

		currentDeadlines[i] = workloadDeadlines[i];

	}

	for (unsigned int i = 0; i < NUM_TASKS; i++) {
		//calculate the approximate power
		if (workloads_1200[i]*Power_1200 > workloads_600[i]*Power_600) {
			optimized_freq[i] = 0;
		}
	}

	// //printFreq(optimized_freq);

	// // check schedulity
	// int idx = -1;
	// float util[8] = {0,0,0,0,0,0,0,0};
	// float u = 2;
	// //1.0 is LIMIT
	// printDBG("num tasks %d\n", NUM_TASKS);
	// while (u > 1.0 && sum(optimized_freq) != NUM_TASKS) {
	// 	idx = -1;
	// 	for (unsigned int i = 0; i < NUM_TASKS; i++) {
	// 		if (optimized_freq[i] == 0) {
	// 			optimized_freq[i] = 1;
	// 			float update = 0.0;
				
	// 			util[i] = calculate_utilization(optimized_freq,workloads_1200,workloads_600,workloadDeadlines);
	// 			optimized_freq[i] = 0;
	// 		}
	// 	}

	// 	//find the max index that is less than one 
	// 	float min = FLT_MIN;
	// 	for (unsigned i = 0; i < sizeof(util); i++) {
	// 		if (util[i] < 1.0 && util[i] > min) {
	// 			min = util[i];
	// 			idx = i;
	// 		}
	// 	}
		
	// 	//idx is -1, just change to the max index;
	// 	if (idx == -1) {
	// 		min = FLT_MIN;
	// 		for (unsigned i = 0; i < sizeof(util); i++){
	// 			if (util[i] > min) {
	// 				min = util[i];
	// 				idx = i;
	// 			}
	// 		}
	// 	}

	// 	//clear util array
	// 	for (unsigned i = 0; i < sizeof(util); i++){
	// 		util[i] = 0.0;
	// 	}
 
	// 	optimized_freq[idx] = 1; 
	// 	u = calculate_utilization(optimized_freq,workloads_1200,workloads_600,workloadDeadlines);
	// 	printDBG("util %f \n",u);
	// }
	printDBG("finished with \n");
	printFreq(optimized_freq);
	printDBG("\n");

	// TODO: Fill the body
	// This function is executed before the scheduling simulation.
	// You need to calculate the execution time of each thread here.

	// Thread functions for workloads: 
	// thread_button, thread_twocolor, thread_temp, thread_track,
	// thread_touch, thread_rgbcolor, thread_aled, thread_buzzer

	// Tip 1. You can call each workload function here like:
	// thread_button();

	// Tip 2. You can get the current time here like:
	// long long curTime = get_current_time_us();
}


// select_task(SharedVariable* sv, const int* aliveTasks):
// This function is called while runnning the actual scheduler
// - Parameters
// sv: The variable which is shared for every function over all threads
// aliveTasks: an array where each element indicates whether the corresponed task is alive(1) or not(0).
// idleTime: a time duration in microsecond. You can know how much time was waiting without any workload
//           (i.e., it's larger than 0 only when all threads are finished and not reache the next preiod.)
// - Return value
// TaskSelection structure which indicates the scheduled task and the CPU frequency

int lastAliveTasks[] = {0,0,0,0,0,0,0,0};

void updateCurrentDeadLines(long long time_difference, int* lastAliveTasks, const int* aliveTasks, long long idleTime) {
	int i = 0;
	int x;
	for (; i < NUM_TASKS; i++) {
		if (aliveTasks[i] == 1) {
			if (lastAliveTasks[i] == 1 && idleTime == 0) {
				x = currentDeadlines[i] - time_difference;
				currentDeadlines[i] = x > 0 ? x:workloadDeadlines[i];
			}
			else {
				currentDeadlines[i] = workloadDeadlines[i];
			}
		}
		else {
			currentDeadlines[i] = 0;
		}
	}
}

int chooseTask(long long *currentDeadlines, const int* aliveTasks) {
	long long minDead = LONG_MAX;
	int taskIdx = -1, i = 0;
	for (; i < NUM_TASKS; i++) {
		if (aliveTasks[i] == 1) {
			if (currentDeadlines[i] < minDead) {
				minDead = currentDeadlines[i];
				taskIdx = i;
			}
		}
	}
	return taskIdx;
}

int chooseFreq(int i) {
	return optimized_freq[i];
}

void updateLastAliveTasks(const int* aliveTasks) {
	int i = 0;
	for (; i < NUM_TASKS; i++) {
		lastAliveTasks[i] = *(aliveTasks+i);
	}
}
long long totalIdleTime = 0; 

void printTask(TaskSelection t){
    printDBG("Task  id:%d, freq:%d", t.task, t.freq);
    printDBG("   ::::   ");
}

void printTasks(const int *aliveTasks){
    int i = 0;
    printDBG("Current Alive Tasks:  ");
    for(;i<8;i++){
        printDBG("%d ", *(aliveTasks+i));
    }
    printDBG(" :::::: ");
}

void printDeadlines(){
    int i = 0;
    printDBG("Current Deadline: ");
    for(;i<NUM_TASKS;i++){
        printDBG("");
        printDBG("%lld  ",currentDeadlines[i]);
    }
    printDBG("  :::::  ");
}

TaskSelection select_task(SharedVariable* sv, const int* aliveTasks, long long idleTime) {
	totalIdleTime += idleTime;
	// printDBG("current idleTime is %lld\n", idleTime);
	// printDBG("Total idleTime is %lld\n", totalIdleTime);

	static int prev_freq = 1;
	static long long prev_timestamp = -1;
	long long curr_timestamp = get_scheduler_elapsed_time_us();
	long long time_difference = 0;

	if (prev_timestamp != -1) {
		time_difference = curr_timestamp - prev_timestamp;
	}
	prev_timestamp = curr_timestamp;

	updateCurrentDeadLines(time_difference, lastAliveTasks, aliveTasks, idleTime);

	TaskSelection sel;
	sel.task = chooseTask(currentDeadlines, aliveTasks);
	sel.freq = chooseFreq(sel.task);

	prev_freq = sel.freq;

	energy = energy + ((float)idleTime/1000000)*50 + ((float)time_difference/1000000)*P_WORK[prev_freq];
	// printDBG("Energy: %lld\n", energy);
	// printDBG("Time Difference %lld\n", time_difference);
	// printTasks(aliveTasks);
	// printTask(sel);
	// printDeadlines();
	// printDBG("idleTime is %lld\n", idleTime);

	updateLastAliveTasks(aliveTasks);
	
    return sel;
}