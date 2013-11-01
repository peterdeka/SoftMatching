/*
 * Male.cpp
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#include "Male.h"


Male::Male(int numvars,int domains_size, float tightness,char **varDomains,int *instance){
	this->domains_size=domains_size;
	this->prefTree=new CTree(numvars,domains_size);
	this->myInstance=(int*)malloc(numvars*sizeof(int));
	this->numvars=numvars;
	this->myOpt=-1.0f;
	this->myOptInstance=(int*)malloc(numvars*sizeof(int));
	for(int i=0;i<numvars;i++)
		this->myOptInstance[i]=-1;
	memcpy(this->myInstance,instance,numvars*sizeof(int));
	buildTree(tightness,numvars,varDomains);
	this->make_DAC();
	this->myOpt=this->DAC_opt(this->myOptInstance);
	this->fixed_tuple_childconstr=(float**)malloc(sizeof(float*)*domains_size*domains_size);
	this->zeroed_tuples_backup=(Tuple*)malloc(sizeof(Tuple)*domains_size*domains_size*numvars);
	for(int i=0;i<domains_size;i++)
		this->fixed_tuple_childconstr[i]=(float*)malloc(sizeof(float)*domains_size);

}



//generazione albero preferenze uomini
void Male::buildTree(float tightness,int numvars,char **varDomains){
	cout <<"Building men tree \n";
	CTree *tree=this->prefTree;
	CTreeNode **curarr=(CTreeNode**)malloc(numvars*sizeof(CTreeNode*));
	CTreeNode **otherarr=(CTreeNode**)malloc(numvars*sizeof(CTreeNode*));
	//int randomVarId= rand() % numvars;
	int randomVarId=0;
	CTreeNode *root=new CTreeNode(randomVarId,varDomains[randomVarId]);
	root->genUnaryConstraints(domains_size);
	cout<<"unary ok\n";
	//ora aggiungo tutte le altre variabili (nodi)
	CTreeNode *node=root;
	tree->setRoot(root);
	curarr[0]=root;
	int curlen=1;
	int otherlen=0;
	cout<<"GC\n";
	while(tree->n_nodes<numvars){	//randomizzo discendenza
		for(int i=0;i<curlen;i++){	//elaboro nodi sospesi
			node=curarr[i];
			int child_limit=numvars-tree->n_nodes;

			tree->genChildren(node,child_limit,varDomains,numvars);

			for(int j=0;j<node->child_n;j++)
				otherarr[otherlen++]=node->children[j];
		}
		if(otherlen==0)	//se non ho generato figli (possibile a causa di random),reitero identico
			continue;
		//altrimenti resetto e switcho strutture dati per procedere ai nodi non visitati
		CTreeNode **tmp=curarr;
		curarr=otherarr;
		curlen=otherlen;
		otherlen=0;
		otherarr=tmp;
	}
	cout<<"EGC\n";
	free(curarr);
	free(otherarr);
	//this->adjustTightness(tightness);
	cout << "Tree built \n";
}

//riceve tree e annulla random dei constraint fino a raggiungere il livello di tightness desiderato (solo per tree e quindi men)
void Male::adjustTightness(float tightness){
	cout << "Adjusting tightness \n";
	CTree *tree=this->prefTree;
	int domains_sz2=domains_size*domains_size;
	//calcolo numero di binconstr da mettere a zero
	int n_unbind=(float)tree->n_bin_constraints*(1.0f-tightness);
	int chosen_var,chosen_child;
	for(int i=0;i<n_unbind;i++){
		chosen_var=rand() % tree->n_nodes;
		CTreeNode *n=tree->linearizedTree[chosen_var];
		if(n->child_n<1){	//nodo foglia
			i--;
			continue;
		}
		chosen_child=rand() % n->child_n;
		int x=rand() % domains_size;
		int y=rand() % domains_size;
		//verifico non sia già nullo
		if(n->childConstraints[chosen_child][x][y]==0){
			i--;
			continue;
		}
		//verifico tavola non diventi totalmente nulla
		int zerocount=0;
		for(int j=0;j<domains_size;j++){
			for(int jj=0;jj<domains_size;jj++){
				if(n->childConstraints[chosen_child][j][jj]==0)
					zerocount++;
			}
		}
		if(zerocount>=domains_sz2-1){	//skippo perchè generei tavola di tutti zeri
			i--;
			continue;
		}
		n->childConstraints[chosen_child][x][y]=0;
	}

	cout << "Tightness adjusted \n";
}

void Male::DOT_representation(string *res){
	this->prefTree->DOTgraph(res);
}

void Male::make_DAC(){
	DAC_first_pass(this->prefTree->root);
}

//primo passo DAC che propaga la directed arc consistency
void Male::DAC_first_pass(CTreeNode *node){
	for(int i=0;i<node->child_n;i++){
		//step ricorsivo
		DAC_first_pass(node->children[i]);
		//fisso me stesso (padre) e verifico coi figli
		for(int j=0;j<domains_size;j++){
			float maxPref=0.0f;
			for(int k=0;k<domains_size;k++){
				float minPref=min(node->dacUnaryConstraints[j],node->childConstraints[i][j][k]);
				minPref=min(minPref,node->children[i]->dacUnaryConstraints[k]);
				if (minPref>maxPref)
					maxPref=minPref;
			}
			if(maxPref<node->dacUnaryConstraints[j])
				node->dacUnaryConstraints[j]=maxPref;
		}
	}

	//****USELESS**
	//fisso me stesso (figlio) e verifico col padre
	/*if(node->father==NULL)
		return;

	for(int j=0;j<domains_size;j++){
		float maxPref=0.0f;
		for(int k=0;k<domains_size;k++){
			float minPref=min(node->dacUnaryConstraints[j],node->fatherConstraints[k][j]);
			minPref=min(minPref,node->father->dacUnaryConstraints[k]);
			if(minPref>maxPref)
				maxPref=minPref;
		}
		if(maxPref<node->dacUnaryConstraints[j])
			node->dacUnaryConstraints[j]=maxPref;
	}*/

}


