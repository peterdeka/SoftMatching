/*
 * Profiler.h
 *
 *  Created on: 04/nov/2013
 *      Author: deka
 */

#ifndef PROFILER_H_
#define PROFILER_H_
#include <time.h>
class Profiler {
	timespec tstart,tend;
	timespec diff(timespec start, timespec end);
public:
	Profiler();
	virtual ~Profiler();
	void start();
	void stop(timespec *tout);

};

#endif /* PROFILER_H_ */
