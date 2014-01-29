/*
 * GSLists.h
 *
 *  Created on: Jan 20, 2014
 *      Author: deka
 */

#ifndef GSLISTS_H_
#define GSLISTS_H_

#include "Male.h"
#include "Female.h"
#include "SMproblem.h"
#include "FemaleContainer.h"
#include "SolDesc.h"
#include <map>
#include <list>
class SolDesc;
class GSLists {

	int num_individuals;
	int linearization;
	Male **men;
	Female **women;
	FemaleContainer *womencont;
	int **maleprefs;
	int **femaleprefs;

	void gen_male_preflist(Male *m, int idx);
	void gen_female_preflist(Female *f, int idx);
	void add_to_map(map<float,list< SolDesc*>*> &m,Male *mm,int *s);
	void add_to_list(list< SolDesc*> &l, int *s,float pref,Male *m);
	int lex_precedes(int *a, int *b);
	void count_tuples_change(Male *m,int *s,float pref,SolDesc *sd);
	int gale_shapley_men_opt(int *matching);
	void print_arr(int *inst,int length);
public:
	GSLists(int num_males, Male** menarray, Female** womenarray,int lineariz);
	int solve_GS(int *matching);
	virtual ~GSLists();
};

#endif /* GSLISTS_H_ */
