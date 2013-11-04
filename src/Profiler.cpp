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
	tstart=clock();
}

double Profiler::stop(){
	clock_t tend = clock() ;
	return (tend-tstart)/(double)CLOCKS_PER_SEC ;
}
