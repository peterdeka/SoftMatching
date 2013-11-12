/*
 * Female.h
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#ifndef FEMALE_H_
#define FEMALE_H_

#include "CTreeNode.h"
#include "Male.h"

class Male;
class Female{
	CTree *prefGraph;

	int domains_size;
	void buildGraph(int numvars,float connectedness, char **varDomains);

public:
	int *myInstance;
	int numvars;
	Female();
	Female(int numvars,float connectedness,int domains_sz,char **varDomains,int *instance);
	virtual ~Female();
	void DOT_representation(string *res);
	float instance_pref(int *instance);
	int compare(Male *m1, Male *m2);
};




#endif /* FEMALE_H_ */
