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

	//GENERAZIONE RANDOM UOMINI E DONNE
	/*for(int i=0;i<NUM_INDIVIDUALS;i++){
		int instvals[NUMVARS];
		gen_random_instance(instvals);
		men[i]= new Male(NUMVARS,DOMAINS_SIZE,MALE_TIGHTNESS,varDomains,instvals);
		//********GENERAZIONE RANDOM DELLE DONNE (NON TUTTE)
		gen_random_instance(instvals);
		women[i]= new Female(NUMVARS,WOMEN_CONNECTEDNESS,DOMAINS_SIZE,varDomains,instvals);
		men_matches[i]=-1;
		cout << "*****men "<<i<<" generated. opt:"<<men[i]->myOpt<<" optinst:";
		print_arr(men[i]->myOptInstance,NUMVARS);
	}
*/

	//GENERAZIONE NON RANDOM DELLE DONNE (TUTTE), BISOGNA ADEGUARE NUM_INDIVIDUALS PERCHE COSI NON E PIU UN PARAMETRO
	int instvals[NUMVARS];
	for(int i=0;i<NUMVARS;i++)
		instvals[i]=0;
	instvals[NUMVARS-1]=-1;
	for(int i=0;i<NUM_INDIVIDUALS;i++){
		for(int j=NUMVARS-1;j>-1;j--){
			if(instvals[j]+1<DOMAINS_SIZE){
				instvals[j]+=1;
				break;
			}
			else
				instvals[j]=0;
		}
		men[i]= new Male(NUMVARS,DOMAINS_SIZE,MALE_TIGHTNESS,varDomains,instvals);
		men_matches[i]=-1;
		//cout << "*****men "<<i<<" generated. opt:"<<men[i]->myOpt<<" optinst:";
		//print_arr(men[i]->myOptInstance,NUMVARS);
		women[i]=new Female(NUMVARS,WOMEN_CONNECTEDNESS,DOMAINS_SIZE,varDomains,instvals);
		//print_arr(instvals,NUMVARS);
	}

	cout << "***** generated\n";
}

SM_problem::~SM_problem() {
	/*for(int i=0;i<NUM_INDIVIDUALS;i++){
		delete men[i];
		delete women[i];
	}
	free(men);
	free(women);*/
	for(int i=0;i<NUMVARS;i++)
		free(varDomains[i]);
	free(varDomains);
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
	cout<<"Verifying stability...\n";

	bool result=true;
	int women_matches[NUM_INDIVIDUALS];	//struttra simmetrica di male_match che mi da i matching dal punto di vista delle donne
	for(int i=0;i<NUM_INDIVIDUALS;i++)
		women_matches[i]=-1;
	for(int i=0;i<NUM_INDIVIDUALS;i++){	//simmetrizzo
		if(men_matches[i]>-1)
			women_matches[men_matches[i]]=i;
	}
	#pragma omp parallel for
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
					//curmale->debugTree("blm.gv");

				#pragma omp critical
				{
						result= false;
				}

				}

			}
		}
	}
	return result;
}

int SM_problem::solve_with_classicGS(){
	Classic_GS gs(NUM_INDIVIDUALS,men,women);
	int nprops=gs.gale_shapley_men_opt(this->men_matches);
	cout << "Gale shapley match: ";
	//print_arr(this->men_matches,NUM_INDIVIDUALS);
	return nprops;
}

int SM_problem::solve_with_softGS(){
	SoftGS softgs(NUM_INDIVIDUALS,men,women);
	/*if(!softgs.test_soft_next())
		cout<<"TEST FAILED\n";
*/
	int nprops=softgs.gale_shapley_men_opt_next1(this->men_matches);
	//cout << "SOFT Gale shapley match: ";
	//print_arr(this->men_matches,NUM_INDIVIDUALS);
	print_arr(men_matches,NUM_INDIVIDUALS);
	return nprops;
}

int SM_problem::solve_with_classicGSNext(){
	ClassicGSNext gsnext(NUM_INDIVIDUALS,men,women);
	//debugTrees("GSN");
	int nprops=gsnext.gale_shapley_men_opt(this->men_matches);
	//debugTrees("GSA");
	//cout << "Classic Gale shapley with NEXT match: ";
	//print_arr(this->men_matches,NUM_INDIVIDUALS);
	return nprops;
}

int SM_problem::solve_with_next23(){
	int lineariz=3;
	SoftGS softgs(NUM_INDIVIDUALS,men,women);
	softgs.init_n23_solver(lineariz);
	int nprops=softgs.gale_shapley_men_opt_next23(this->men_matches,lineariz);
	//TODO softgs.dealloc_n23_solver
	cout<<"I went over 2% "<<men[0]->count_over2perc<<" times \n";
	print_arr(men_matches,NUM_INDIVIDUALS);
	return nprops;
}

int SM_problem::solve_with_GSLists(int linearization){
	GSLists gsl(NUM_INDIVIDUALS,men,women,linearization);
	int nprops=gsl.solve_GS(this->men_matches);
	print_arr(men_matches,NUM_INDIVIDUALS);
	return nprops;
}

void SM_problem::print_arr(int *inst,int length){
	for (int i=0;i<length;i++)
			cout << inst[i]<<"-";
	cout << " \n ";

}

void SM_problem::debugTrees(char *filename){
	for(int i=0;i<NUM_INDIVIDUALS;i++)
		{
		 char s[200];
		 char tmp[200];
		 strcpy(s,filename);
		 sprintf(tmp,"%d.gv",i);
		 strcat(s,tmp);
		men[i]->debugTree(s);
		}
}

