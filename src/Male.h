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
	struct Tuple {
		        int var_idx;
		        int child_idx;
		        int idx_in_bintbl[2];
		    };

	CTree *prefTree;
	int domains_size;
	int numvars;
	float myOpt;
	int *myOptInstance;
	float **fixed_tuple_childconstr;		//mi serve quando faccio il fix, tavola di servizio per evitare malloc ogni volta
	float **fixedtuple_backup;			//solo puntatore di backup per ripristinare fixed tuple
	void buildTree(float tightness,int numvars,char **varDomains);
	void adjustTightness(float tightness);
	void make_DAC();
	void DAC_first_pass(CTreeNode *node);
	float DAC_opt(int *opt_instance);
	void opt_as_child(CTreeNode *node,int *opt_instance,int *curidx);
	void opt_as_father(CTreeNode *node,int *opt_instance,int *curidx);
	bool CSP_next(int *instance, float cutval,int *nextinstance);	//risolve come un problema di HCSP applicando il cut a cutval
	void find_first_tuple_with_pref(int* instance, float pref, Tuple *tuple);
	void fix(Tuple *fixtuple);
	void unfix(Tuple *fixtuple);
	bool next_tuple_with_pref(Tuple *tin, Tuple *tout, float pref);
	float find_next_pref_level(float curpref);
	bool Male::CSP_solve_arc_consist(CTreeNode *node, float cutval);
	float Male::CSP_solve(float cutval, int *solution);

	public:
	int *myInstance;
	Male(int numvars,int domains_size,float tightness,char ** varDomains, int *instance);
	//int opt(Female **women,int n_women);
	void DOT_representation(string *res);
	bool SOFT_next(Female *curfemale,int *nextinstance);
	float pref(Female *f);

};

#endif /* MALE_H_ */
