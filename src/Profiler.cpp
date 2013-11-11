/*
 * Profiler.cpp
 *
 *  Created on: 04/nov/2013
 *      Author: deka
 */

#include "Profiler.h"

Profiler::Profiler() {
	// TODO Auto-generated constructor stub

}

Profiler::~Profiler() {
	// TODO Auto-generated destructor stub
}

void Profiler::start(){
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tstart);
}

void Profiler::stop(timespec *tout){
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tend);
	tout->tv_sec=diff(tstart,tend).tv_sec;
	tout->tv_nsec=diff(tstart,tend).tv_nsec;
}

timespec Profiler::diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
