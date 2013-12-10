/*
 * SoftGS.h
 *
 *  Created on: 19/ott/2013
 *      Author: deka
 *  GS basato su alberi con soft constraints, obiettivo della ricerca
 */

#ifndef SOFTGS_H_
#define SOFTGS_H_

#include "Male.h"
#include "Female.h"
#include "FemaleContainer.h"

class SoftGS {
	int num_individuals;
	Male **men;
	Female **women;
	FemaleContainer *womencont;
	int find_female_with_instance(int *instance);
	void print_arr(int *inst,int length);

public:
	SoftGS(int num_males, Male** menarray, Female** womenarray);
	virtual ~SoftGS();
	int gale_shapley_men_opt_next1(int *matching);
	int test23();
	bool test_soft_next();
};

#endif /* SOFTGS_H_ */
