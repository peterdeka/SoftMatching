/*
 * Women.h
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#ifndef WOMEN_H_
#define WOMEN_H_

#include "CTreeNode.h"

class Women{
	CTree *prefGraph;
	int *myInstance;
	int domains_size;
	void buildGraph(int numvars,float connectedness, char **varDomains);

public:
	Women(int numvars,float connectedness,int domains_sz,char **varDomains);
	void DOT_representation(string *res);
	int instance_pref(int *instance);
	int compare_instances(int *instance1,int *instance2);
};




#endif /* WOMEN_H_ */