// DAC 2nd pass che estrae l'istanziazione ottima e la sua preferenza, output breadth first

void Male::opt_as_child(CTreeNode *node,int *opt_instance,int *curidx){
	//prendo il max dei miei unary
	opt_instance[*curidx]=0;
	float maxPref=node->dacUnaryConstraints[opt_instance[*curidx]];
	for(int j=1;j<domains_size;j++){
		if(node->dacUnaryConstraints[j]>maxPref){
			opt_instance[*curidx]=j;
			maxPref=node->dacUnaryConstraints[j];
		}
	}
	*curidx+=1;
}


void Male::opt_as_father(CTreeNode *node,int *opt_instance,int *curidx){
	if(*curidx==0)
		opt_as_child(node,opt_instance,curidx);
	for(int i=0;i<node->child_n;i++)
		opt_as_child(node->children[i],opt_instance,curidx);
	for(int i=0;i<node->child_n;i++)
		opt_as_father(node->children[i],opt_instance,curidx);
}




//per encapsulation devi passare opt_instance già allocato
float Male::DAC_opt(int *opt_instance){
	int curidx=0;
	//prendo il max dei miei unary
	//opt_as_father(this->prefTree->root,opt_instance,&curidx);
	//return this->prefTree->root->dacUnaryConstraints[opt_instance[0]];
	CTreeNode *curnode=prefTree->root;
	float maxPref=0.0f;
	for(int j=0;j<domains_size;j++){
		if(curnode->dacUnaryConstraints[j]>maxPref){
			maxPref=curnode->dacUnaryConstraints[j];
			curnode->value=j;
		}
	}

	for(int i=0;i<prefTree->n_nodes;i++){
		curnode=prefTree->linearizedTree[i];
		for(int k=0;k<curnode->child_n;k++){
			for(int j=0;j<domains_size;j++){
				if(curnode->childConstraints[k][curnode->value][j]>=curnode->dacUnaryConstraints[curnode->value]){
					curnode->children[k]->value=j;
					break;
				}
			}
		}
	}

	for(int i=0;i<prefTree->n_nodes;i++)
		opt_instance[i]=prefTree->linearizedTree[i]->value;

	return this->prefTree->root->dacUnaryConstraints[opt_instance[0]];
}
//FINE DAC 2nd pass

//calcola valore di preferenza di un'istanza
float Male::pref(Female *f){
	float pref=10.0f;
	set_solution(f->myInstance);
	for(int i=0;i<prefTree->n_nodes;i++){
		CTreeNode *curnode=prefTree->linearizedTree[i];
		pref=min(pref,curnode->dacUnaryConstraints[curnode->value]);
		for(int k=0;k<curnode->child_n;k++){
			CTreeNode* curchild=curnode->children[k];
			pref=min(pref,curnode->childConstraints[k][curnode->value][curchild->value]);
		}
	}
	return pref;
}

