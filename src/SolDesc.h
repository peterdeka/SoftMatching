/*
 * SolDesc.h
 *
 *  Created on: Jan 29, 2014
 *      Author: deka
 */

#ifndef SOLDESC_H_
#define SOLDESC_H_
#include "SMproblem.h"

class SolDesc {
public:
	Male::Tuple t_gen;	//posizione tupla che genera la soluzione
	int n_t_change;	//numero di tuple da cambiare per raggiungere opt
	float q_t_change;	//di quanto devono essere cambiate le tuple per raggiungere opt
	int *sol;
	SolDesc();
	virtual ~SolDesc();
};

#endif /* SOLDESC_H_ */
