//============================================================================
// Name        : SoftMatching.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

/*
 * linearizedTree contiene all'indice i la variabile con id=i, non segue alcun ordinamento (breadth o depth)
 * 					-generazione binary constr (no dup check) piu veloce
 * 					-piu veloce generazione random del women graph
 */



#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <algorithm>
#include "SMproblem.h"
#include "Profiler.h"
using namespace std;

/*#define NUMVARS 10	//numero variabili (quindi nodi dell'albero)
#define DOMAINS_SIZE 2	//dimensione dei domini delle variabili
char **varDomains;//ogni varId è associato al suo dominio (che è un array di valori)

#define MALE_TIGHTNESS 0.8	//percentuale di binary constraint NON nulli


//funzione che costruisce i domini delle variabili a caso
void buildVarDomains(){
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

void gen_random_instance(int *instance){
	for(int i=0;i<NUMVARS;i++){
		instance[i]=(int)rand() % DOMAINS_SIZE;
	}
}

void print_instance(int *inst){
	for (int i=0;i<NUMVARS;i++)
			cout << inst[i];
	cout << " -> ";
	for (int i=0;i<NUMVARS;i++)
				cout << varDomains[i][inst[i]];
}

void testMen(){
	int instvals[NUMVARS];
	gen_random_instance(instvals);
	Male a=Male(NUMVARS,DOMAINS_SIZE,MALE_TIGHTNESS,varDomains,instvals);
	a.DAC();
	int opt_inst[NUMVARS];
	int idx=0;
	a.DAC();
	float pref=a.DAC_opt(opt_inst,&idx);
	string st;
	a.DOT_representation(&st);
	ofstream myfile;
	myfile.open ("Mgraph.gv");
	myfile << st;
	myfile.close();
	cout << "Your men opt is instance ";
	print_instance(opt_inst);
	cout << " with pref " << pref;
}

void testWomen(){
	int instvals[NUMVARS];
	gen_random_instance(instvals);
	Female a=Female(NUMVARS,0.3f,DOMAINS_SIZE,varDomains,instvals);
	string st;
	a.DOT_representation(&st);
	ofstream myfile;
	myfile.open ("Wgraph.gv");
	myfile << st;
	myfile.close();
	int testinstance[NUMVARS];
	gen_random_instance(testinstance);
	cout << "Instance ";
	print_instance(testinstance);
	cout << " has pref "<< a.instance_pref(testinstance)<<"\n";
}*/

int main() {

	SM_problem *p= new SM_problem();
	Profiler *prof=new Profiler();
	prof->start();
	int npropsC=0;
	//int npropsC=p->solve_with_classicGS();
	double classictime=prof->stop();
	//p->debugTrees("be");
//	if(p->verify_is_weakstable())
//		cout<< "Verified weak stable OK\n";
//	else
//		cout<< "Sorry solution not weak stable";

		prof->start();
	int npropsS=p->solve_with_softGS();
	double softtime=prof->stop();
	if(p->verify_is_weakstable())
			cout<< "SOFTGS Verified weak stable OK\n";
		else
			{

				cout<< "SOFTGS Sorry solution not weak stable\n";
			}


	prof->start();
		int npropsCN=p->solve_with_classicGSNext();
		p->debugTrees("me");
		double classicnexttime=prof->stop();
		if(p->verify_is_weakstable())
			cout<< "GSNEXT Verified weak stable OK\n";
		else
			cout<< "GSNEXT Sorry solution not weak stable\n";

	p->debugTrees("af");
	//results
	cout << "(soft: "<<softtime<<"ms "<<npropsS<<" proposals"<<" classic: "<<classictime<<"ms "<<npropsC<<" proposals"<<" classicNext: "<<classicnexttime<<"ms "<<npropsCN<<" proposals)\n";
	return 0;
}