//
//int Male::opt(Female **women,int n_women){
//	int bestidx=0;
//	float bestpref=-1;
//	int i=0;
//	for( i=0;i<n_women;i++){
//		float curpref=instance_pref(women[i]->myInstance);
//			if(curpref > bestpref){
//				bestpref=curpref;
//				bestidx=i;
//			}
//	}
//	return i;
//}


void Male::fix(Tuple *fixtuple){

		if(fixtuple!=NULL){
			//salvo puntatore a tavola originale non fixata
			fixedtuple_backup=prefTree->linearizedTree[fixtuple->var_idx]->childConstraints[fixtuple->child_idx];
			//fixo tavola di servizio di quest'uomo
			for(int i=0;i<domains_size;i++){
				for(int k=0;k<domains_size;k++){
					if(fixtuple->idx_in_bintbl[0]==i && fixtuple->idx_in_bintbl[1]==k)
						this->fixed_tuple_childconstr[i][k]=1.0f;
					else
						this->fixed_tuple_childconstr[i][k]=0;
				}
			}
			//swappo i puntatori
			this->prefTree->linearizedTree[fixtuple->var_idx]->childConstraints[fixtuple->child_idx]=this->fixed_tuple_childconstr;
			this->prefTree->linearizedTree[fixtuple->var_idx]->children[fixtuple->child_idx]->fatherConstraints=this->fixed_tuple_childconstr;
		}
}


void Male::unfix(Tuple *fixtuple){
	this->prefTree->linearizedTree[fixtuple->var_idx]->childConstraints[fixtuple->child_idx]=this->fixedtuple_backup;
	this->prefTree->linearizedTree[fixtuple->var_idx]->children[fixtuple->child_idx]->fatherConstraints=this->fixedtuple_backup;
}


//HCSP next applied to w-cut of SCSP tree
bool Male::CSP_next(int *instance, float cutval, int *nextinstance){
	//cout<<"CSPNEXT cutting at "<<cutval<<"\n";
	//lintree is breadth first->ordering is ok
	//ora procedo con l'operazione risalendo l'albero
	for(int i=prefTree->n_nodes-1;i>-1;i--){
		//resetto istanziazione corrente perchè nell'operazione di ricerca la modifico
		for(int ii=0;ii<prefTree->n_nodes;ii++){		//istanzio la soluzione passata
				prefTree->linearizedTree[ii]->value=instance[ii];
				nextinstance[ii]=instance[ii];
		}
		CTreeNode *curnode=prefTree->linearizedTree[i];
		bool found=false;
		//cerco prossimo valore valido in ordinamento del dominio
		for(int k=curnode->value+1;k<domains_size;k++){
			//verifico cutvalue
			if(curnode==prefTree->root){
				if(curnode->dacUnaryConstraints[k]>=cutval){
					nextinstance[i]=k;
					curnode->value=k;
					found=true;
					break;
				}
			}
			else{
				if(curnode->dacUnaryConstraints[k]>=cutval && curnode->fatherConstraints[curnode->father->value][k]>=cutval ){  // verifico unary del padre?! in teoria no
					nextinstance[i]=k;
					curnode->value=k;
					found=true;
					break;
				}
			}
		}
		if(found){
			//reset-succ, "riazzera contatore" di tutti i nodi successori
			for(int k=i+1;k<prefTree->n_nodes;k++){
				bool foundconsistent=false;	//vedo se effettivamente resettando trovo soluzione consistente TODO check with prof
				curnode=prefTree->linearizedTree[k];
				for(int j=0;j<domains_size;j++){
					//verifico cutvalue
					if(curnode->dacUnaryConstraints[j]>=cutval && curnode->fatherConstraints[curnode->father->value][j]>=cutval && curnode->father->dacUnaryConstraints[curnode->father->value]>=cutval){
						nextinstance[k]=j;
						curnode->value=j;
						foundconsistent=true;
						break;
					}
				}
				if(!foundconsistent)	//resettando non ho trovato un assegnamento consistente per la variabile k-esima
					return false;
				//altrimenti continuo e passo alla prossima variabile successiva
			}

			return true;	//trovato next, esco
		}
	}
	return false;
}




