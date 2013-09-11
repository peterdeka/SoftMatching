//============================================================================
// Name        : SoftMatching.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include<ctime>
using namespace std;

#define NUMVARS 10	//numero variabili (quindi nodi dell'albero)
#define DOMAINS_SIZE 2	//dimensione dei domini delle variabili
char varDomains[NUMVARS][DOMAINS_SIZE];	//ogni varId  associato al suo dominio (che  un array di valori)



//definisce una tavola di binary constraints che lega valori di preferenza a
//valori della variabile
class CBinConstrTable{
	char *value;	//array di valori possibili del dominio
	float *pref;	//array di preferenze associate
};

//definisce un nodo dell'albero(una variabile quindi)
class CTreeNode{
	int varId;	//nome della variabile
	float *unaryConstraints;	//constraint unari
	CTreeNode *father;	//pointer al nodo padre
	CTreeNode **children;	//pointer ai nodi figli(array)
	int child_n;	//numero di figli
	CBinConstrTable *childConstraints;	//constraint sui figli(array)
	//strutture operative temporanee
	char value;	//quando si vuole assegnare un valore per istanziare
	int prefValue;	//per l'algoritmo alfacut
	public:
    	void addChild (CTreeNode,CBinConstrTable);
    	void genUnaryConstraints(char* vardomain);
    	void CTreeNode::genBinaryConstraints(char** vardomain);
};

void CTreeNode::addChild(CTreeNode node,CBinConstrTable constraints){

}

//genera vincoli unari dato il dominio della variabile
void CTreeNode::genUnaryConstraints(char* vardomain){
	this->unaryConstraints=malloc(DOMAINS_SIZE*sizeof(float));
	int i=0;
	for(i=0;i<DOMAINS_SIZE;i++){	//assegno valori di preferenza random
		float r = (float)rand()/(float)RAND_MAX;
		if(r==0.0f)
			r+=0.1f;
		this->unaryConstraints[i]=r;
	}
}

//genera vincoli binari sui figli dato l'array coi domini(tutti, lui sceglie in base a varid del figlio)
void CTreeNode::genBinaryConstraints(char** vardomain){
	this->childConstraints=malloc(this->child_n*sizeof(CBinConstrTable*));
	int u=0;
	for(u=0;u<this->child_n;u++){	//per ogni figlio
		int i=0;
		for(i=0;i<DOMAINS_SIZE;i++){	//assegno valori di preferenza random
			float r = (float)rand()/(float)RAND_MAX;
			if(r==0.0f)
				r+=0.1f;
			this->unaryConstraints[i]=r;
		}
	}
}


//funzione che costruisce i domini delle variabili a caso
void buildVarDomains(){
	int i=0;
	for(i=0;i<NUMVARS;i++){
		int charbase= rand() % 254-DOMAINS_SIZE;	//parto da questo char
		int u=0;
		for(u=0;u<DOMAINS_SIZE;u++){
			varDomains[i][u]=(char)(charbase+u);	//e ne metto domains_size
		}
	}
}


CTreeNode *root;

void buildTree(){
	root=new CTreeNode();
	root->varId=0;
	root->genUnaryConstraints(varDomains[root->varId]);
	//ora aggiungo tutte le altre variabili (nodi)
	int i=1;
	CTreeNode *prevNode=root;
	for (i=1;i<NUMVARS;i++){
		//randomizzo discendenza


	}
}

//genera i figli di un nodo, complicata perch cosi mantiene (breadth first) l'ordinamento originale delle variabili
void genChilds(CTreeNode *n, int *curVarId){
	int child_n=rand() % (NUMVARS-(*curVarId+1));	//quanti figli?
	n->child_n=child_n;
	n->children=malloc(child_n*sizeof(CTreeNode*));
	int i=0;
	for(i=0;i<child_n;i++){
		CTreeNode* childnode=new CTreeNode();
		n->children[i]=childnode;
		childnode->varId=++*curVarId;
		childnode->father=n;
		childnode->genUnaryConstraints(varDomains[childnode->varId]);

	}
}

int main() {
	srand((unsigned)time(0));
	buildVarDomains();
	buildTree();

	return 0;
}
