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

typedef void* (*thread_process)(void*);

int optimized_freq[] = {1,1,1,1,1,1,1,1};

long long currentDeadlines[] = {0,0,0,0,0,0,0,0};

thread_process functions[] = {&thread_button, &thread_twocolor, &thread_temp,
&thread_track, &thread_touch, &thread_rgbcolor,&thread_aled, &thread_buzzer };

long long energy = 0;

int CPU_WORK[] = {450,1050};

long long totalIdleTime = 0;

int sum(int *a) {
	unsigned int sum = 0;
	for (unsigned int i = 0;i<8;i++) {
		sum+=a[i];
	}
	return sum;
}

// void printFreq(int *freq){
//     for(unsigned int i = 0;i<8;i++){
//         printDBG("%d ", freq[i]);        
//     }
//     printDBG("\n");    
// }



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
		// printDBG("wordload for 1200, task %d", i);
		// printDBG(" %lld ",workloads_1200[i]);
		// printDBG("\n");
		//then test with min cpu freq
		set_by_min_freq();
		start = get_current_time_us(); 
		(*(functions[i]))(sv);
		end = get_current_time_us();
		workloads_600[i] = end-start;
		// printDBG("wordload for 600, task %d", i);
		// printDBG(" %lld ",workloads_600[i]);
		// printDBG("\n");
		currentDeadlines[i] = workloadDeadlines[i];
	}

	for (unsigned int i = 0; i < 8; i++) {
		//calculate the approximate power
		if (workloads_1200[i]*Power_1200 > workloads_600[i]*Power_600) {
			optimized_freq[i] = 0;
		}
	}

	// check schedulity
	int index;
	float util[8] = {0,0,0,0,0,0,0,0};
	float u = FLT_MAX;
	//1.0 is LIMIT
	//break loop condition: whether u is smaller than 1 and all freq set to HIGH
	while (u > 1.0 && sum(optimized_freq) != 8) {
		index = -1;
		//switch one to max can calculate the utilization.
		for (unsigned int i = 0; i < 8; i++) {
			if (optimized_freq[i] == 0) {
				optimized_freq[i] = 1;
				float sum = 0.0;
				for (unsigned k = 0;k<8;k++) {
					if (optimized_freq[k] == 1) {
						sum += ((float)workloads_1200[k])/currentDeadlines[k];
					}
					else {
						sum += ((float)workloads_600[k])/currentDeadlines[k];
					}
				}
				printDBG("%f", sum);
				util[i] = sum;
				optimized_freq[i] = 0;
			}
		}

		//find the max index that is less than one 
		float min = FLT_MIN;
		for (unsigned i = 0; i < 8; i++) {
			if (util[i] < 1.0 && util[i] > min) {
				min = util[i];
				index = i;
			}
		}
		
		//index is -1, just change to the max index;
		if (index == -1) {
			for (unsigned i = 0; i < 8; i++){
				if (util[i] > min && optimized_freq[i] == 0) {
					min = util[i];
					index = i;
				}
			}
		}

		//clear util array
		for (unsigned i = 0; i < 8; i++){
			util[i] = 0.0;
		}

		optimized_freq[index] = 1; 

		float sum = 0.0;
		for (unsigned k = 0;k<8;k++) {
			if (optimized_freq[k] == 1) {
				sum += ((float)workloads_1200[k])/currentDeadlines[k];
			}
			else {
				sum += ((float)workloads_600[k])/currentDeadlines[k];
			}
		}
		u = sum;
		// printDBG("util %f \n",u);
	}
	// printDBG("the finalized freq choice is\n");
	// printFreq(optimized_freq);
	// printDBG("\n");

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


int chooseTask(long long *currentDeadlines, const int* aliveTasks) {
	long long minDead = LONG_MAX;
	int taskindex = -1, i = 0;
	for (; i < NUM_TASKS; i++) {
		if (aliveTasks[i] == 1) {
			if (currentDeadlines[i] < minDead) {
				minDead = currentDeadlines[i];
				taskindex = i;
			}
		}
	}
	return taskindex;
}

int chooseFreq(int i) {
	return optimized_freq[i];
}

// void printTask(TaskSelection t){
//     printDBG("Task id:%d, Task freq:%d", t.task, t.freq);
//     printDBG("\n");
// }

// void printTasks(const int *aliveTasks){
//     printDBG("Current Alive Tasks:  ");
//     for(unsigned int i = 0;i<8;i++){
//         printDBG("%d ", *(aliveTasks+i));
//     }
//     printDBG("\n");
// }

// void printDeadlines(){
//     printDBG("Current Deadline: ");
//     for(unsigned int i = 0;i<NUM_TASKS;i++){
//         printDBG("");
//         printDBG("%lld  ",currentDeadlines[i]);
//     }
//     printDBG("\n");
// }

TaskSelection select_task(SharedVariable* sv, const int* aliveTasks, long long idleTime) {
	totalIdleTime += idleTime;
	// printDBG("current idleTime is %lld\n", idleTime);
	// printDBG("Total idleTime is %lld\n", totalIdleTime);

	static int prev_freq;
	//initialize time line
	static long long prev_timestamp = -1;
	long long curr_timestamp = get_scheduler_elapsed_time_us();
	long long time_difference = 0;

	if (prev_timestamp != -1) {
		time_difference = curr_timestamp - prev_timestamp;
	}
	prev_timestamp = curr_timestamp;
	//update current deadlines
	int diff = 0;
	for (unsigned i = 0; i < NUM_TASKS; i++) {
		if (aliveTasks[i] == 1) {
			if (lastAliveTasks[i] == 1 && idleTime == 0) {
				diff = currentDeadlines[i] - time_difference;
				currentDeadlines[i] = diff > 0 ? diff:workloadDeadlines[i];
			}
			else {
				currentDeadlines[i] = workloadDeadlines[i];
			}
		}
		else {
			currentDeadlines[i] = 0;
		}
	}

	TaskSelection res;
	res.task = chooseTask(currentDeadlines, aliveTasks);
	res.freq = chooseFreq(res.task);

	prev_freq = res.freq;

	energy = energy + ((float)idleTime/1000000)*50 + ((float)time_difference/1000000)*CPU_WORK[prev_freq];
	// printDBG("Energy: %lld\n", energy);
	// printDBG("Time Difference %lld\n", time_difference);
	// printTasks(aliveTasks);
	// printTask(sel);
	// printDeadlines();
	// printDBG("idleTime is %lld\n", idleTime);
	// update current alive tasks
	for (unsigned int i = 0; i < 8; i++) {
		lastAliveTasks[i] = *(aliveTasks+i);
	}
	
    return res;
}