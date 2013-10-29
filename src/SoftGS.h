/*
 * SoftGS.h
 *
 *  Created on: 19/ott/2013
 *      Author: deka
 */

#ifndef SOFTGS_H_
#define SOFTGS_H_

#include "Male.h"
#include "Female.h"
class SoftGS {
	int num_individuals;
	Male **men;
	Female **women;
	int find_female_with_instance(int *instance);
	void print_arr(int *inst,int length);

public:
	SoftGS(int num_males, Male** menarray, Female** womenarray);
	virtual ~SoftGS();
	void gale_shapley_men_opt(int *matching);
};

#endif /* SOFTGS_H_ */
