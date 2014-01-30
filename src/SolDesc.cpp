/*
 * SolDesc.cpp
 *
 *  Created on: Jan 29, 2014
 *      Author: deka
 */

#include "SolDesc.h"

SolDesc::SolDesc() {
	// TODO Auto-generated constructor stub
	sol=(int*)malloc(sizeof(int)*NUMVARS);
}

SolDesc::~SolDesc() {
	// TODO Auto-generated destructor stub
	//free(sol);
}

