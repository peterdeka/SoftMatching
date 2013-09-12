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
/*class CBinConstrTable{
	char *value;	//array di valori possibili del dominio
	float *pref;	//array di preferenze associate
};
*/

//definisce un nodo dell'albero(una variabile quindi)
class CTreeNode{
public:
	int varId;	//nome della variabile
	float *unaryConstraints;	//constraint unari
	CTreeNode *father;	//pointer al nodo padre
	CTreeNode **children;	//pointer ai nodi figli(array)
	int child_n;	//numero di figli
	float ***childConstraints;	//constraint sui figli(array di array bidimensionali,una matrice per ogni figlio)
	//strutture operative temporanee
	int value;	//quando si vuole assegnare un valore per istanziare
	int prefValue;	//per l'algoritmo alfacut

    	//void addChild (CTreeNode,CBinConstrTable);
    	void genUnaryConstraints();
    	void genBinaryConstraints();
};



//genera vincoli unari dato il dominio della variabile
void CTreeNode::genUnaryConstraints(){
	this->unaryConstraints=(float*)malloc(DOMAINS_SIZE*sizeof(float));
	for(int i=0;i<DOMAINS_SIZE;i++){	//assegno valori di preferenza random
		float r = (float)rand()/(float)RAND_MAX;
		if(r==0.0f)
			r+=0.1f;
		this->unaryConstraints[i]=r;
	}
}

//genera vincoli binari sui figli
void CTreeNode::genBinaryConstraints(){
	this->childConstraints=(float***)malloc(this->child_n*sizeof(float**));

	for(int u=0;u<this->child_n;u++){	//per ogni figlio
		float** tp=(float**)malloc(DOMAINS_SIZE*DOMAINS_SIZE*sizeof(float*));
		for(int i=0;i<DOMAINS_SIZE;i++){	//assegno valori di preferenza random come prodotto cartesiano
			for(int z=0;z<DOMAINS_SIZE;z++){
				float r = (float)rand()/(float)RAND_MAX;
				if(r==0.0f)
					r+=0.1f;
				tp[i][z]=r;
			}
		}
		this->childConstraints[u]=tp;
	}
}


//funzione che costruisce i domini delle variabili a caso
void buildVarDomains(){

	for(int i=0;i<NUMVARS;i++){
		int charbase= rand() % 254-DOMAINS_SIZE;	//parto da questo char
		int u=0;
		for(u=0;u<DOMAINS_SIZE;u++){
			varDomains[i][u]=(char)(charbase+u);	//e ne metto domains_size
		}
	}
}

//genera i figli di un nodo, complicata perch cosi mantiene (breadth first) l'ordinamento originale delle variabili
void genChilds(CTreeNode *n, int *curVarId){
	int child_n=rand() % (NUMVARS-(*curVarId+1));	//quanti figli?
	n->child_n=child_n;
	n->children=(CTreeNode**)malloc(child_n*sizeof(CTreeNode*));

	for(int i=0;i<child_n;i++){
		CTreeNode* childnode=new CTreeNode();
		n->children[i]=childnode;
		childnode->varId=++*curVarId;
		childnode->father=n;
		childnode->genUnaryConstraints();
	}
	if(rand() % 100 >50)	//vedo se generare binary constraints TODO cpsi  tutti figli li hanno o nessuno,invece devi differenziare
		n->genBinaryConstraints();
}

CTreeNode *root;

void buildTree(){
	CTreeNode **curarr=(CTreeNode**)malloc(NUMVARS*sizeof(CTreeNode*));
	CTreeNode **otherarr=(CTreeNode**)malloc(NUMVARS*sizeof(CTreeNode*));

	root=new CTreeNode();
	root->varId=0;
	root->genUnaryConstraints();
	//ora aggiungo tutte le altre variabili (nodi)
	CTreeNode *node=root;
	int curVarId=0;	//tiene traccia identificativo variabile per generazione ordinata
	curarr[0]=root;
	int curlen=1;
	int otherlen=0;
	while(curVarId<=NUMVARS){	//randomizzo discendenza
		for(int i=0;i<curlen;i++){	//elaboro nodi sospesi
			node=curarr[i];
			genChilds(node,&curVarId);
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
}



int main() {
	srand((unsigned)time(0));
	buildVarDomains();
	buildTree();

	return 0;
}
