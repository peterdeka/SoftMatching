/*
 * SMproblem.h
 *
 *  Created on: 16/ott/2013
 *      Author: deka
 */

#ifndef SMPROBLEM_H_
#define SMPROBLEM_H_

#include "Male.h"
#include "Female.h"
#include "ClassicGS.h"
#include "SoftGS.h"
#include "ClassicGSNext.h"


#define NUM_INDIVIDUALS  7776
#define NUMVARS 5//numero variabili (quindi nodi dell'albero)
#define DOMAINS_SIZE 6	//dimensione dei domini delle variabili
#define WOMEN_CONNECTEDNESS 0.5
#define MALE_TIGHTNESS 0.1	//percentuale di binary constraint NON nulli

class SM_problem {
	Male **men;
	Female **women;
	int men_matches[NUM_INDIVIDUALS];
	char **varDomains;//ogni varId � associato al suo dominio (che � un array di valori)
	void gen_random_instance(int *instance);
	void buildVarDomains();
	void print_arr(int *inst,int length);

public:
	SM_problem();
	virtual ~SM_problem();
	int domsz=DOMAINS_SIZE;
	int numvars=NUMVARS;

	bool verify_is_weakstable();
	int solve_with_classicGS();
	int solve_with_softGS();
	int solve_with_classicGSNext();
	void debugTrees(char *filename);
};

#endif /* SMPROBLEM_H_ */
