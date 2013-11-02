/*
 * SMproblem.cpp
 *
 *  Created on: 16/ott/2013
 *      Author: deka
 */

#include "SMproblem.h"
//#include <fstream>

SM_problem::SM_problem() {
	srand((unsigned)time(0));
	buildVarDomains();
	men=(Male**)malloc(NUM_INDIVIDUALS*sizeof(Male*));
	women=(Female**)malloc(NUM_INDIVIDUALS*sizeof(Female*));
	for(int i=0;i<NUM_INDIVIDUALS;i++){
		int instvals[NUMVARS];
		gen_random_instance(instvals);
		men[i]= new Male(NUMVARS,DOMAINS_SIZE,MALE_TIGHTNESS,varDomains,instvals);
		//********GENERAZIONE RANDOM DELLE DONNE (NON TUTTE)
		//gen_random_instance(instvals);
		//women[i]= new Female(NUMVARS,WOMEN_CONNECTEDNESS,DOMAINS_SIZE,varDomains,instvals);
		men_matches[i]=-1;
		cout << "*****men "<<i<<" generated. opt:"<<men[i]->myOpt<<" optinst:";
		print_arr(men[i]->myOptInstance,NUMVARS);
	}

	/*string st;
	men[0]->DOT_representation(&st);
	ofstream myfile;
	myfile.open ("Mgraph.gv");
	myfile << st;
	myfile.close();
*/
	//GENERAZIONE NON RANDOM DELLE DONNE (TUTTE), BISOGNA ADEGUARE NUM_INDIVIDUALS PERCHE COSI NON E PIU UN PARAMETRO
	int instvals[NUMVARS];
	fill(instvals, instvals + NUMVARS, 0);
	int curpos=0;
	instvals[curpos]=-1;

	for(int i=0;i<NUM_INDIVIDUALS;i++){
		bool moved=false;
		while(instvals[curpos]+1>=DOMAINS_SIZE){
			curpos++;
			for(int k=curpos-1;k>-1;k--){
								instvals[k]=0;
							}
			moved=true;
		}

		instvals[curpos]+=1;
		if(moved)
					curpos=0;

		women[i]=new Female(NUMVARS,WOMEN_CONNECTEDNESS,DOMAINS_SIZE,varDomains,instvals);
		print_arr(instvals,NUMVARS);
	}

	cout << "*****women generated\n";

}

SM_problem::~SM_problem() {
	for(int i=0;i<NUM_INDIVIDUALS;i++){
		delete men[i];
		delete women[i];
	}
}

//funzione che costruisce i domini delle variabili a caso
void SM_problem::buildVarDomains(){
	varDomains=(char**)malloc(NUMVARS*sizeof(char*));
	for(int i=0;i<NUMVARS;i++){
		varDomains[i]=(char*)malloc(DOMAINS_SIZE*sizeof(char));
		int charbase= abs((rand() % 24)+97);	//parto da questo char
		int u=0;
		for(u=0;u<DOMAINS_SIZE;u++){
			varDomains[i][u]=(char)(charbase+u);	//e ne metto domains_size
		}
	}
}

void SM_problem::gen_random_instance(int *instance){
	for(int i=0;i<NUMVARS;i++){
		instance[i]=(int)rand() % DOMAINS_SIZE;
	}
}

// a partire da men_matchings veriifca weak stability //TODO verify unmatched criteria
bool SM_problem::verify_is_weakstable(){
	int women_matches[NUM_INDIVIDUALS];	//struttra simmetrica di male_match che mi da i matching dal punto di vista delle donne
	for(int i=0;i<NUM_INDIVIDUALS;i++)
		women_matches[i]=-1;
	for(int i=0;i<NUM_INDIVIDUALS;i++){	//simmetrizzo
		if(men_matches[i]>-1)
			women_matches[men_matches[i]]=i;
	}

	for(int i=0;i< NUM_INDIVIDUALS;i++){
		if(men_matches[i]==-1)//TODO single unmatched?????
			continue;
		Male *curmale=men[i];
		float mcurpref=curmale->pref(women[men_matches[i]]);
		for(int k=0;k<NUM_INDIVIDUALS;k++){
			if(k==men_matches[i])
				continue;
			Female *curfemale= women[k];
			if(curmale->pref(curfemale) > mcurpref){
				if(women_matches[k]==-1)
					continue;
				float wcurpref=curfemale->instance_pref(men[women_matches[k]]->myInstance);
				if(curfemale->instance_pref(curmale->myInstance) > wcurpref){
					cout<<"|*|*|*BLOCKING PAIR*|*\n m:"<<curmale->pref(curfemale)<<">"<< mcurpref<< " f:"<<curfemale->instance_pref(curmale->myInstance)<< ">"<< wcurpref
<<"\n";
					return false;
				}

			}
		}
	}
	return true;
}

void SM_problem::solve_with_classicGS(){
	Classic_GS gs(NUM_INDIVIDUALS,men,women);
	gs.gale_shapley_men_opt(this->men_matches);
	cout << "Gale shapley stable match: ";
	print_arr(this->men_matches,NUM_INDIVIDUALS);
}

void SM_problem::solve_with_softGS(){
	SoftGS softgs(NUM_INDIVIDUALS,men,women);
	if(!softgs.test_soft_next())
		cout<<"TEST FAILED\n";
	softgs.gale_shapley_men_opt(this->men_matches);
	cout << "SOFT Gale shapley stable match: ";
	print_arr(this->men_matches,NUM_INDIVIDUALS);
}

void SM_problem::print_arr(int *inst,int length){
	for (int i=0;i<length;i++)
			cout << inst[i]<<"-";
	cout << " \n ";

}

