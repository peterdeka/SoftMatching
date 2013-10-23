/*
 * Male.h
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#ifndef MALE_H_
#define MALE_H_
#include "CTree.h"
#include "Female.h"

class Male{
	CTree *prefTree;
	int domains_size;
	int numvars;
	float myOpt;
	int *myOptInstance;
	void buildTree(float tightness,int numvars,char **varDomains);
	void adjustTightness(float tightness);
	void make_DAC();
	void DAC_first_pass(CTreeNode *node);
	float DAC_opt(int *opt_instance);
	void opt_as_child(CTreeNode *node,int *opt_instance,int *curidx);
	void opt_as_father(CTreeNode *node,int *opt_instance,int *curidx);
	bool CSP_next(int *instance, float cutval,int *nextinstance);	//risolve come un problema di HCSP applicando il cut a cutval


	public:
	int *myInstance;
	Male(int numvars,int domains_size,float tightness,char ** varDomains, int *instance);
	//int opt(Female **women,int n_women);
	void DOT_representation(string *res);
	bool SOFT_next(Female *curfemale,int *nextinstance);
	float pref(Female *f);

};

#endif /* MALE_H_ */
