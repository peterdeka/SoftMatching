/*
 * FemaleContainer.h
 *
 *  Created on: Nov 11, 2013
 *      Author: deka
 */

#ifndef FEMALECONTAINER_H_
#define FEMALECONTAINER_H_
#include "Female.h"
#include <tr1/unordered_map>
#include <cstring>

class FemaleContainer {
	tr1::unordered_map<string, int> hashtable;
	int nfemales;
	int nvars;
	void stringify_instance(int* inst, int nvars, char* s_out);

public:
	FemaleContainer(Female **females, int nfemales);
	virtual ~FemaleContainer();
	int find_female_with_instance(int* instance);

};

#endif /* FEMALECONTAINER_H_ */
