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

	float sum=0;
	SM_problem *p= new SM_problem();
	Profiler *prof=new Profiler();
	cout<<"problem generated "<<p->numvars<<" vars "<<"with "<<p->domsz<<" domain\n";
	char fname[255];
	sprintf(fname,"output_dsz%d_nvars%d_23.csv",p->domsz,p->numvars);
	ofstream myfile;
	myfile.open (fname);
	//myfile<<"t_Soft, t_classic_next, props_Soft, props_classic_next\n";
	myfile<<"t_Soft23, props_Soft23\n";
	timespec tm0,tm1;

	for(int j=0;j<5;j++){
		delete p;
				p= new SM_problem();
	prof->start();
	bool stable=true;
	int nprops23=p->solve_with_GSLists(3);
	prof->stop(&tm0);
	if(p->verify_is_weakstable())
				cout<< "SOFTGS_n23 Verified weak stable OK\n";
			else
			{
				cout<< "SOFTGS_n32 Sorry solution not weak stable\n";
			}

	float time=tm0.tv_sec+tm0.tv_nsec/1000000000.0;
	cout << "soft: "<<time <<" nprops23 "<<" proposals\n";
	myfile << time<< ", "<<nprops23<<"\n";
	sum+=time;
	}
cout <<"END: "<<sum/5.0f<<"\n";
	/*for(int j=0;j<5;j++){
		delete p;
		p= new SM_problem();
		cout<<"problem generated\n";
		prof->start();
		bool stable=true;
		int npropsS=p->solve_with_softGS();
		prof->stop(&tm0);
		if(p->verify_is_weakstable())
			cout<< "SOFTGS Verified weak stable OK\n";
		else
		{
			stable=false;
			cout<< "SOFTGS Sorry solution not weak stable\n";
		}
		cout << "soft: "<<tm0.tv_sec<<"s "<<tm0.tv_nsec<<"nsec "<<npropsS<<" proposals\n";
		//p->debugTrees("be");
		prof->start();
		int npropsCN=p->solve_with_classicGSNext();
		//p->debugTrees("me");
		prof->stop(&tm1);
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
		myfile << tm0.tv_sec+tm0.tv_nsec/1000000000.0<< ", "<<tm1.tv_sec+tm1.tv_nsec/1000000000.0<< ", "<<npropsS<<", "<<npropsCN<<"\n";
		cout << "classicwithnext: "<<tm1.tv_sec+tm1.tv_nsec/1000000000.0<<"sec "<<npropsCN<<" proposals\n";

	}*/

	delete p;
	delete prof;
	myfile.close();
	return 0;
}

