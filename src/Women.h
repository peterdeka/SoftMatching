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
	int myInstance[];
	int domains_size;

public:
	Women(int numvars,int domains_sz,char **varDomains);
	void buildGraph(float connectedness, char **varDomains);
	void DOT_representation(string *res);
};




#endif /* WOMEN_H_ */
