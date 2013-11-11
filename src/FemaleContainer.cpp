/*
 * FemaleContainer.cpp
 *
 *  Created on: Nov 11, 2013
 *      Author: deka
 */

#include "FemaleContainer.h"

FemaleContainer::FemaleContainer(Female **females, int nfemales) {
	cout<<"initializing hashtable\n";
	nvars=females[0]->numvars;
	this->nfemales=nfemales;
	char tmp[nvars+3];
	for(int i=0;i<nfemales;i++){
		stringify_instance(females[i]->myInstance,nvars,tmp);
		string inststr(tmp);
		hashtable[inststr]=i;

	}
	cout<<"Container initialized\n";
}

FemaleContainer::~FemaleContainer() {
	// TODO Auto-generated destructor stub
}

int FemaleContainer::find_female_with_instance(int* instance){
	char tmp[nvars+3];
	stringify_instance(instance,nvars,tmp);
	string inststr(tmp);
	return hashtable[inststr];
}

void FemaleContainer::stringify_instance(int* inst, int nvars, char* s_out){
	sprintf(s_out,"%d",inst[0]);
	char t[3];
	for(int i=1;i<nvars;i++){
		sprintf(t,"%d",inst[i]);
		strcat(s_out,t);
	}
}
