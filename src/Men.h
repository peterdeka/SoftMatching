/*
 * Men.h
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#ifndef MEN_H_
#define MEN_H_
#include "CTree.h"

class Men{
	CTree *prefTree;
	int myInstance[];
	int domains_size;
	void buildTree(float tightness,int numvars,char **varDomains);
	void adjustTightness(float tightness);
	void DAC_first_pass(CTreeNode *node);
	void opt_as_child(CTreeNode *node,int *opt_instance,int *curidx);
	void opt_as_father(CTreeNode *node,int *opt_instance,int *curidx);

	public:
	Men(int numvars,int domains_size,float tightness,char ** varDomains);
	void DAC();
	float DAC_opt(int *opt_instance,int *curidx);
	void DOT_representation(string *res);
};

#endif /* MEN_H_ */
