/*
 * SoftGS.cpp
 *
 *  Created on: 19/ott/2013
 *      Author: deka
 */

#include "SoftGS.h"

SoftGS::SoftGS(int num_males, Male** menarray, Female** womenarray) {
	this->num_individuals=num_males;
	this->men=menarray;
	this->women=womenarray;
}

SoftGS::~SoftGS() {
	// TODO Auto-generated destructor stub
}

bool SoftGS::test_soft_next(){
	for(int i=0;i<num_individuals;i++){
		cout<<"NEWINDIVIDUAL opt:"<<men[i]->myOpt<<"\n";
		int sol[men[i]->numvars],nx[men[i]->numvars];
		int *cursol,*nextsol;
		cursol=sol;
		nextsol=nx;
		float pref=men[i]->myOpt;
		int count=1;
		memcpy(sol,men[i]->myOptInstance,sizeof(int)*men[i]->numvars);
		cout<<men[i]->pref(women[find_female_with_instance(cursol)])<< " ";
				for(int k=0;k<men[i]->numvars;k++){
					cout<<men[i]->prefTree->linearizedTree[k]->domain[cursol[k]];
				}
				cout<<"\n";
		while(men[i]->SOFT_next(women[find_female_with_instance(cursol)],nextsol) && count<num_individuals){
			cout<<men[i]->pref(women[find_female_with_instance(nextsol)])<< " ";
			bool equals=true;
			for(int k=0;k<men[i]->numvars;k++){
				cout<<men[i]->prefTree->linearizedTree[k]->domain[nextsol[k]];
				if(nextsol[k]!=cursol[k])
					equals=false;
			}
			cout<<"\n";

			if(equals)
			{
				cout<<"Error equals\n";
				men[i]->debugTree("errorequals.gv");
			}
			if(men[i]->n_zeroed_tuples>0){
				cout << "NOT CLEAN\n";
				exit(-1);
			}
			count+=1;
			if(men[i]->pref(women[find_female_with_instance(nextsol)])>pref){
				cout<<"*****NOT MONOTONIC\n";
				exit(-1);
			}
			pref=men[i]->pref(women[find_female_with_instance(nextsol)]);
			int *tmp=cursol;
			cursol=nextsol;
			nextsol=tmp;
			//cout <<"SOL "<<pref<<" :";
			//print_arr(cursol,men[i]->numvars);
		}
		if(count<num_individuals){
			cout<<"*****NOT ENOUGH SOLUTIONS "<<count<<"\n";
			men[i]->debugTree("error.gv");
			return false;
		}
	}
	return true;
}


void SoftGS::gale_shapley_men_opt(int *matching){
	int freemen=num_individuals;
	int *femalematching=(int*)malloc(num_individuals*sizeof(int)); //temp per gestire velocemente
	for(int i=0;i<num_individuals;i++){
		matching[i]=-1;
		femalematching[i]=-1;
	}

	while(freemen>0){

		for(int i=0;i<num_individuals;i++){
			Male *curman=men[i];
			int curinstance[curman->numvars];
			bool first=true;
			int proposeto;
			//DBG
			if(matching[i]==-1)
				cout<<"*** m"<<i<<" is unmatched\n";
			//EDBG
			while(matching[i]==-1){	//se free

				if(first){
					first=false;
					proposeto=find_female_with_instance(curman->myOptInstance);
				}
				else{
					if(!curman->SOFT_next(women[proposeto],curinstance))
						{
							cout<<"******* NEXT returned 0\n";
							break;
						}
					cout<<"NEXT\n";
					proposeto=find_female_with_instance(curinstance);
				}

				cout << "m"<<i<<" ? "<<proposeto<<" with pref "<<curman->pref(women[proposeto])<<"\n";
				if(femalematching[proposeto]==-1){	//free girl
					cout <<"free girl: men " <<i<<" <- women "<<proposeto<<" \n";
					matching[i]=proposeto;
					femalematching[proposeto]=i;
					freemen--;
				}
				else{	//already engaged, see if prefers new proposal
					Female *curwoman=women[proposeto];
					int ispreferred=curwoman->compare(curman,men[femalematching[proposeto]]);
					if(ispreferred>0){
						cout <<"girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}
					else if(ispreferred==0 && ((float)rand()/(float)RAND_MAX)>0.5f){// i<femalematching[proposeto]){
						cout <<"TIEBREAK girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}

				}
			}

		}
	}

}

//restituisce indice nell'array women della donna con queste carattiristiche (questa istanza)
int SoftGS::find_female_with_instance(int *instance){
	Female *f;
	for(int i=0;i<num_individuals;i++){
		f=women[i];
		bool sheis=true;
		for(int k=0;k<f->numvars;k++){
			if(instance[k]!=f->myInstance[k]){
				sheis=false;
				break;
			}
		}
		if(sheis)
			return i;
	}
	//cout << "****FEMALE NOT FOUND ";
	//print_arr(instance,f->numvars);
	return -1;
}

void SoftGS::print_arr(int *inst,int length){
	for (int i=0;i<length;i++)
			cout << inst[i]<<"-";
	cout << " \n ";

}
