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
	this->n_zeroed_tuples=0;
	this->n23_last_returned_idx=-1;
	this->n23_sols_num=0;
	this->cached_solutions=NULL;
	for(int i=0;i<numvars;i++)
		this->myOptInstance[i]=-1;
	memcpy(this->myInstance,instance,numvars*sizeof(int));
	this->fixed_tuple_childconstr=(float**)malloc(sizeof(float*)*domains_size*domains_size);
	this->zeroed_tuples_backup=(Tuple*)malloc(sizeof(Tuple)*domains_size*domains_size*numvars);
	for(int i=0;i<domains_size;i++)
		this->fixed_tuple_childconstr[i]=(float*)malloc(sizeof(float)*domains_size);
	buildTree(tightness,numvars,varDomains);
		this->make_DAC();
		this->myOpt=this->DAC_opt(this->myOptInstance);

}


void Male::init_next23_list(int linearization, int cachedproposals){
	prefTree->alloc_weighted_tables();
	cached_solutions=(int**)malloc(cachedproposals*sizeof(int*));
	for(int i=0;i<cachedproposals;i++)
		cached_solutions[i]=(int*)malloc(numvars*sizeof(int));
	this->n23_sols_num=this->k_cheapest(cachedproposals,linearization,cached_solutions);
	if(this->n23_sols_num<1){
		cout << "Error building next23_list - no solutions returned by kcheapest\n";
		free(cached_solutions);
		exit(1);
	}
	this->n23_last_returned_idx=0;

}


Male::~Male() {
	delete this->prefTree;
	free(this->myInstance);
	free(this->myOptInstance);
	free(this->zeroed_tuples_backup);
	for(int i=0;i<domains_size;i++){
		free(this->fixed_tuple_childconstr[i]);
	}
	free(this->fixed_tuple_childconstr);
	if(cached_solutions!=NULL){
		for(int i=0;i<this->n23_sols_num;i++)
			free(cached_solutions[i]);
		free(cached_solutions);
	}
	//TODO
}

//generazione albero preferenze uomini
void Male::buildTree(float tightness,int numvars,char **varDomains){
	//cout <<"Building men tree \n";
	CTree *tree=this->prefTree;
	CTreeNode **curarr=(CTreeNode**)malloc(numvars*sizeof(CTreeNode*));
	CTreeNode **otherarr=(CTreeNode**)malloc(numvars*sizeof(CTreeNode*));
	//int randomVarId= rand() % numvars;
	int randomVarId=0;
	CTreeNode *root=new CTreeNode(randomVarId,varDomains[randomVarId]);
	root->genUnaryConstraints(domains_size);

	//ora aggiungo tutte le altre variabili (nodi)
	CTreeNode *node=root;
	tree->setRoot(root);
	curarr[0]=root;
	int curlen=1;
	int otherlen=0;

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

	free(curarr);
	free(otherarr);
	//this->adjustTightness(tightness);
	//cout << "Tree built \n";
}


//riceve tree e annulla random dei constraint fino a raggiungere il livello di tightness desiderato (solo per tree e quindi men)
void Male::adjustTightness(float tightness){
	cout << "Adjusting tightness \n";
	CTree *tree=this->prefTree;
	int domains_sz2=domains_size*domains_size;
	//calcolo numero di binconstr da mettere a zero
	int n_unbind=(float)tree->n_bin_constraints*(tightness);
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
		//verifico non sia gi� nullo
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
		if(zerocount>=domains_sz2-1){	//skippo perch� generei tavola di tutti zeri
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
}


//per encapsulation devi passare opt_instance gi� allocato
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
			//curnode->value=j;
		}
	}

	Tuple t1,t2;
	Tuple *oldt=NULL,*newt=NULL;
	oldt=&t1;
	newt=&t2;
	bool optfound=false;
	if(!find_first_tuple_with_pref(NULL,maxPref,newt))
		exit(1);

	do{
		fix(newt);
		if(CSP_solve(maxPref,opt_instance)==maxPref)
			optfound=true;
		unfix(newt);
		//cout<<"TOPT "<<prefTree->linearizedTree[newt->var_idx]->childConstraints[newt->child_idx][newt->idx_in_bintbl[0]][newt->idx_in_bintbl[1]]<<"\n";
		if(!optfound){
		Tuple *t=newt;
		newt=oldt;
		oldt=t;
		}

	}while(!optfound && next_tuple_with_pref(*oldt,newt,maxPref));
	//if(oldt!=NULL)
	//cout<<"TOPT "<<prefTree->linearizedTree[oldt->var_idx]->childConstraints[oldt->child_idx][oldt->idx_in_bintbl[0]][oldt->idx_in_bintbl[1]]<<"\n";
	//if(newt!=NULL)
	//cout<<"TOPT "<<prefTree->linearizedTree[newt->var_idx]->childConstraints[newt->child_idx][newt->idx_in_bintbl[0]][newt->idx_in_bintbl[1]]<<"\n";
	return this->prefTree->root->dacUnaryConstraints[opt_instance[0]];
}