float Male::CSP_solve(float cutval, int *solution){
	bool solfound=false;
	for(int i=0;i<domains_size;i++){
		if(prefTree->root->dacUnaryConstraints[i]>=cutval){
			prefTree->root->value=i;
			bool found=true;
			for(int k=0;k<prefTree->root->child_n;k++){
				if(CSP_solve_arc_consist(prefTree->root->children[k],cutval)==false){
					found=false;
					break;
				}
			}
			if(found){		//soluzione trovata
				solfound=true;
				break;
			}

		}
	}
	if(!solfound)
		return -1;
	//trovata quindi restituisco
	for(int i=0;i<numvars;i++){
		solution[i]=prefTree->linearizedTree[i]->value;
	}
	Female tmpf;
	tmpf.myInstance=solution;
	return pref(&tmpf);
}



//funzione ricorsiva necessaria per csp solve
bool Male::CSP_solve_arc_consist(CTreeNode *node, float cutval){
	int fatherval=node->father->value;
	for(int i=0;i<domains_size;i++){
		//trovo un valore che va bene con mio padre
		if(node->dacUnaryConstraints[i]>=cutval && node->fatherConstraints[fatherval][i]>=cutval){
			node->value=i;
			//propago nei figli
			bool found=true;
			for(int k=0;k<node->child_n;k++){
				if(CSP_solve_arc_consist(node->children[k],cutval)==false){
					found=false;
					break;
				}
			}
			if(found)
				return true;
		}
	}
	return false;
}


bool Male::next_tuple_with_pref(Tuple tin, Tuple *tout, float pref){

	//CERCO CON STESSA PREFERENZA
	//seguo linearizzazione quindi cerco in miei binary e poi in altri nodi (sempre successvi)
	CTreeNode* curnode=prefTree->linearizedTree[tin.var_idx];
	//cerco in base all'ordinamento del dominio nella mia tabella binary
	for(int c=tin.child_idx;c<curnode->child_n;c++){
		for(int i=tin.idx_in_bintbl[0];i< domains_size;i++){
			for(int k=tin.idx_in_bintbl[1]+1;k<domains_size;k++){
				if(curnode->childConstraints[tin.child_idx][i][k]==pref){
					tout->var_idx=tin.var_idx;
					tout->child_idx=tin.child_idx;
					tout->idx_in_bintbl[0]=i;
					tout->idx_in_bintbl[1]=k;
					return true;
				}
			}
		}
		//riparto quando cambio figlio
		tin.idx_in_bintbl[0]=0;
		tin.idx_in_bintbl[1]=0;
	}

	//cerco nelle variabili successive
	for(int i=tin.var_idx+1;i<numvars;i++){
		curnode=prefTree->linearizedTree[i];
		for(int k=0;k<curnode->child_n;k++){
			float **tentry=curnode->childConstraints[k];
			for(int j=0;j<domains_size;j++){
				for(int o=0;o<domains_size;o++){
					if(tentry[j][o]==pref){
						tout->var_idx=i;
						tout->child_idx=k;
						tout->idx_in_bintbl[0]=j;
						tout->idx_in_bintbl[1]=o;
						return true;
					}
				}
			}
		}
	}
	return false;
}


float Male::find_next_pref_level(float curpref){
	float nextlevel=-1;
	CTreeNode *curnode;
	for(int i=0;i<numvars;i++){
		curnode=prefTree->linearizedTree[i];
		for(int k=0;k<curnode->child_n;k++){
			float **tentry=curnode->childConstraints[k];
			for(int j=0;j<domains_size;j++){
				for(int o=0;o<domains_size;o++){
					if(tentry[j][o]<curpref && tentry[j][o]>nextlevel){
						nextlevel=tentry[j][o];
					}
				}
			}
		}
	}
	//cout<<"NEXTLEVEL________"<<curpref<<"--->>"<<nextlevel<<"\n";
	return nextlevel;
}


