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
#include "Men.h"
#include "Women.h"
using namespace std;

#define NUMVARS 10	//numero variabili (quindi nodi dell'albero)
#define DOMAINS_SIZE 2	//dimensione dei domini delle variabili
char **varDomains;//ogni varId è associato al suo dominio (che è un array di valori)

#define MALE_TIGHTNESS 0.5	//percentuale di binary constraint NON nulli


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


void testMen(){
	Men a=Men(NUMVARS,DOMAINS_SIZE,MALE_TIGHTNESS,varDomains);
	a.DAC();
	int opt_inst[NUMVARS];
		int idx=0;
	a.DAC_opt(opt_inst,&idx);
	string st;
	a.DOT_representation(&st);
	ofstream myfile;
	myfile.open ("graph.gv");
	myfile << st;
	myfile.close();
}

void testWomen(){
	Women a=Women(NUMVARS,0.3f,DOMAINS_SIZE,varDomains);
	string st;
	a.DOT_representation(&st);
	ofstream myfile;
	myfile.open ("graph.gv");
	myfile << st;
	myfile.close();
}

int main() {
	srand((unsigned)time(0));
	buildVarDomains();
	//testMen();
	testWomen();
	return 0;
}

