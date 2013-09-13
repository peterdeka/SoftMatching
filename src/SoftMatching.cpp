//============================================================================
// Name        : SoftMatching.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <ctime>
#include <string>
#include <sstream>
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
    	void genChildren(int *curVarId);
    	void JSONSubtree(string *res );
    	void DOTSubtree(string *res,char* rootcall );

    CTreeNode() //destructor
    {
    	varId=-1;
    	unaryConstraints=NULL;
    	father=NULL;
    	children=NULL;
    	child_n=0;
    	childConstraints=NULL;
    	value=-1;
    	prefValue=-1;
    };

    ~CTreeNode() //destructor
    {
    	free(this->unaryConstraints);
    	free(this->children);

    };
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
			tp[i]=(float*)malloc(DOMAINS_SIZE*sizeof(float));
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

//genera i figli di un nodo, complicata perch cosi mantiene (breadth first) l'ordinamento originale delle variabili
void CTreeNode::genChildren( int *curVarId){
	int childrenleft=NUMVARS-(*curVarId+1);
	int child_n=0;
	if(childrenleft==1)
		child_n=(int)((float)rand()/(float)RAND_MAX);
	else
		child_n=rand() % childrenleft;	//quanti figli? TODO modulo 1 e modulo 0

	this->child_n=child_n;
	this->children=(CTreeNode**)malloc(child_n*sizeof(CTreeNode*));

	for(int i=0;i<child_n;i++){
		CTreeNode* childnode=new CTreeNode();
		this->children[i]=childnode;
		childnode->varId=++*curVarId;
		childnode->father=this;
		childnode->genUnaryConstraints();
	}
	if(rand() % 100 >50)	//vedo se generare binary constraints TODO cosi  tutti figli li hanno o nessuno,invece devi differenziare
		this->genBinaryConstraints();
}

void CTreeNode::JSONSubtree(string *res){
	char svarid[10];
	snprintf(svarid,9,"%d",this->varId);
	const char *t=svarid;
	*res+="{varId:";
	string ssv(t);
	*res+=ssv;
	*res+=",children:[";	//TODO varid e constraints
	if(this->child_n>0){
		for(int i=0;i<this->child_n;i++){
			this->children[i]->JSONSubtree(res);
			if(i<this->child_n-1)
				*res+=",";
		}
	}
	*res+="]}";
}

//http://graphs.grevian.org/example.html
void CTreeNode::DOTSubtree(string *res,char *rootcall){
	char isrootcall=0;
	if(*rootcall==1){
		isrootcall=1;
		*rootcall=0;
		*res+="digraph {";
	}
	for(int i=0;i<this->child_n;i++){
		char svarid[20];
		sprintf(svarid,"%d -> %d;",this->varId,this->children[i]->varId);
		string ssv(svarid);
		*res+=ssv;
		this->children[i]->DOTSubtree(res,rootcall);

	}
	if(isrootcall==1)
		*res+="}";
}

//----FINE CLASSE TREENODE

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
	while(curVarId<NUMVARS-2){	//randomizzo discendenza
		for(int i=0;i<curlen;i++){	//elaboro nodi sospesi
			node=curarr[i];
			node->genChildren(&curVarId);
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
}



int main() {
	srand((unsigned)time(0));
	buildVarDomains();
	buildTree();
	string st;
	//root->JSONSubtree(&st);
	char rootcall=1;
	root->DOTSubtree(&st,&rootcall);
	std::cout << st << '\n';
	return 0;
}
