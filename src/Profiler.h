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
	clock_t tstart;
public:
	Profiler();
	virtual ~Profiler();
	void start();
	double stop();
};

#endif /* PROFILER_H_ */
