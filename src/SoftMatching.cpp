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


int main() {

	SM_problem *p= new SM_problem();
	Profiler *prof=new Profiler();
	char fname[255];
	sprintf(fname,"output_dsz%d_nvars%d",p->domsz,p->numvars);
	ofstream myfile;
	myfile.open (fname);
	myfile<<"t_Soft t_classic_next props_Soft props_classic_next\n";
	timespec tm;
	for(int j=0;j<10;j++){
		delete p;
		p= new SM_problem();
		prof->start();
		bool stable=true;
		int npropsS=p->solve_with_softGS();
		double softtime;
		prof->stop(&tm);
		if(p->verify_is_weakstable())
			cout<< "SOFTGS Verified weak stable OK\n";
		else
		{
			stable=false;
			cout<< "SOFTGS Sorry solution not weak stable\n";
		}
		cout << "soft: "<<tm.tv_sec<<"s "<<tm.tv_nsec<<"nsec "<<npropsS<<" proposals\n";
		//p->debugTrees("be");
		prof->start();
		int npropsCN=p->solve_with_classicGSNext();
		//p->debugTrees("me");
		prof->stop(&tm);
		if(p->verify_is_weakstable())
			cout<< "GSNEXT Verified weak stable OK\n";
		else
			{
				stable=false;
				cout<< "GSNEXT Sorry solution not weak stable\n";
			}

		//p->debugTrees("af");
		//results
		if(!stable){
			cout<<"******ERROR NOT STABLE********** QUITTING...\n";
			exit(1);
		}
		//cout << "(soft: "<<softtime<<"s "<<npropsS<<" proposals; classicNext: "<<classicnexttime<<"s "<<npropsCN<<" proposals)\n";
		//myfile << softtime<< " "<<classicnexttime<< " "<<npropsS<<" "<<npropsCN<<"\n";
		cout << "classicwithnext: "<<tm.tv_sec<<"s "<<tm.tv_nsec<<"nsec "<<npropsS<<" proposals\n";

	}
	myfile.close();
	return 0;
}

