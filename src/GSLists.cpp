/*
 * GSLists.cpp
 *
 *  Created on: Jan 20, 2014
 *      Author: deka
 */

#include "GSLists.h"
#include <stdexcept>
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
	map<float,list<SolDesc*>*> prefmap;
	int cursol[NUMVARS];
	int nxsol[NUMVARS];
	int *a,*b,*tmp;
	for(int i=0;i<NUMVARS;i++)
		cursol[i]=0;
/*
	add_to_map(prefmap,m,cursol);
	a=(int*)&cursol;
	b=(int*)&nxsol;
	while(m->CSP_next(a,0.0f,b)){
		add_to_map(prefmap,m,b);

		tmp=a;
		a=b;
		b=tmp;

	}*/
	int instvals[NUMVARS];
		for(int i=0;i<NUMVARS;i++)
			instvals[i]=0;
		instvals[NUMVARS-1]=-1;
		for(int i=0;i<NUM_INDIVIDUALS;i++){
			for(int j=NUMVARS-1;j>-1;j--){
				if(instvals[j]+1<DOMAINS_SIZE){
					instvals[j]+=1;
					break;
				}
				else
					instvals[j]=0;
			}
			add_to_map(prefmap,m,instvals);
		}
	//linearizzo la mappa nella lista di preferenza
	int index=num_individuals;
	int *prefarr=(int*)malloc(sizeof(int)*num_individuals);
	//cout<<"first: ";
	for (std::map<float,list<SolDesc*> *>::iterator it=prefmap.begin(); it!=prefmap.end(); ++it){
		list<SolDesc*> l=(*it->second);
	    for (list<SolDesc*>::reverse_iterator itl=l.rbegin(); itl != l.rend(); ++itl){
	    	//print_arr((*itl)->sol,NUMVARS);+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ERROROR
	    	prefarr[--index]=womencont->find_female_with_instance((*itl)->sol);
	    }
	}
	maleprefs[idx]=prefarr;
	//cout<<"end\n";
	//svuoto map
	for( map<float,list<SolDesc*>*>::iterator i = prefmap.begin(); i != prefmap.end(); ++i ){
		list<SolDesc*> l=(*i->second);
		for (list<SolDesc*>::iterator itl=l.begin(); itl != l.end(); ++itl){
			delete (*itl);
		}

		delete (i->second);
	}

}

void GSLists::add_to_map(map<float,list< SolDesc*>*> &m,Male *mm,int *s){
	float p=mm->instpref(s);
	//print_arr(s,NUMVARS);
	list<SolDesc*>* l;
	if(m.count(p)>0)
		l=m[p];
	else{
		//cout<<"creating entry for "<<p<<"\n";
	   l=new list<SolDesc*>;
	   m[p]=l;
	}
	add_to_list(*l,s,p,mm);
}

void GSLists::add_to_list(list< SolDesc*> &l, int *s,float pref,Male *m){
	//vedo la tupla che genera la soluzione
	Male::Tuple t;
	if(!m->find_first_tuple_with_pref(s,pref,&t)){
		cout<<"GSLISTS error first tuple not found \n";
		exit(1);
	}
	SolDesc *sd=new SolDesc();
	memcpy(sd->sol,s,NUMVARS*sizeof(int));
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
		 if(t.var_idx < (*it)->t_gen.var_idx || (t.var_idx==(*it)->t_gen.var_idx && t.child_idx < (*it)->t_gen.child_idx )){
			 l.insert(it,sd);
			 inserted=true;
			 break;
		 }
		 else if(t.var_idx > (*it)->t_gen.var_idx || (t.var_idx==(*it)->t_gen.var_idx && t.child_idx > (*it)->t_gen.child_idx )){
			 continue;
		 }
		// else if((t.var_idx == (*it)->t_gen.var_idx && t.child_idx == (*it)->t_gen.child_idx )){
			 if(t.idx_in_bintbl[0] < (*it)->t_gen.idx_in_bintbl[0] || (t.idx_in_bintbl[0] == (*it)->t_gen.idx_in_bintbl[0] && t.idx_in_bintbl[1] < (*it)->t_gen.idx_in_bintbl[1])){
				 	 l.insert(it,sd);
				 	 inserted=true;
				 	 break;
			 }
			 else if(t.idx_in_bintbl[0] > (*it)->t_gen.idx_in_bintbl[0] || (t.idx_in_bintbl[0] == (*it)->t_gen.idx_in_bintbl[0] && t.idx_in_bintbl[1] > (*it)->t_gen.idx_in_bintbl[1]))
				 continue;
		// }
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
		 }
		 //ultima risorsa: lex
		if(lex_precedes(sd->sol,(*it)->sol)>0){
			l.insert(it,sd);
			inserted=true;
			break;
		}
	 }
	 if(!inserted){
		 l.push_back(sd);
	 }

}

int GSLists::lex_precedes(int *a, int* b){
	for(int i=0;i<NUMVARS;i++){
		if(a[i]==b[i])
			continue;
		if(a[i]<b[i])
			return 1;
		else
			return -1;
	}

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
	free(femalematching);
	free(lastproposed);
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

void GSLists::print_arr(int *inst,int length){
	for (int i=0;i<length;i++)
			cout << inst[i]<<"-";
	cout << " \n ";

}