void Male::zeroout_tuple(Tuple *t){
	zeroed_pref=prefTree->linearizedTree[t->var_idx]->childConstraints[t->child_idx][t->idx_in_bintbl[0]][t->idx_in_bintbl[1]];
	//cout<<"ZEROUT"<<zeroed_pref<<"\n";
	prefTree->linearizedTree[t->var_idx]->childConstraints[t->child_idx][t->idx_in_bintbl[0]][t->idx_in_bintbl[1]]=0;
	prefTree->linearizedTree[t->var_idx]->children[t->child_idx]->fatherConstraints[t->idx_in_bintbl[0]][t->idx_in_bintbl[1]]=0;
	zeroed_tuples_backup[n_zeroed_tuples].var_idx=t->var_idx;
	zeroed_tuples_backup[n_zeroed_tuples].child_idx=t->child_idx;
	zeroed_tuples_backup[n_zeroed_tuples].idx_in_bintbl[0]=t->idx_in_bintbl[0];
	zeroed_tuples_backup[n_zeroed_tuples].idx_in_bintbl[1]=t->idx_in_bintbl[1];
	n_zeroed_tuples++;
}

void Male::zeroout_prectuples_with_pref(Tuple *t_star,float pref){
	//tengo traccia delle tuple che azzero
	n_zeroed_tuples=0;
	zeroed_pref=pref;
	int ji=t_star->child_idx;
	int ki=t_star->idx_in_bintbl[0];
	int hi=t_star->idx_in_bintbl[1];//**********************-1
	for(int i=t_star->var_idx ;i>-1;i--){
		CTreeNode *curnode=prefTree->linearizedTree[i];
		if(i<t_star->var_idx){
				ji=curnode->child_n-1;
				ki=domains_size-1;
				hi=domains_size-1;
			}
			for(int j=ji;j>-1;j--){
				for(int k=ki;k>-1;k--){
					for(int h=hi;h>-1;h--){
						if(curnode->childConstraints[j][k][h]==pref){
							zeroed_tuples_backup[n_zeroed_tuples].var_idx=i;
							zeroed_tuples_backup[n_zeroed_tuples].child_idx=j;
							zeroed_tuples_backup[n_zeroed_tuples].idx_in_bintbl[0]=k;
							zeroed_tuples_backup[n_zeroed_tuples].idx_in_bintbl[1]=h;
							n_zeroed_tuples++;
							curnode->childConstraints[j][k][h]=0;
							curnode->children[j]->fatherConstraints[k][h]=0;
						}
					}
				}
			}
		}
}

void Male::reset_zeroed_prectuples(){
	Tuple *t;
	for(int i=0;i<n_zeroed_tuples;i++){
		t=zeroed_tuples_backup+i;
		prefTree->linearizedTree[t->var_idx]->childConstraints[t->child_idx][t->idx_in_bintbl[0]][t->idx_in_bintbl[1]]=zeroed_pref;
		prefTree->linearizedTree[t->var_idx]->children[t->child_idx]->fatherConstraints[t->idx_in_bintbl[0]][t->idx_in_bintbl[1]]=zeroed_pref;
	}
	n_zeroed_tuples=0;
	//cout << "ZEROED RESET to "<<zeroed_pref<<"\n";
}



void Male::debugTree(char* fname){
	string st;
		char rootcall=1;
				prefTree->root->DOTSubtree(&st,&rootcall,domains_size);
				ofstream myfile;
			myfile.open (fname);
			myfile << st;
			myfile.close();
}

