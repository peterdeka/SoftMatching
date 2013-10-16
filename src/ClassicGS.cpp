/*
 * ClassicGS.cpp
 *
 *  Created on: 16/ott/2013
 *      Author: deka
 */

#include "ClassicGS.h"

Classic_GS::Classic_GS(int num_males, Male** menarray, Female** womenarray) {
	this->num_individuals=num_males;
	this->men=menarray;
	this->women=womenarray;
	this->menprefs=(int**)malloc(num_males*sizeof(int*));
	this->womenprefs=(float**)malloc(num_males*sizeof(float*));
	//inizializzo le liste di preferenza rendendole classiche GS
	//1 calcolo preferenze
	//2 ordino
	float *tmparr=(float*)malloc(num_males*sizeof(float));
	//uomini hanno in casella i-esima donna in posizione i-esima (linear access)
	for(int i=0;i<num_males;i++){
		menprefs[i]=(int*)malloc(num_males*sizeof(int));
		for(int k=0;k<num_males;k++){
			tmparr[i]=menarray[i]->instance_pref(womenarray[k]->myInstance);
		}
		for(int j=0;j<num_males;j++){	//riordino prendendo ogni volta il max del tmparray
			int curmaxidx=0;
			for(int k=0;k<num_males;k++){
				if(tmparr[curmaxidx]>tmparr[k]){
					curmaxidx=k;
				}
			}
			menprefs[i][j]=curmaxidx;	//metto id della donna curmax
			tmparr[curmaxidx]=-1;	//cosi non lo prendo piu
		}
	}

	//donne invece hanno in casella i-esima la preferenza per l'uomo i-esimo (random access)
	for(int i=0;i<num_males;i++){
		womenprefs[i]=(float*)malloc(num_males*sizeof(float));
		for(int k=0;k<num_males;k++){
			womenprefs[i][k]=womenarray[i]->instance_pref(menarray[k]->myInstance);
		}
	}

	free(tmparr);
}


Classic_GS::~Classic_GS() {
	for(int i=0;i<num_individuals;i++){
		free(menprefs[i]);
		free(womenprefs[i]);
	}
	free(this->menprefs);
	free(this->womenprefs);
}

void Classic_GS::gale_shapley_men_opt(int *matching){
	int freemen=num_individuals;
	int *lastproposed=(int*)malloc(num_individuals*sizeof(int));
	int *femalematching=(int*)malloc(num_individuals*sizeof(int)); //temp per gestire velocemente
	for(int i=0;i<num_individuals;i++){
		lastproposed[i]=-1;
		matching[i]=-1;
		femalematching[i]=-1;
	}

	while(freemen>0){
		int proposeto=-1;
		for(int i=0;i<num_individuals;i++){
			if(matching[i]>-1)
				continue;	//already matched
			lastproposed[i]+=1;
			proposeto=menprefs[i][lastproposed[i]];
			if(proposeto>num_individuals-1)
				exit(-1);				//FIXME TODO
			if(femalematching[proposeto]==-1){	//free girl
				cout <<"free girl: men " <<i<<" <- women "<<proposeto<<" \n";
				matching[i]=proposeto;
				femalematching[proposeto]=i;
				freemen--;
				continue;
			}
			else{	//already engaged, see if prefers new proposal
				if(womenprefs[proposeto][i] > womenprefs[proposeto][femalematching[proposeto]] ){
					cout <<"girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
					matching[femalematching[proposeto]]=-1;
					femalematching[proposeto]=i;
					matching[i]=proposeto;
				}
				else if(womenprefs[proposeto][i] == womenprefs[proposeto][femalematching[proposeto]] && i>femalematching[proposeto]){
					cout <<"TIEBREAK girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
					matching[femalematching[proposeto]]=-1;
					femalematching[proposeto]=i;
					matching[i]=proposeto;
				}
			}
		}

	}

}
