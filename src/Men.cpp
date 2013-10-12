/*
 * Men.cpp
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#include "Men.h"


Men::Men(int numvars,int domains_size, float tightness,char **varDomains){
	this->domains_size=domains_size;
	this->prefTree=new CTree(numvars,domains_size);
	this->myInstance=(int*)malloc(numvars*sizeof(int));
	buildTree(tightness,numvars,varDomains);
}



//generazione albero preferenze uomini
void Men::buildTree(float tightness,int numvars,char **varDomains){
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
void Men::adjustTightness(float tightness){
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

void Men::DOT_representation(string *res){
	this->prefTree->DOTgraph(res);
}

void Men::DAC(){
	DAC_first_pass(this->prefTree->root);
}

//primo passo DAC che propaga la directed arc consistency
void Men::DAC_first_pass(CTreeNode *node){
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
	//fisso me stesso (figlio) e verifico col padre
	if(node->father==NULL)
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
	}

}


// DAC 2nd pass che estrae l'istanziazione ottima e la sua preferenza, output breadth first

void Men::opt_as_child(CTreeNode *node,int *opt_instance,int *curidx){
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


void Men::opt_as_father(CTreeNode *node,int *opt_instance,int *curidx){
	if(*curidx==0)
		opt_as_child(node,opt_instance,curidx);
	for(int i=0;i<node->child_n;i++)
		opt_as_child(node->children[i],opt_instance,curidx);
	for(int i=0;i<node->child_n;i++)
		opt_as_father(node->children[i],opt_instance,curidx);
}


//per encapsulation devi passare opt_instance giˆ allocato
float Men::DAC_opt(int *opt_instance,int *curidx){
	//prendo il max dei miei unary
	opt_as_father(this->prefTree->root,opt_instance,curidx);
	cout << "Your opt is" << this->prefTree->root->dacUnaryConstraints[opt_instance[0]] << "\n";
	cout << "With solution: ";
	for (int i=0;i<this->prefTree->n_nodes;i++)
		cout << opt_instance[i];
	return this->prefTree->root->dacUnaryConstraints[opt_instance[0]];
}

//FINE DAC 2nd pass
