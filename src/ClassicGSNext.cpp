/*
 * ClassicGSNext.cpp
 *
 *  Created on: 02/nov/2013
 *      Author: deka
 */

#include "ClassicGSNext.h"

ClassicGSNext::ClassicGSNext(int num_males, Male** menarray, Female** womenarray) {

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
	int nextsol[men[0]->numvars];
	Female *curf;
	for(int i=0;i<num_males;i++){
		int cnt=0;
		menprefs[i]=(int*)malloc(num_males*sizeof(int));
		for(int j=0;j<num_males;j++)
			menprefs[i][j]=-1;		//inizializzo lista a -1 cosi vedo incompleteness
		menprefs[i][0]=find_female_with_instance(men[i]->myOptInstance);
		curf=women[menprefs[i][0]];
		cnt++;
		while(men[i]->SOFT_next(curf,nextsol)){
			menprefs[i][cnt]=find_female_with_instance(nextsol);
			curf=women[menprefs[i][cnt]];
			cnt++;
		}

	}

	//donne invece hanno in casella i-esima la preferenza per l'uomo i-esimo (random access)
	for(int i=0;i<num_males;i++){
		womenprefs[i]=(float*)malloc(num_males*sizeof(float));
		for(int k=0;k<num_males;k++){
			womenprefs[i][k]=womenarray[i]->instance_pref(menarray[k]->myInstance);
			//cout<<"pref["<<i<<"]["<<k<<"]="<<womenprefs[i][k]<<"\n";
		}
	}

	free(tmparr);
}


void ClassicGSNext::gale_shapley_men_opt(int *matching){
	int freemen=num_individuals;
	//int *lastproposed=(int*)malloc(num_individuals*sizeof(int));
	int *femalematching=(int*)malloc(num_individuals*sizeof(int)); //temp per gestire velocemente
	for(int i=0;i<num_individuals;i++){
		//lastproposed[i]=-1;
		matching[i]=-1;
		femalematching[i]=-1;
	}

	while(freemen>0){
		int proposeto=-1;
		for(int i=0;i<num_individuals;i++){
			int lastproposed=-1;
			while(matching[i]==-1){	//se free

				//lastproposed[i]+=1;
				lastproposed+=1;
				//cout << "LASTPROP("<<i<<") = "<<lastproposed[i]<<"\n";
				proposeto=menprefs[i][lastproposed];
				//if(proposeto>num_individuals-1)
				//exit(-1);
				cout << "m"<<i<<" ? "<<proposeto<<"\n";
				if(femalematching[proposeto]==-1){	//free girl
					//cout <<"free girl: men " <<i<<" <- women "<<proposeto<<" \n";
					matching[i]=proposeto;
					femalematching[proposeto]=i;
					freemen--;
				}
				else{	//already engaged, see if prefers new proposal
					if(womenprefs[proposeto][i] > womenprefs[proposeto][femalematching[proposeto]] ){
						//cout <<"girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}
					else if(womenprefs[proposeto][i] == womenprefs[proposeto][femalematching[proposeto]] && ((float)rand()/(float)RAND_MAX)>0.5f){// i<femalematching[proposeto]){
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
int ClassicGSNext::find_female_with_instance(int *instance){
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


ClassicGSNext::~ClassicGSNext() {
	// TODO Auto-generated destructor stub
}

