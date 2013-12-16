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
	this->womencont=new FemaleContainer(this->women,num_males);
}

SoftGS::~SoftGS() {
	delete this->womencont;
}


int SoftGS::test23(){
	int maxprops=ceil((float)num_individuals*0.02f);
	men[0]->init_next23_list(2,maxprops);
}

void SoftGS::init_n23_solver(int linearization){
	int maxprops=ceil((float)num_individuals*0.02f);
	if(maxprops<10)
		maxprops=10;
	for(int i=0;i<num_individuals;i++)
		men[i]->init_next23_list(linearization,maxprops);
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
		while(men[i]->SOFT_next(women[find_female_with_instance(cursol)],nextsol) ){
			cout<<men[i]->pref(women[find_female_with_instance(nextsol)])<< " ";
						for(int k=0;k<men[i]->numvars;k++){
								cout<<men[i]->prefTree->linearizedTree[k]->domain[nextsol[k]];
						}
						cout<<"\n";

			//cout<<men[i]->pref(women[find_female_with_instance(nextsol)])<< " ";

			/*//second last equals check		QUESTI DUE CHECK NON SONO SIGNIFICATIVI PERCHE LE DUE CONDIZIONI SI POSSONO VERIFICARE
			bool eq=true;
			for(int k=0;k<men[i]->numvars;k++){
				if(nextsol[k]!=penultima[k])
					eq=false;
			}
			if(eq){
				cout<<"Error PENULTIMA equals\n";
				men[i]->debugTree("errore_penultima_quals.gv");
				exit(1);
			}
			for(int k=0;k<men[i]->numvars;k++){
				penultima[k]=cursol[k];
			}

				//consecutive equals check
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
			}*/
		/*	if(men[i]->n_zeroed_tuples>0){
				cout << "NOT CLEAN\n";
				exit(-1);
			}*/
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
		//	print_arr(cursol,men[i]->numvars);
		}
		if(count<num_individuals){
			cout<<"*****NOT ENOUGH SOLUTIONS "<<count<<"\n";
			men[i]->debugTree("error.gv");
			//return false;
		}
		//else*/
			//cout<<"m"<<i<<" "<<count<<" solutions.\n";
	}
	return true;
}




int SoftGS::gale_shapley_men_opt_next23(int *matching,int linearization){
	int nprops=0;
	ofstream mydbg;
	int *lastproposed=(int*)malloc(num_individuals*sizeof(int));	//tiene traccia dell'ultima donna a cui l'i-esimo uomo ha proposto
	mydbg.open("softgs.txt");
	int *femalematching=(int*)malloc(num_individuals*sizeof(int)); //temp per gestire velocemente
	for(int i=0;i<num_individuals;i++){
		matching[i]=-1;
		femalematching[i]=-1;
		lastproposed[i]=-1;
	}

	bool singles=true;
	while(singles){
		singles=false;
		for(int i=0;i<num_individuals;i++){
			Male *curman=men[i];
			curman->reset_zeroed_prectuples();
			int curinstance[curman->numvars];
			int proposeto;

			while(matching[i]==-1){	//se free
				singles=true;
				if(lastproposed[i]==-1)
					proposeto=womencont->find_female_with_instance(curman->myOptInstance);
				else{
					if(!curman->SOFT_next23(linearization,curinstance))
					{
						cout<<"*********WARNING PROBLEM BECAME SMTI************\n";
						exit(1);
					}
					//cout<<"NEXT\n";
					proposeto=womencont->find_female_with_instance(curinstance);//find_female_with_instance(curinstance);
				}
				//#ifdef GS_DBG
				mydbg << "m"<<i<<" ? w"<<proposeto<<" with pref "<<curman->pref(women[proposeto])<<"\n";
				//#endif
				lastproposed[i]=proposeto;
				nprops++;
				if(femalematching[proposeto]==-1){	//free girl
//#ifdef GS_DBG
				mydbg <<"free girl: men " <<i<<" <- women "<<proposeto<<" \n";
//#endif
					matching[i]=proposeto;
					femalematching[proposeto]=i;
				}
				else{	//already engaged, see if prefers new proposal
					Female *curwoman=women[proposeto];
					int ispreferred=curwoman->compare(curman,men[femalematching[proposeto]]);
					if(ispreferred>0){
//#ifdef GS_DBG
						mydbg <<"girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
//#endif
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}
					else if(ispreferred==0 ){//&& ((float)rand()/(float)RAND_MAX)>0.5f){// i<femalematching[proposeto]){
						cout <<"TIEBREAK girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}

				}
			}
			curman->reset_zeroed_prectuples();
		}
	}
	mydbg.close();
	free(femalematching);
	free(lastproposed);
	return nprops;
}


int SoftGS::gale_shapley_men_opt_next1(int *matching){
	int nprops=0;
	ofstream mydbg;
	int *lastproposed=(int*)malloc(num_individuals*sizeof(int));	//tiene traccia dell'ultima donna a cui l'i-esimo uomo ha proposto
	mydbg.open("softgs.txt");
	int *femalematching=(int*)malloc(num_individuals*sizeof(int)); //temp per gestire velocemente
	for(int i=0;i<num_individuals;i++){
		matching[i]=-1;
		femalematching[i]=-1;
		lastproposed[i]=-1;
	}

	bool singles=true;
	while(singles){
		singles=false;
		for(int i=0;i<num_individuals;i++){
			Male *curman=men[i];
			curman->reset_zeroed_prectuples();
			int curinstance[curman->numvars];
			int proposeto;

			while(matching[i]==-1){	//se free
				singles=true;
				if(lastproposed[i]==-1)
					proposeto=womencont->find_female_with_instance(curman->myOptInstance);
				else{
					if(!curman->SOFT_next(women[lastproposed[i]],curinstance))
					{
						cout<<"*********WARNING PROBLEM BECAME SMTI************\n";
						exit(1);
					}
					//cout<<"NEXT\n";
					proposeto=womencont->find_female_with_instance(curinstance);//find_female_with_instance(curinstance);
				}
				//#ifdef GS_DBG
				mydbg << "m"<<i<<" ? w"<<proposeto<<" with pref "<<curman->pref(women[proposeto])<<"\n";
				//#endif
				lastproposed[i]=proposeto;
				nprops++;
				if(femalematching[proposeto]==-1){	//free girl
//#ifdef GS_DBG
				mydbg <<"free girl: men " <<i<<" <- women "<<proposeto<<" \n";
//#endif
					matching[i]=proposeto;
					femalematching[proposeto]=i;
				}
				else{	//already engaged, see if prefers new proposal
					Female *curwoman=women[proposeto];
					int ispreferred=curwoman->compare(curman,men[femalematching[proposeto]]);
					if(ispreferred>0){
//#ifdef GS_DBG
						mydbg <<"girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
//#endif
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}
					else if(ispreferred==0 ){//&& ((float)rand()/(float)RAND_MAX)>0.5f){// i<femalematching[proposeto]){
						cout <<"TIEBREAK girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}

				}
			}
			curman->reset_zeroed_prectuples();
		}
	}
	mydbg.close();
	free(femalematching);
	free(lastproposed);
	return nprops;
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
