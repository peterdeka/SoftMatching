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
	memcpy(this->myInstance,instance,numvars*sizeof(int));
	buildTree(tightness,numvars,varDomains);
	this->make_DAC();
	this->myOpt=this->DAC_opt(this->myOptInstance);
}



//generazione albero preferenze uomini
void Male::buildTree(float tightness,int numvars,char **varDomains){
	cout <<"Building men tree \n";
	CTree *tree=this->prefTree;
	CTreeNode **curarr=(CTreeNode**)malloc(numvars*sizeof(CTreeNode*));
	CTreeNode **otherarr=(CTreeNode**)malloc(numvars*sizeof(CTreeNode*));
	int randomVarId= rand() % numvars;
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
	this->adjustTightness(tightness);
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
		//verifico non sia giˆ nullo
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
		if(zerocount>=domains_sz2-1){	//skippo perch generei tavola di tutti zeri
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


//per encapsulation devi passare opt_instance giˆ allocato
float Male::DAC_opt(int *opt_instance){
	int curidx=0;
	//prendo il max dei miei unary
	opt_as_father(this->prefTree->root,opt_instance,&curidx);
	return this->prefTree->root->dacUnaryConstraints[opt_instance[0]];
}
//FINE DAC 2nd pass

//calcola valore di preferenza di un'istanza
float Male::pref(Female *f){
	float pref=10.0f;
	int *instance=f->myInstance;
		for(int i=0;i<prefTree->n_nodes;i++){
			int curval=instance[i];
			CTreeNode *curnode=prefTree->linearizedTree[i];
			pref=min(pref,curnode->unaryConstraints[curval]);
			for(int k=0;k<curnode->child_n;k++){
				CTreeNode* curchild=curnode->children[k];
				pref=min(pref,curnode->childConstraints[k][curval][instance[curchild->varId]]);
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


//HCSP next applied to w-cut of SCSP tree
bool Male::CSP_next(int *instance, float cutval, int *nextinstance){
	//lintree is breadth first->ordering is ok
	for(int i=0;i<prefTree->n_nodes;i++){		//istanzio la soluzione passata
		prefTree->linearizedTree[i]->value=instance[i];
		nextinstance[i]=instance[i];
	}
	//ora procedo con l'operazione risalendo l'albero
	for(int i=prefTree->n_nodes-1;i>-1;i--){
		CTreeNode *curnode=prefTree->linearizedTree[i];
		bool found=false;
		//cerco prossimo valore valido in ordinamento del dominio
		for(int k=instance[i]+1;k<domains_size;k++){
			//verifico cutvalue
			if(curnode->dacUnaryConstraints[k]>=cutval && curnode->fatherConstraints[curnode->father->value][k]>=cutval ){  //TODO verifico unary del padre?! in teoria no
				instance[i]=k;
				found=true;
				break;
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
						instance[k]=j;
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


//salvagnini - rossi
bool Male::SOFT_next(Female *curfemale,int *nextinstance){	//TODO work in progress
	float instpref=pref(curfemale);
	if(instpref==this->myOpt){
		if(CSP_next(curfemale->myInstance,this->myOpt,nextinstance))
			return true;
	}
	//niente devo cercare ad un valore di preferenza inferiore

}


