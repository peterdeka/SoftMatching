/*
 * SMproblem.h
 *
 *  Created on: 16/ott/2013
 *      Author: deka
 */

#ifndef SMPROBLEM_H_
#define SMPROBLEM_H_

#import "Male.h"
#import "Female.h"

#define NUM_INDIVIDUALS 10
#define NUMVARS 10	//numero variabili (quindi nodi dell'albero)
#define DOMAINS_SIZE 2	//dimensione dei domini delle variabili

#define MALE_TIGHTNESS 0.8	//percentuale di binary constraint NON nulli

class SM_problem {
	Male *men[NUM_INDIVIDUALS];
	Female *women[NUM_INDIVIDUALS];
	int men_matches[NUM_INDIVIDUALS];
	char **varDomains;//ogni varId è associato al suo dominio (che è un array di valori)
	void gen_random_instance(int *instance);
	void buildVarDomains();

public:
	SM_problem();
	virtual ~SM_problem();
	bool verify_is_weakstable();
};

#endif /* SMPROBLEM_H_ */