//salvagnini - rossi
bool Male::SOFT_next(Female *curfemale,int *nextsol){	//TODO work in progress
	float p_star=pref(curfemale);
	Tuple t_star;
	//cout<<"CALLED NEXT\n*******************\n";
	//cerco soluzione successiva generata dalla stessa tupla dell'attuale (con stesso livello di pref)
	if(!find_first_tuple_with_pref(curfemale->myInstance,p_star,&t_star))
		{
			cout<<"***SORRY***";
			exit(-1);
		}
	//debugTree("debug_prefix.gv");
	//cout <<"TSTAR:" <<t_star.var_idx<<" "<<t_star.child_idx<<" "<<t_star.idx_in_bintbl[0] <<" "<<t_star.idx_in_bintbl[1]<<"\n";
	fix(&t_star);
	if(CSP_next(curfemale->myInstance,p_star,nextsol)){

	//	cout <<"***CSPNEXT";
		//print_arr(curfemale->myInstance,numvars);
	//	cout<<"-->";
	//	print_arr(nextsol,numvars);
	//	cout<<"\n";
		//debugTree("debug_fixed.gv");
		unfix(&t_star);
		//debugTree("debug_unfixed.gv");
		return true;
	}
	cout<<"NO CSPNEXT sol found for "<<p_star<<" after ";
	print_arr(curfemale->myInstance,numvars);
	unfix(&t_star);

	//non l'ho trovata, quindi devo trovare un altra tupla allo stesso livello di pref ma lex successiva
	//1 annullo tutte le tuple dell'albero con preferenza = pstar e precedenti a t
	zeroout_prectuples_with_pref(&t_star, p_star);
	//debugTree("debug_prec_zeroed.gv");
	//2 procedo alla ricerca
	float cpref=p_star;
	Tuple tfound;
	float tmppref=p_star;
	while(1){
	//	cout<<"-\n";
		if(!next_tuple_with_pref(t_star, &tfound, cpref)){		//finite tuple con preferenza cpref, scendo
			tmppref=find_next_pref_level(cpref);
	//		cout << "NEXT PREF LEVEL:"<<tmppref<<"\n";
			if(tmppref<=0)
				return false;		//non si scende piu di preferenza, finite le soluzioni
			find_first_tuple_with_pref(NULL,tmppref,&tfound);
			// reset previuosly set to 0
			reset_zeroed_prectuples();
		}
		//secondo if, ho trovato una tupla e voglio vedere se mi da soluzione al suo livello di preferenza
	//	cout <<"TFOUND:" <<tfound.var_idx<<" "<<tfound.child_idx<<" "<<tfound.idx_in_bintbl[0] <<" "<<tfound.idx_in_bintbl[1]<<"\n";
		fix(&tfound);
		float candpref=CSP_solve(tmppref, nextsol);
		unfix(&tfound);
		if(candpref==tmppref)
			{
		//		print_arr(curfemale->myInstance,numvars);
		//		cout<<"-->";
		//		print_arr(nextsol,numvars);
				if(prefTree->linearizedTree[tfound.var_idx]->children[tfound.child_idx]->value!=tfound.idx_in_bintbl[1] && prefTree->linearizedTree[tfound.var_idx]->value!=tfound.idx_in_bintbl[0])
					cout << "****NOT NICE, solution doesnt come from correct tuple\n";
				return true;
			}
		cout<<"no CSPSOLVE sol found for "<<tmppref<<"\n";
		cpref=tmppref;
		// set tuple to 0
		zeroout_tuple(&tfound);
	}
	return false; //never hit
}

//assegna un'istanziazione all'albero
void Male::set_solution(int *instance){
	for(int i=0;i<numvars;i++)
		prefTree->linearizedTree[i]->value=instance[i];
}

void Male::print_arr(int *inst,int length){
	for (int i=0;i<length;i++)
			cout << inst[i]<<"-";
	cout << " \n ";

}
//procedo secondo la linearizzazione (ordine tuple quindi breadth first) a trovare il primo binary che ha preferenza=pref
bool Male::find_first_tuple_with_pref(int* instance, float pref, Tuple *tuple){
	if(instance!=NULL){ 	//devo cercarla all'interno della soluzione passata
		//istanzio la soluzione
		set_solution(instance);
		//cout <<"SEARCHING for pref tuple: p="<<pref<<" for sol:";
		//print_arr(instance,numvars);
		for(int i=0;i<numvars;i++){
			CTreeNode *curnode=prefTree->linearizedTree[i];
			for(int k=0;k<curnode->child_n;k++){
				//cout<<"*"<<curnode->childConstraints[k][curnode->value][curnode->children[k]->value]<<"=?="<<pref<<"\n";
				if(curnode->childConstraints[k][curnode->value][curnode->children[k]->value]==pref){
					tuple->var_idx=i;
					tuple->child_idx=k;
					tuple->idx_in_bintbl[0]=curnode->value;
					tuple->idx_in_bintbl[1]=curnode->children[k]->value;
					return true;
				}
			}
		}
	}
	else{		//non devo trovarla all'interno di una soluzione data ma all'interno dell'intero grafo
		for(int i=0;i<numvars;i++){
			CTreeNode *curnode=prefTree->linearizedTree[i];
			for(int k=0;k<curnode->child_n;k++){
				for(int j=0;j<domains_size;j++){
					for(int h=0;h<domains_size;h++){
						if(curnode->childConstraints[k][j][h]==pref){
							tuple->var_idx=i;
							tuple->child_idx=k;
							tuple->idx_in_bintbl[0]=j;
							tuple->idx_in_bintbl[1]=h;
							return true;
						}
					}
				}

			}
		}
	}
	return false;
}
