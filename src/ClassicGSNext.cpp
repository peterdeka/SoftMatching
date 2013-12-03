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
	//this->womenprefs=(float**)malloc(num_males*sizeof(float*));
	//inizializzo le liste di preferenza rendendole classiche GS
	this->womencont=new FemaleContainer(this->women,num_males);
	float *tmparr=(float*)malloc(num_males*sizeof(float));
	//uomini hanno in casella i-esima donna in posizione i-esima (linear access)
	int nextsol[men[0]->numvars];
	Female *curf;
	for(int i=0;i<num_males;i++){
		//cout<<"m"<<i<<" ";
		int cnt=0;
		menprefs[i]=(int*)malloc(num_males*sizeof(int));
		for(int j=0;j<num_males;j++)
			menprefs[i][j]=-1;		//inizializzo lista a -1 cosi vedo incompleteness
		menprefs[i][0]=womencont->find_female_with_instance(men[i]->myOptInstance);
		curf=women[menprefs[i][0]];
		//cout<<"-"<<menprefs[i][cnt]<<":"<<men[i]->pref(curf);
		cnt++;
		while(men[i]->SOFT_next(curf,nextsol)){
			menprefs[i][cnt]=womencont->find_female_with_instance(nextsol);
			if(menprefs[i][cnt]==-1){
				cout<<"GS with next lists: girl not found\n";
				exit(1);
			}

			curf=women[menprefs[i][cnt]];
			//cout<<"-"<<menprefs[i][cnt]<<":"<<men[i]->pref(curf);
			cnt++;

		}
		//cout<<"m"<<i<<" nprefs:"<<cnt<<"\n";
		men[i]->reset_zeroed_prectuples();

	}

	//donne invece hanno in casella i-esima la preferenza per l'uomo i-esimo (random access)
	//DEPR, COMPARE RESPECTS LINEARIZATION
	/*for(int i=0;i<num_males;i++){
		womenprefs[i]=(float*)malloc(num_males*sizeof(float));
		for(int k=0;k<num_males;k++){
			womenprefs[i][k]=womenarray[i]->instance_pref(menarray[k]->myInstance);
			//cout<<"pref["<<i<<"]["<<k<<"]="<<womenprefs[i][k]<<"\n";
		}
	}*/

	free(tmparr);
	cout<<"Classic GS with next lists: preference lists generated.\n";
}

ClassicGSNext::~ClassicGSNext() {
	delete womencont;
	for(int i=0;i<num_individuals;i++){
		free(menprefs[i]);
		//free(womenprefs[i]);
	}
	free(menprefs);
	//free(womenprefs);
	//TODO
}

int ClassicGSNext::gale_shapley_men_opt(int *matching){
	int numprops=0;
	int *lastproposed=(int*)malloc(num_individuals*sizeof(int));
	ofstream mydbg;
	mydbg.open("classicNEXT.txt");
	int *femalematching=(int*)malloc(num_individuals*sizeof(int)); //temp per gestire velocemente
	for(int i=0;i<num_individuals;i++){
		matching[i]=-1;
		femalematching[i]=-1;
		lastproposed[i]=-1;
	}

	bool singles=true;
	while(singles){
		singles=false;
		int proposeto=-1;
		for(int i=0;i<num_individuals;i++){
			while(matching[i]==-1){	//se free
				singles=true;
				lastproposed[i]+=1;
				proposeto=menprefs[i][lastproposed[i]];
				if(proposeto==-1){	//finite donne accettabili
					cout<<"*********WARNING PROBLEM BECAME SMTI************\n";
					exit(1);
				}
				mydbg << "m"<<i<<" ? w"<<proposeto<<" with pref "<<men[i]->pref(women[proposeto])<<"\n";
				numprops++;
				if(femalematching[proposeto]==-1){	//free girl
					mydbg <<"free girl: men " <<i<<" <- women "<<proposeto<<" \n";
					matching[i]=proposeto;
					femalematching[proposeto]=i;
				}
				else{	//already engaged, see if prefers new proposal
					int preferred=women[proposeto]->compare(men[i],men[femalematching[proposeto]]);
					//if(womenprefs[proposeto][i] > womenprefs[proposeto][femalematching[proposeto]] ){
					if(preferred>0){
						mydbg <<"girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}
					//else if(womenprefs[proposeto][i] == womenprefs[proposeto][femalematching[proposeto]] && ((float)rand()/(float)RAND_MAX)>0.5f){// i<femalematching[proposeto]){
					else if(preferred==0 ){//&& ((float)rand()/(float)RAND_MAX)>0.5f){
						cout <<"TIEBREAK \n";//val:"<<womenprefs[proposeto][i]<<" girl " <<proposeto<<" says goodbye to men "<<femalematching[proposeto]<<" for men "<< i<<" \n";
						matching[femalematching[proposeto]]=-1;
						femalematching[proposeto]=i;
						matching[i]=proposeto;
					}

				}
			}

		}
	}
	mydbg.close();
	free(femalematching);
	free(lastproposed);
	return numprops;
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
