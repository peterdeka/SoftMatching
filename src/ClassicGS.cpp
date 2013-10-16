/*
 * ClassicGS.cpp
 *
 *  Created on: 16/ott/2013
 *      Author: deka
 */

#include "ClassicGS.h"

Classic_GS::Classic_GS(int num_males, Male* menarray, Female* womenarray) {
	this->num_individuals=num_males;
	this->men=menarray;
	this->women=womenarray;
	this->menprefs=(int**)malloc(num_males*sizeof(int*));
	this->womenprefs=(int**)malloc(num_males*sizeof(int*));
	//inizializzo le liste di preferenza rendendole classiche GS
	//1 calcolo preferenze
	//2 ordino
	float *tmparr=(float*)malloc(num_males*sizeof(float));
	//uomini
	for(int i=0;i<num_males;i++){
		menprefs[i]=(int*)malloc(num_males*sizeof(int));
		for(int k=0;k<num_males;k++){
			tmparr[i]=menarray[i].instance_pref(womenarray[k].myInstance);
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

	//donne
	for(int i=0;i<num_males;i++){
		womenprefs[i]=(int*)malloc(num_males*sizeof(int));
		for(int k=0;k<num_males;k++){
			tmparr[i]=womenarray[i].instance_pref(menarray[k].myInstance);
		}
		for(int j=0;j<num_males;j++){	//riordino prendendo ogni volta il max del tmparray
			int curmaxidx=0;
			for(int k=0;k<num_males;k++){
				if(tmparr[curmaxidx]>tmparr[k]){
					curmaxidx=k;
				}
			}
			womenprefs[i][j]=curmaxidx;	//metto id della donna curmax
			tmparr[curmaxidx]=-1;	//cosi non lo prendo piu
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
	while(freemen>0){
//TODO GALE SHAPLEY
	}

}