//calcola valore di preferenza di un'istanza
float Male::pref(Female *f){
	//cout<<"*****PREF****\n";
	float pref=10.0f;
	set_solution(f->myInstance);
	for(int i=0;i<prefTree->n_nodes;i++){
		CTreeNode *curnode=prefTree->linearizedTree[i];
		pref=min(pref,curnode->dacUnaryConstraints[curnode->value]);
		for(int k=0;k<curnode->child_n;k++){
			CTreeNode* curchild=curnode->children[k];
			pref=min(pref,curnode->childConstraints[k][curnode->value][curchild->value]);
		//	cout<<"CHECK "<<curnode->childConstraints[k][curnode->value][curchild->value]<<"\n";
		}
	}
//	cout<<"***END PREF****\n";
	return pref;
}



void Male::fix(Tuple *fixtuple){

		if(fixtuple!=NULL){
			//salvo puntatore a tavola originale non fixata
			fixedtuple_backup=prefTree->linearizedTree[fixtuple->var_idx]->childConstraints[fixtuple->child_idx];
			//fixo tavola di servizio di quest'uomo
			for(int i=0;i<domains_size;i++){
				for(int k=0;k<domains_size;k++){
					if(fixtuple->idx_in_bintbl[0]==i && fixtuple->idx_in_bintbl[1]==k)
						this->fixed_tuple_childconstr[i][k]=prefTree->linearizedTree[fixtuple->var_idx]->childConstraints[fixtuple->child_idx][fixtuple->idx_in_bintbl[0]][fixtuple->idx_in_bintbl[1]];
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
	bool found=true;
	for(int i=prefTree->n_nodes-1;i>-1;i--){

		if(found){
			//resetto istanziazione corrente perch� nell'operazione di ricerca la modifico
			for(int ii=0;ii<prefTree->n_nodes;ii++){		//istanzio la soluzione passata
				prefTree->linearizedTree[ii]->value=instance[ii];
			}}
		CTreeNode *curnode=prefTree->linearizedTree[i];

		int startk=curnode->value+1;
		//cerco prossimo valore valido in ordinamento del dominio
		for(int k=startk;k<domains_size;k++){

			if(found){
				//resetto istanziazione corrente perch� nell'operazione di ricerca la modifico
				for(int ii=0;ii<prefTree->n_nodes;ii++)		//istanzio la soluzione passata
					prefTree->linearizedTree[ii]->value=instance[ii];
			}
			found=false;
			//cout << "CSPNEXT moving node "<<i <<" to "<<curnode->domain[k]<<"\n";
			//verifico cutvalue
			if(curnode==prefTree->root){
				if(curnode->dacUnaryConstraints[k]>=cutval){
					//nextinstance[i]=k;
					curnode->value=k;
					found=true;

				}
			}
			else{
				if(curnode->dacUnaryConstraints[k]>=cutval && curnode->fatherConstraints[curnode->father->value][k]>=cutval ){  // verifico unary del padre?! in teoria no
					//nextinstance[i]=k;
					curnode->value=k;
					found=true;

				}
			}

			if(!found)
				continue;

			//reset-succ, "riazzera contatore" di tutti i nodi successori
			//cout<<"CSPNEXT found candidate "<<curnode->domain[curnode->value]<<"\n";
			bool foundconsistent=true;
			// � l'ultimo nodo quindi non ho successivi da azzerare, ho finito
			//			if(i==prefTree->n_nodes-1){
			//				foundconsistent=true;
			//				//cout<< "CSPNEXT found candidate node is last, ok "<<i<<"\n";
			//			}
			for(int j=i+1;j<prefTree->n_nodes;j++){
				CTreeNode *nd=prefTree->linearizedTree[j];
				//cout<< "CSPNEXT consisting node "<<j<<" and everything under it \n";
				if(!CSP_solve_arc_consist(nd,cutval)){
					foundconsistent=false;
					break;
				}
			}
			if(foundconsistent)
			{
				//cout<<"CSPNEXT returning solution\n";
				for(int x=0;x<prefTree->n_nodes;x++)
					nextinstance[x]=prefTree->linearizedTree[x]->value;
				return true;				//trovato next, esco
			}


		}

	}
	//cout<< "CSPNEXT not returning solution \n";
	return false;
}




float Male::CSP_solve(float cutval, int *solution){
	bool solfound=false;
	float spref;
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
	Female *tmpf=new Female(solution,numvars);

	//cout <<"CSPSOLVE FOUND cutval:"<<cutval<<"\n";
	//print_arr(solution,numvars);
	spref= pref(tmpf);
	delete tmpf;
	return spref;
}



//funzione ricorsiva necessaria per csp solve
bool Male::CSP_solve_arc_consist(CTreeNode *node, float cutval){
	int fatherval=node->father->value;
	for(int i=0;i<domains_size;i++){
		//trovo un valore che va bene con mio padre
		if(node->dacUnaryConstraints[i]>=cutval && node->fatherConstraints[fatherval][i]>=cutval){
		//	cout<< "CSPNEXT-arcconsist found consistent value "<<node->domain[i]<<" for node "<<node->varId<<"\n";
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
	int iinit,kinit;
	bool first=true;
	//cout<<"NEXT_T_WITH_P looking for "<<pref<<" after tuple: v"<<tin.var_idx<<" c"<<tin.child_idx<<" "<<tin.idx_in_bintbl[0]<<" "<<tin.idx_in_bintbl[1]<<"\n";
	for(int c=tin.child_idx;c<curnode->child_n;c++){
		if(c==tin.child_idx)
			iinit=tin.idx_in_bintbl[0];
		else
			iinit=0;

		for(int i=iinit;i< domains_size;i++){
			kinit=0;
			if(first){
				kinit=tin.idx_in_bintbl[1]+1;
				first=false;
			}
			for(int k=kinit;k<domains_size;k++){
				//cout<<"NWITHP: checking same var. child"<<c<<" "<<i<<" "<<k<<"\n";
				if(curnode->childConstraints[c][i][k]==pref){
					tout->var_idx=tin.var_idx;
					tout->child_idx=c;
					tout->idx_in_bintbl[0]=i;
					tout->idx_in_bintbl[1]=k;
					//cout << "FOUNDNEXTTUPLE in same var for child "<<tout->child_idx<<" "<<tout->idx_in_bintbl[0]<<" "<<tout->idx_in_bintbl[1]<<"\n";
					return true;
				}
			}
		}

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
						//cout << "FOUNDNEXTTUPLE "<<tout->var_idx<<" "<<tout->child_idx<<" "<<tout->idx_in_bintbl[0]<<" "<<tout->idx_in_bintbl[1]<<"\n";
						return true;
					}
				}
			}
		}
	}
	//cout <<"SOLVE not found after"<<tin.var_idx<<" "<<tin.child_idx<<" "<<tin.idx_in_bintbl[0]<<" "<<tin.idx_in_bintbl[1]<<"\n";
	//debugTree("solvenotfoundtin.gv");
	return false;
}


//mi serve per verificare stabilita rispettando ordinamento della linearizzazione NEXT
int Male::compare(Female *f1, Female *f2){
	float p1=pref(f1);
	float p2=pref(f2);
	if(p1>p2)
		return 1;
	else if(p1<p2)
		return -1;
	//tie
	Tuple t1,t2;
	find_first_tuple_with_pref(f1->myInstance,p1,&t1);
	find_first_tuple_with_pref(f2->myInstance,p1,&t2);
	//ordinamento lex variabili
	if(t1.var_idx<t2.var_idx)
		return 1;
	else if(t1.var_idx>t2.var_idx)
		return -1;
	else{
		if(t1.child_idx<t2.child_idx)
			return 1;
		else if(t1.child_idx>t2.child_idx)
			return -1;
		else{
			if(t1.idx_in_bintbl[0]>t2.idx_in_bintbl[0])
				return 1;
			else if(t1.idx_in_bintbl[0]<t2.idx_in_bintbl[0])
				return -1;
			else{
				if(t1.idx_in_bintbl[1]>t2.idx_in_bintbl[1])
					return 1;
				else if(t1.idx_in_bintbl[1]<t2.idx_in_bintbl[1])
					return -1;
				else
					return 0;
			}
		}
	}

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
	prefTree->linearizedTree[t->var_idx]->childConstraints[t->child_idx][t->idx_in_bintbl[0]][t->idx_in_bintbl[1]]=0;
	prefTree->linearizedTree[t->var_idx]->children[t->child_idx]->fatherConstraints[t->idx_in_bintbl[0]][t->idx_in_bintbl[1]]=0;
	zeroed_tuples_backup[n_zeroed_tuples].var_idx=t->var_idx;
	zeroed_tuples_backup[n_zeroed_tuples].child_idx=t->child_idx;
	zeroed_tuples_backup[n_zeroed_tuples].idx_in_bintbl[0]=t->idx_in_bintbl[0];
	zeroed_tuples_backup[n_zeroed_tuples].idx_in_bintbl[1]=t->idx_in_bintbl[1];
	//cout<<"zeroed alone "<<t->var_idx<<"."<<t->child_idx<<"."<<t->idx_in_bintbl[0]<<"\n";
	n_zeroed_tuples++;
}

void Male::zeroout_prectuples_with_pref(Tuple *t_star,float pref){
	//tengo traccia delle tuple che azzero
	zeroed_pref=pref;
	int ji=t_star->child_idx;
	int ki=t_star->idx_in_bintbl[0];
	int hi=t_star->idx_in_bintbl[1];//**********************-1 TODO
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
							//cout<<"zeroed "<<i<<"."<<j<<"."<<k<<"\n";
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
		//cout<<"resetted "<<t->var_idx<<"."<<t->child_idx<<"."<<t->idx_in_bintbl[0]<<"\n";
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
bool Male::SOFT_next(Female *curfemale,int *nextsol){
	float p_star=pref(curfemale);
	Tuple *t_star=new Tuple();
	//cout<<"CALLED NEXT\n*******************\n";
	//cerco soluzione successiva generata dalla stessa tupla dell'attuale (con stesso livello di pref)
	if(!find_first_tuple_with_pref(curfemale->myInstance,p_star,t_star))
	{
		cout<<"***SORRY***";
		exit(-1);
	}
	//debugTree("debug_prefix.gv");
	//cout <<"TSTAR:" <<t_star.var_idx<<" "<<t_star.child_idx<<" "<<t_star.idx_in_bintbl[0] <<" "<<t_star.idx_in_bintbl[1]<<"\n";
	fix(t_star);
	if(CSP_next(curfemale->myInstance,p_star,nextsol)){
		unfix(t_star);
		delete t_star;
		return true;
	}

	unfix(t_star);

	//non l'ho trovata, quindi devo trovare un altra tupla allo stesso livello di pref ma lex successiva
	//1 annullo tutte le tuple dell'albero con preferenza = pstar e precedenti a t
	zeroout_prectuples_with_pref(t_star, p_star);

	//2 procedo alla ricerca
	float cpref=p_star;
	Tuple *tfound=new Tuple();
	float tmppref=p_star;
	while(1){
		if(!next_tuple_with_pref(*t_star, tfound, cpref)){		//finite tuple con preferenza cpref, scendo
			//cout<<"*NO MORE tuples with pref "<<cpref<<"\n";
			tmppref=find_next_pref_level(cpref);
			//cout << "NEXT PREF LEVEL:"<<tmppref<<"\n";
			if(tmppref<=0){
				reset_zeroed_prectuples();
				delete tfound;
				delete t_star;
				return false;		//non si scende piu di preferenza, finite le soluzioni
			}
			// reset previuosly set to 0
			find_first_tuple_with_pref(NULL,tmppref,tfound);
			reset_zeroed_prectuples();
		}
		//secondo if, ho trovato una tupla e voglio vedere se mi da soluzione al suo livello di preferenza
		fix(tfound);
		float candpref=CSP_solve(tmppref, nextsol);
		unfix(tfound);
		if(candpref==tmppref)
		{
			//if(prefTree->linearizedTree[tfound->var_idx]->children[tfound->child_idx]->value!=tfound->idx_in_bintbl[1] && prefTree->linearizedTree[tfound->var_idx]->value!=tfound->idx_in_bintbl[0])
				//cout << "****NOT NICE, solution doesnt come from correct tuple\n";
			//reset_zeroed_prectuples();
			delete tfound;
			delete t_star;
			return true;
		}
		//cout<<"**TUPLE at pref "<<tmppref<<" does not solve\n";
		//cout<<"no CSPSOLVE sol found for "<<tmppref<<"\n";
		cpref=tmppref;
		// set tuple to 0
		zeroout_tuple(tfound);
		//swap tuples
		Tuple *tmp=t_star;
		t_star=tfound;
		tfound=tmp;
	}
	delete tfound;
	delete t_star;
	return false; //never hit
}


bool Male::check_cost(int *solution, float cost){
	this->set_solution(solution);
	float ccost=0;
	for(int i=0;i<prefTree->n_nodes;i++){
		CTreeNode *n=prefTree->linearizedTree[i];
		for(int j=0;j<n->child_n;j++)
			ccost+=n->weightedChildConstr[j][n->value][n->children[j]->value];
	}
	if(ccost==cost)
		return true;
	return false;
}


bool Male::SOFT_next23(int linearization, int *nextinstance){
	//finite le soluzioni?
	if(this->n23_last_returned_idx>=this->n23_sols_num){
		//cout << "not enough solutions, KCHEAP increasing to "<<this->n23_sols_num*2<<"\n";
		if(cached_solutions!=NULL){
			for(int i=0;i<this->n23_sols_num;i++)
				free(cached_solutions[i]);
			free(cached_solutions);
		}
		this->n23_last_returned_idx=0;
		cached_solutions=(int**)malloc(this->n23_sols_num*2*sizeof(int*));
		for(int i=0;i<this->n23_sols_num*2;i++)
				cached_solutions[i]=(int*)malloc(numvars*sizeof(int));
		int r_sols=k_cheapest(2*this->n23_sols_num,linearization,cached_solutions);

		if(r_sols<=this->n23_sols_num)
			return false;
		this->n23_sols_num=r_sols;
	}

	memcpy(nextinstance,cached_solutions[this->n23_last_returned_idx++],numvars*sizeof(int));
	return true;
}


int Male::k_cheapest(int k, int linearization, int **solutions){
	float p_star=this->myOpt;
	Tuple *t_star=new Tuple();
	Tuple *tfound=new Tuple();
	int n=0;
	int *nextsol=(int*)malloc(numvars*sizeof(int));
	if(!find_first_tuple_with_pref(NULL,p_star,t_star))
	{
		cout<<"***SORRY***";
		exit(-1);
	}
	fix(t_star);
	float candpref=CSP_solve(p_star, nextsol);
	if(candpref==p_star){
		fuzzy_to_weighted(linearization,p_star,p_star);
		n= elim_m_opt(k,solutions,0);
	}
	unfix(t_star);
	zeroout_prectuples_with_pref(t_star, p_star);

	float cpref=p_star;
	float tmppref=p_star;
	while(n<k){
		if(!next_tuple_with_pref(*t_star, tfound, cpref)){		//finite tuple con preferenza cpref, scendo
	//		cout<<"*NO MORE tuples with pref "<<cpref<<"\n";
			tmppref=find_next_pref_level(cpref);
		//	cout << "NEXT PREF LEVEL:"<<tmppref<<"\n";
			if(tmppref<=0){
				break;		//non si scende piu di preferenza, finite le soluzioni
			}
			// reset previuosly set to 0
			find_first_tuple_with_pref(NULL,tmppref,tfound);
			reset_zeroed_prectuples();
		}
		fix(tfound);
		float candpref=CSP_solve(tmppref, nextsol);
		if(candpref==tmppref){
			fuzzy_to_weighted(linearization,this->myOpt,tmppref);
			n+=elim_m_opt(k-n,solutions,n);
		}
		unfix(tfound);
		cpref=tmppref;
		zeroout_tuple(tfound);
		Tuple *tmp=t_star;
		t_star=tfound;
		tfound=tmp;
	}

//	cout << "kcheap returning " << n<<" out of "<<k<<" requested sols \n";
	reset_zeroed_prectuples();
	delete tfound;
	delete t_star;
	return n;
}


int Male::elim_m_opt(int m, int **solutions,int widx ){
	CTreeNode * n;
	//inizializza i bucket
	for(int i=0;i<prefTree->n_nodes;i++){
		n=prefTree->linearizedTree[i];

		for(int j=0;j<domains_size;j++){

			n->unaryBucket.push_back( vector<float>());
			n->unaryBucket.back().push_back(0.0f);
			//inizializza i bucket per i messaggi con le soluzioni parziali
			int* msg=(int*)malloc(this->numvars*sizeof(int));
			for(int p=0;p<this->numvars;p++){
					//n->messages[j][k][p]=-1;
				if(p==i)
					msg[p]=j;
				else
					msg[p]=-1;
			}

			n->messages.push_back(vector<int*>());
			n->messages.back().push_back(msg);
		}
	}
	elim_m_opt_rec(prefTree->root,m);

	//estrazione soluzioni migliori
	float lastmincost=-1.0f;

	CTreeNode *rn=prefTree->root;
	int bucksz=rn->unaryBucket[0].size();
	int nsols=min(m,domains_size*bucksz);
	int goodsols=0;
	for(int mi=0;mi<nsols;mi++){
		float mincost=1000;
		int minidx=0;
		int mindomain=0;
		for(int y=0;y<domains_size;y++){
			for(unsigned z=0;z<rn->unaryBucket[y].size();z++){
				if(rn->unaryBucket[y][z]<mincost && rn->unaryBucket[y][z]>=lastmincost){
					minidx=z;
					mindomain=y;
					mincost=rn->unaryBucket[y][z];
				}
			}
		}
		lastmincost=rn->unaryBucket[mindomain][minidx];
		rn->unaryBucket[mindomain][minidx]=3000;//non lo pesco piu
		if(lastmincost>=1000){
	//		cout << "Cost is over 1000. Go on. \n";
			break;
		}
		goodsols++;
		//cout << "Solution with cost " << lastmincost<<" : ";
		//print_arr(rn->messages[mindomain][minidx],numvars);
		/*if(check_cost(rn->messages[mindomain][minidx],lastmincost))
			cout<< "checked cost ok \n";
		else
			cout<< "COST ERROR \n";*/
		memcpy(solutions[widx++],rn->messages[mindomain][minidx],numvars*sizeof(int));

	}

	//dealloca i bucket
	for(int i=0;i<prefTree->n_nodes;i++){
			n=prefTree->linearizedTree[i];
//			for(int j=0;j<domains_size;j++){
//				free(n->unaryBucket[j]);
//				for(int k;k<n->n_in_bucket;k++)
//					free(n->messages[j][k]);
//				free(n->messages[j]);
//			}
//			free(n->messages);
//			free(n->unaryBucket);
			n->unaryBucket.clear();
			for(unsigned j=0;j<n->messages.size();j++){
				for(unsigned k=0;k<n->messages[j].size();k++){
					free(n->messages[j][k]);
				}
			}
			n->messages.clear();
	}

	return goodsols;
}


//assume tutti i vincoli unari siano a zero, TODO optimize by sorting buckets and using modified combination operator
void Male::elim_m_opt_rec(CTreeNode *node,int m){
	if(node->child_n<1)
		return;

	//eseguo su ogni figlio
	for(int i=0;i<node->child_n;i++){
		elim_m_opt_rec(node->children[i],m);
	}

	//combination and marginalization
	CTreeNode *nd;

	for (int c = 0; c < node->child_n; c++) {	//per ogni figlio
		nd=node->children[c];

		for(int i=0;i<domains_size;i++){	//dominio padre
			//array che contiene temporaneamente tutti i valori per una variabile padre,di cui prenderemo poi gli m minimi
			float *tmp=(float*)malloc(domains_size*node->unaryBucket[i].size()*nd->unaryBucket[0].size()*sizeof(float));
			int tidx=0;
			//array che contiene il messaggio fuso temporaneo per questo valore del dominio
			vector<int*> tmpmessage;
			for (unsigned b = 0; b < node->unaryBucket[i].size(); b++) {
				for(int j=0;j<domains_size;j++){	//dominio figlio

					for(unsigned t=0;t<nd->unaryBucket[j].size();t++){	//per tutti i valori nel bucket del figlio per la variabile j
						tmp[tidx++]=node->weightedChildConstr[c][i][j]+nd->unaryBucket[j][t]+node->unaryBucket[i][b];
						//cout<<node->weightedChildConstr[c][i][j]+nd->unaryBucket[j][t]+node->unaryBucket[i][b] <<" \n";
					}
				}
			}
			//int nbuck=node->unaryBucket[i].size();
			node->unaryBucket[i].clear();
			//MARGINALIZATION prendo da tmp gli m min per j e i loro messages
			if(tidx<=m){
				for(int k=0;k<tidx;k++){
					int *dstm=(int*)malloc(numvars*sizeof(int));
					node->unaryBucket[i].push_back(tmp[k]);
					//messaggi

					int cdom_pos=floor((float)(k%(nd->unaryBucket[0].size()*nd->domains_sz))/(float)nd->unaryBucket[0].size());
					int fbuck_pos=floor((float)k/((float)nd->unaryBucket[cdom_pos].size()*(float)nd->domains_sz));	//posizione nel bucket i del padre
					int cbuck_pos=k%nd->unaryBucket[cdom_pos].size();
					// tutto il merge deve avvenire in un messaggio temporaneo e alla fine di questo domvalue deve andare a sostituire
					//quello del nodo altrimenti perdi in corsa i valori
					//cout << fbuck_pos<< " < " <<node->messages[i].size()<<"\n";
					merge_messages(node->messages[i][fbuck_pos],nd->messages[cdom_pos][cbuck_pos],dstm);//fbuckpos o k?
					// aggiunta valore variabile proiettata al messaggio
					dstm[nd->varId]=cdom_pos;
					tmpmessage.push_back(dstm);
					//print_arr(tmpmessage[k],numvars);
				}

				//node->n_in_bucket=tidx;
			}
			else{

				for(int p=0;p<m;p++){
					int minidx=0;
					for(int k=0;k<tidx;k++){
						if(tmp[k]<tmp[minidx])
							minidx=k;
					}
					int *dstm=(int*)malloc(numvars*sizeof(int));
					node->unaryBucket[i].push_back(tmp[minidx]);
					//messaggi

					int cdom_pos=floor((float)(minidx%(nd->unaryBucket[0].size()*nd->domains_sz))/(float)nd->unaryBucket[0].size());
					int fbuck_pos=floor((float)minidx/((float)nd->unaryBucket[cdom_pos].size()*(float)nd->domains_sz));	//posizione nel bucket i del padre
					int cbuck_pos=minidx%nd->unaryBucket[cdom_pos].size();
					merge_messages(node->messages[i][fbuck_pos],nd->messages[cdom_pos][cbuck_pos],dstm);	//come sopra
					dstm[nd->varId]=cdom_pos;
					tmpmessage.push_back(dstm);
					tmp[minidx]=1000;
				}
				//node->n_in_bucket=m;

			}
			//vado ora a sostituire l'array di messaggi per il valore di dominio i
			for(unsigned b=0;b<node->messages[i].size();b++){
				free(node->messages[i][b]);
			}
			//free(node->messages[i]);

			node->messages[i].swap(tmpmessage);
			//delete(node->messages[i]);
			//node->messages[i]=*tmpmessage;
			free(tmp);
		}

	}

}

//fonde i due messaggi, (m1 messaggio del padre)
void Male::merge_messages(int *m1, int *m2,int *dst){
	for(int i=0;i<numvars;i++){
		if(m1[i]>-1 && m2[i]>-1){
			cout<< "Error merging messages, two assignments for same var\n";
			exit(1);
		}
		if(m2[i]>-1)
			dst[i]=m2[i];
		else if(m1[i]>-1)
			dst[i]=m1[i];
		else
			dst[i]=-1;
		//cout <<dst[i];
	}
	//cout << "\n";
}


void Male::fuzzy_to_weighted(int linearization, float opt,float pl){
	for(int i=0;i<prefTree->n_nodes;i++){
		CTreeNode *n=prefTree->linearizedTree[i];
		for(int j=0;j<n->child_n;j++){
			for(int h=0;h<domains_size;h++){
				for(int g=0;g<domains_size;g++){
					float cp=n->childConstraints[j][h][g];
					if(cp>=opt)
						n->weightedChildConstr[j][h][g]=0.0f;
					else if(cp<pl)
						n->weightedChildConstr[j][h][g]=1000.0f;
					else{
						if(linearization==2)
							n->weightedChildConstr[j][h][g]=1.0f;
						else
							n->weightedChildConstr[j][h][g]=cp-opt;
					}
					//cout << "W "<<n->weightedChildConstr[j][h][g]<<"\n";
				}
			}
		}
	}
}

//assegna un'istanziazione all'albero
void Male::set_solution(int *instance){
	for(int i=0;i<numvars;i++)
		prefTree->linearizedTree[i]->value=instance[i];
}

void Male::print_arr(int *inst,int length){
	for (int i=0;i<length;i++)
			cout << inst[i]<<" ";
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
