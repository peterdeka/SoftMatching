/*
 * GSLists.cpp
 *
 *  Created on: Jan 20, 2014
 *      Author: deka
 */

#include "GSLists.h"

GSLists::GSLists(int num_males, Male** menarray, Female** womenarray,int lineariz) {
cout<<"READY LISTS\n";
//TODO
	linearization=lineariz;
	men=menarray;
	women=womenarray;
	num_individuals=num_males;
	womencont=new FemaleContainer(womenarray,num_males);
	maleprefs=(int**)malloc(sizeof(int*)*num_individuals);
}

int GSLists::solve_GS(int *matching){
	//genero liste
	for(int i=0;i<num_individuals;i++){
		gen_male_preflist(men[i],i);
	}
	return gale_shapley_men_opt(matching);
}

void GSLists::gen_male_preflist(Male *m, int idx){
	map<float,list<SolDesc*>> prefmap;
	int cursol[NUMVARS];
	for(int i=0;i<NUMVARS;i++)
		cursol[i]=0;

	add_to_map(prefmap,m,cursol);
	while(m->CSP_next(cursol,0.0f,cursol)){
		add_to_map(prefmap,m,cursol);
	}
	//linearizzo la mappa nella lista di preferenza
	int index=0;
	int *prefarr=(int*)malloc(sizeof(int)*num_individuals);
	for (std::map<float,list<SolDesc*> >::iterator it=prefmap.begin(); it!=prefmap.end(); ++it){
		cout<<"first: "<<it->first;
		list<SolDesc*> l=it->second;
	    for (list<SolDesc*>::iterator itl=l.begin(); itl != l.end(); ++itl){
	    	prefarr[index++]=womencont->find_female_with_instance((*itl)->sol);
	    }
	}
	maleprefs[idx]=prefarr;

}

void GSLists::add_to_map(map<float,list< SolDesc*>> m,Male *mm,int *s){
	float p=mm->instpref(s);
	list<SolDesc*> l=m[p];
	add_to_list(l,s,p,mm);
}

void GSLists::add_to_list(list< SolDesc*> l, int *s,float pref,Male *m){
	//vedo la tupla che genera la soluzione
	Male::Tuple t;
	if(!m->find_first_tuple_with_pref(s,pref,&t)){
		cout<<"GSLISTS error first tuple not found \n";
		exit(1);
	}
	SolDesc *sd=new SolDesc();
	memcpy(sd->sol,s,NUMVARS);
	sd->t_gen=t;
	sd->n_t_change=0;
	sd->q_t_change=0.0f;
	//se linear>1
	if(linearization>1){
		//vedo quante ne devo cambiare e di quanto
		count_tuples_change(m,s,pref,sd);
	}
	//cerco dove metterlo
	bool inserted=false;
	 for (list<SolDesc*>::iterator it=l.begin(); it != l.end(); ++it){
	    //confronto tupla generatrice
		 if(t.var_idx < (*it)->t_gen.var_idx || (t.var_idx==(*it)->t_gen.var_idx && t.child_idx< (*it)->t_gen.child_idx )){
			 l.insert(it,sd);
			 inserted=true;
			 break;
		 }
		 else if(t.var_idx > (*it)->t_gen.var_idx || (t.var_idx==(*it)->t_gen.var_idx && t.child_idx> (*it)->t_gen.child_idx )){
			 continue;
		 }
		 if(linearization>1){
			 //numero tuple da cambiare
			 if(sd->n_t_change < (*it)->n_t_change){
				 l.insert(it,sd);
				 inserted=true;
				 break;
			 }
			 else if(sd->n_t_change > (*it)->n_t_change){
				 continue;
			 }
			 if(linearization==3){
				 //di quanto cambiare
				 if(sd->q_t_change < (*it)->q_t_change){
					 l.insert(it,sd);
					 inserted=true;
					 break;
				 }
				 else if(sd->q_t_change > (*it)->q_t_change){
					 continue;
				 }
			 }
			 //ultima risorsa: lex
			 if(lex_precedes(sd->sol,(*it)->sol)>1){
				 l.insert(it,sd);
				 inserted=true;
				 break;
			 }
		 }
	 }
	 if(!inserted){
		 l.push_back(sd);
	 }

}

int GSLists::lex_precedes(int *a, int* b){
	for(int i=0;i<NUMVARS;i++){
		if(a[i]>b[i])
			return -1;
	}
	return 1;
}

void GSLists::count_tuples_change(Male *m,int *s,float pref,SolDesc * sd){
	m->set_solution(s);
	for(int i=0;i<m->prefTree->n_nodes;i++){
		CTreeNode *n=m->prefTree->linearizedTree[i];
		for(int j=0;j<n->child_n;j++){
			float diff=pref-n->childConstraints[j][n->value][n->children[j]->value];
			if(diff>0){
				sd->n_t_change+=1;
				sd->q_t_change+=diff;
			}
		}
	}
}


int GSLists::gale_shapley_men_opt(int *matching){
	int numprops=0;
	int *lastproposed=(int*)malloc(num_individuals*sizeof(int));
	ofstream mydbg;
	mydbg.open("gslists.txt");
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
				proposeto=maleprefs[i][lastproposed[i]];
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
	//free(femalematching);
	//free(lastproposed);
	return numprops;
}

GSLists::~GSLists() {
	// TODO Auto-generated destructor stub
	for(int i=0;i<num_individuals;i++){
		free(maleprefs[i]);
	}
	free(maleprefs);
	delete womencont;
}

