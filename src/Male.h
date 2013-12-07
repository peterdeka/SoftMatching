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
#include <fstream>
#include <cstdlib>
#include<cstring>
#include<cmath>
#define NEXT2 0;
#define NEXT3 1;
class Female;
class Male{
	struct Tuple {
		        int var_idx;
		        int child_idx;
		        int idx_in_bintbl[2];
		    };


	int domains_size;



	float **fixed_tuple_childconstr;		//mi serve quando faccio il fix, tavola di servizio per evitare malloc ogni volta
	float **fixedtuple_backup;			//solo puntatore di backup per ripristinare fixed tuple
	Tuple *zeroed_tuples_backup;		//tengo traccia delle tuple azzerate
	float zeroed_pref;

	void buildTree(float tightness,int numvars,char **varDomains);
	void adjustTightness(float tightness);
	void make_DAC();
	void DAC_first_pass(CTreeNode *node);
	float DAC_opt(int *opt_instance);
	bool CSP_next(int *instance, float cutval,int *nextinstance);	//risolve come un problema di HCSP applicando il cut a cutval
	bool find_first_tuple_with_pref(int* instance, float pref, Tuple *tuple);
	void fix(Tuple *fixtuple);
	void unfix(Tuple *fixtuple);
	bool next_tuple_with_pref(Tuple tin, Tuple *tout, float pref);
	float find_next_pref_level(float curpref);
	bool CSP_solve_arc_consist(CTreeNode *node, float cutval);
	float CSP_solve(float cutval, int *solution);
	void zeroout_prectuples_with_pref(Tuple *t_star,float pref);

	void zeroout_tuple(Tuple *t);
	void set_solution(int *instance);
	void print_arr(int *inst,int length);
	int elim_m_opt(int m, int **solutions, int widx ); //dechter m bucket elimination to get m opt from WCSP, assumes all unary are 0
	void elim_m_opt_rec(CTreeNode *node,int m);
	void fuzzy_to_weighted(int linearization,float opt, float pl);	//inserisce le informazioni per operare come WCSP
	int k_cheapest(int k, int linearization, int **solutions);	//rossi-pini-venable
	void merge_messages(int *m1, int *m2, int* dst);

public:
	CTree *prefTree;
	int *myInstance;
	int *myOptInstance;
	float myOpt;
	int numvars;
	int n_zeroed_tuples;
	Male(int numvars,int domains_size,float tightness,char ** varDomains, int *instance);
	virtual ~Male();
	//int opt(Female **women,int n_women);
	void DOT_representation(string *res);
	bool SOFT_next(Female *curfemale,int *nextinstance);
	float pref(Female *f);
	void debugTree(char* fname);
	int compare(Female *f1, Female *f2);
	void reset_zeroed_prectuples();
	void kCheapest(int dist,int k, int **solutions,int *nsolutions);
};

#endif /* MALE_H_ */
