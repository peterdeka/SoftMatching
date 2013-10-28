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
			int curinstance[num_individuals];
			bool first=true;
			while(matching[i]==-1){	//se free
				int proposeto;
				if(first){
					first=false;
					proposeto=find_female_with_instance(curman->myOptInstance);
				}
				else{
					curman->SOFT_next(women[matching[i]],curinstance);
					proposeto=find_female_with_instance(curinstance);
				}

				cout << "m"<<i<<" ? "<<proposeto<<"\n";
				if(femalematching[proposeto]==-1){	//free girl
					//cout <<"free girl: men " <<i<<" <- women "<<proposeto<<" \n";
					matching[i]=proposeto;
					femalematching[proposeto]=i;
					freemen--;
				}
				else{	//already engaged, see if prefers new proposal
					Female *curwoman=women[proposeto];
					int ispreferred=curwoman->compare(curman,men[femalematching[proposeto]]);
					if(ispreferred>0){
						//cout <<"girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}
					else if(ispreferred==0 && ((float)rand()/(float)RAND_MAX)>0.5f){// i<femalematching[proposeto]){
						//cout <<"TIEBREAK val:"<<womenprefs[proposeto][i]<<" girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
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
	for(int i=0;i<num_individuals;i++){
		Female *f=women[i];
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
	return -1;
}
