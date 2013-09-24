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
#include <fstream>
#include <algorithm>

using namespace std;

#define NUMVARS 10	//numero variabili (quindi nodi dell'albero)
#define DOMAINS_SIZE 2	//dimensione dei domini delle variabili
char varDomains[NUMVARS][DOMAINS_SIZE];	//ogni varId è associato al suo dominio (che è un array di valori)
#define DOT_TABLE_BEG "<<FONT POINT-SIZE=\"10\"><TABLE CELLBORDER=\"0\" CELLPADDING=\"1\" CELLSPACING=\"0\">"
#define DOT_TABLE_END "</TABLE></FONT>>];\n"
#define MALE_TIGHTNESS 0.8	//percentuale di binary constraint NON nulli

class CTree;
//definisce un nodo dell'albero(una variabile quindi)
class CTreeNode{
public:
	CTree *tree;
	int varId;	//nome della variabile
	char* domain;
	float *unaryConstraints;	//constraint unari
	float *dacUnaryConstraints;
	CTreeNode *father;	//pointer al nodo padre
	CTreeNode **children;	//pointer ai nodi figli(array)
	int child_n;	//numero di figli
	float ***childConstraints;	//constraint sui figli(array di array bidimensionali,una matrice per ogni figlio)
	//strutture operative temporanee
	int value;	//quando si vuole assegnare un valore per istanziare
	int prefValue;	//per l'algoritmo alfacut

    	//void addChild (CTreeNode,CBinConstrTable);
    	void genUnaryConstraints();
    	int genBinaryConstraints();
    	void genChildren(int *curVarId);
    	void JSONSubtree(string *res );
    	void DOTSubtree(string *res,char* rootcall );

    CTreeNode(int avarId,char *vardomain) //destructor
    {
    	tree=NULL;
    	varId=avarId;
    	domain=vardomain;
    	unaryConstraints=NULL;
    	dacUnaryConstraints=NULL;
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
    	free(this->dacUnaryConstraints);
    	free(this->children);

    };
};


class CTree{
public:
	CTreeNode *root;
	CTreeNode **linearizedTree;
	int n_nodes;
	int n_bin_constraints;
	float tightness;

	void addNode(CTreeNode *node){
		linearizedTree[n_nodes++]=node;
	}

	void setRoot(CTreeNode *node){
		this->addNode(node);
		this->root=node;
	}

	CTree(){
		root=NULL;
		tightness=1;
		n_bin_constraints=0;
		this->linearizedTree=NULL;
		this->n_nodes=NULL;
	};

	CTree(int nodes_num){
		root=NULL;
		tightness=1;
		n_bin_constraints=0;
		this->linearizedTree=(CTreeNode**)malloc(nodes_num*sizeof(CTreeNode*));
		this->n_nodes=0;
	}

	~CTree() //destructor
	{
	    	free(this->linearizedTree);
	};

	void genChildren( CTreeNode *curNode,int child_limit);
	void DOTgraph(string *res);

};

//genera i figli di un nodo (breadth first) l'ordinamento originale delle variabili
void CTree::genChildren( CTreeNode *curNode,int child_limit){
	int childrenleft=child_limit;
	int child_n=0;
	if(childrenleft<2)
		child_n=(int)((float)rand()/(float)RAND_MAX);
	else
		child_n=rand() % 3;	//quanti figli? (0,1,2)

	curNode->child_n=child_n;
	curNode->children=(CTreeNode**)malloc(child_n*sizeof(CTreeNode*));

	for(int i=0;i<child_n;i++){
		//scelgo random var
		int randomVarId=-1;
		while(randomVarId<0){
			randomVarId=rand()%NUMVARS;
			//vedo se c'è già
			for(int j=0;j<this->n_nodes;j++){
				if(this->linearizedTree[j]->varId==randomVarId)
					randomVarId=-1;
			}
		}
		CTreeNode* childnode=new CTreeNode(randomVarId,varDomains[randomVarId]);
		curNode->children[i]=childnode;
		childnode->father=curNode;
		childnode->genUnaryConstraints();
		this->addNode(childnode);
	}

	this->n_bin_constraints+=curNode->genBinaryConstraints();
}

//produce grafo generico, non assume alcuna ipotesi sia albero (serve per donne)
void CTree::DOTgraph(string *res){
	*res+="digraph {";
	char svarid[650];
	char tmp[150];
	for(int j=0;j<this->n_nodes;j++){
		CTreeNode *curnode=this->linearizedTree[j];
		for(int i=0;i<curnode->child_n;i++){
			//EDGE (Senza semicolon)
			sprintf(svarid,"%d -> %d ",curnode->varId,curnode->children[i]->varId);
			//scrivo tabelle con binary constraints sugli edge
			sprintf(tmp,"[label=%s",DOT_TABLE_BEG);
			strcat(svarid,tmp);
			for(int d=0;d<DOMAINS_SIZE;d++){
				for(int e=0;e<DOMAINS_SIZE;e++){
					sprintf(tmp,"<TR><TD>%c%c</TD><TD>%.1f</TD></TR>",curnode->domain[d],curnode->children[i]->domain[e],curnode->childConstraints[i][d][e]);
					strcat(svarid,tmp);
				}
			}
			strcat(svarid,DOT_TABLE_END);
		///FINE tabella binary constraints
			string ssv(svarid);
		*res+=ssv;
		}
	}
	*res+="}";
}


//METODI CLASSE TREENODE
//genera vincoli unari dato il dominio della variabile
void CTreeNode::genUnaryConstraints(){
	this->unaryConstraints=(float*)malloc(DOMAINS_SIZE*sizeof(float));
	this->dacUnaryConstraints=(float*)malloc(DOMAINS_SIZE*sizeof(float));
	for(int i=0;i<DOMAINS_SIZE;i++){	//assegno valori di preferenza random
		float r = (float)rand()/(float)RAND_MAX;
		if(r==0.0f)
			r+=0.1f;
		this->unaryConstraints[i]=r;
		this->dacUnaryConstraints[i]=r;	//TODO check, questo va bene per inizializzare nodi foglia se non vengono elaborati(?Algo?)
	}
}

//genera vincoli binari sui figli
int CTreeNode::genBinaryConstraints(){
	this->childConstraints=(float***)malloc(this->child_n*sizeof(float**));
	int n_gen_constraints=0;
	for(int u=0;u<this->child_n;u++){	//per ogni figlio
		float** tp=(float**)malloc(DOMAINS_SIZE*DOMAINS_SIZE*sizeof(float*));
		for(int i=0;i<DOMAINS_SIZE;i++){	//assegno valori di preferenza random come prodotto cartesiano
			tp[i]=(float*)malloc(DOMAINS_SIZE*sizeof(float));
			for(int z=0;z<DOMAINS_SIZE;z++){
				float r = (float)rand()/(float)RAND_MAX;
				if(r==0.0f)
					r+=0.1f;
				tp[i][z]=r;
				n_gen_constraints++;
			}
		}
		this->childConstraints[u]=tp;
	}
	return n_gen_constraints;
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
//produce grafo a partire da un nodo, valido solo per alberi(no grafi generici), utile se si vuole sottoalbero specifico
void CTreeNode::DOTSubtree(string *res,char *rootcall){
	char isrootcall=0;
	if(*rootcall==1){
		isrootcall=1;
		*rootcall=0;
		*res+="digraph {";
	}
	char svarid[650];
	char tmp[150];
	for(int i=0;i<this->child_n;i++){
		//EDGE (Senza semicolon)
		sprintf(svarid,"%d -> %d ",this->varId,this->children[i]->varId);
		//scrivo tabelle con binary constraints sugli edge
		sprintf(tmp,"[label=%s",DOT_TABLE_BEG);
		strcat(svarid,tmp);
		for(int d=0;d<DOMAINS_SIZE;d++){
			for(int e=0;e<DOMAINS_SIZE;e++){
				sprintf(tmp,"<TR><TD>%c%c</TD><TD>%.1f</TD></TR>",this->domain[d],this->children[i]->domain[e],this->childConstraints[i][d][e]);
				strcat(svarid,tmp);
			}
		}
		strcat(svarid,DOT_TABLE_END);
		///FINE tabella binary constraints
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
		int charbase= abs((rand() % 24)+97);	//parto da questo char
		int u=0;
		for(u=0;u<DOMAINS_SIZE;u++){
			varDomains[i][u]=(char)(charbase+u);	//e ne metto domains_size
		}
	}
}



//generazione albero preferenze uomini
void buildTree(CTree *tree){

	CTreeNode **curarr=(CTreeNode**)malloc(NUMVARS*sizeof(CTreeNode*));
	CTreeNode **otherarr=(CTreeNode**)malloc(NUMVARS*sizeof(CTreeNode*));
	int randomVarId= rand() % NUMVARS;
	CTreeNode *root=new CTreeNode(randomVarId,varDomains[randomVarId]);
	root->genUnaryConstraints();
	//ora aggiungo tutte le altre variabili (nodi)
	CTreeNode *node=root;
	tree->setRoot(root);
	curarr[0]=root;
	int curlen=1;
	int otherlen=0;
	while(tree->n_nodes<NUMVARS){	//randomizzo discendenza
		for(int i=0;i<curlen;i++){	//elaboro nodi sospesi
			node=curarr[i];
			int child_limit=NUMVARS-tree->n_nodes;
			tree->genChildren(node,child_limit);
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

//generazione grafo preferenze donne
void buildWomenGraph(CTree *tree, float tightness){ //TODO non e tightness altro nome
	int randomVarId= rand() % NUMVARS;
	CTreeNode *root=new CTreeNode(randomVarId,varDomains[randomVarId]);
	root->genUnaryConstraints();
	//ora aggiungo tutte le altre variabili (nodi)
	CTreeNode *node=root;
	tree->setRoot(root);
	while(tree->n_nodes<NUMVARS){
		int randomVarId=-1;
		while(randomVarId<0){
			randomVarId=rand()%NUMVARS;
			//vedo se c'è già
			for(int j=0;j<tree->n_nodes;j++){
			if(tree->linearizedTree[j]->varId==randomVarId)
				randomVarId=-1;
			}
		}
		CTreeNode* node=new CTreeNode(randomVarId,varDomains[randomVarId]);
		node->genUnaryConstraints();
		tree->addNode(node);
	}
	//ora genero i vincoli binari (non è un albero quindi faccio un sottoprodotto cartesiano)
	int n_constr=(float)(NUMVARS*NUMVARS)*tightness;
	int rndId2;
	while(n_constr>0){
		randomVarId=rand()%NUMVARS;
		int nchild=rand()%3;
		node=tree->linearizedTree[randomVarId];
		if(node->child_n>0)	//gia generati per lui TODO occhio che rischi loop infinito
			continue;

		node->children=(CTreeNode **)malloc(nchild*sizeof(CTreeNode*));
		node->child_n=nchild;
		for(int j=0;j<nchild;j++){
			rndId2=rand()%NUMVARS;
			if(rndId2==randomVarId){
				j--;
				continue;
			}
			node->children[j]=tree->linearizedTree[randomVarId];
			node->genBinaryConstraints();
			n_constr-=nchild;
		}
	}
}


//riceve tree e annulla random dei constraint fino a raggiungere il livello di tightness desiderato
void adjustTightness(CTree *tree,float tightness){
	int domains_sz2=DOMAINS_SIZE*DOMAINS_SIZE;
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
		int x=rand() % DOMAINS_SIZE;
		int y=rand() % DOMAINS_SIZE;
		//verifico non sia già nullo
		if(n->childConstraints[chosen_child][x][y]==0){
			i--;
			continue;
		}
		//verifico tavola non diventi totalmente nulla
		int zerocount=0;
		for(int j=0;j<DOMAINS_SIZE;j++){
			for(int jj=0;jj<DOMAINS_SIZE;jj++){
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

}

//TODO NOT COMPLETE NOT CHECKED (ALGO)
/*void DAC_first_pass(CTreeNode * node, float *fatherunary){
	float **myunary;
	if(node->child_n>0){
		myunary=(float**)malloc(node->child_n*sizeof(float*));	//qui accumulo i valori che vengono prodotti dai figli
		for(int i =0;i<node->child_n;i++){
			myunary[i]=(float*)malloc(DOMAINS_SIZE*sizeof(float));
			DAC_first_pass(node->children[i],myunary[i]);
		}
		//per ogni elemento del mio dominio faccio il min per eleggere il valore da sostituire TODO check
		for(int i=0;i<DOMAINS_SIZE;i++){
			float min=node->unaryConstraints[i];
			for(int j=0;j<node->child_n;j++){
				if(myunary[j][i]<min)
					min=myunary[j][i];
			}
			node->dacUnaryConstraints[i]=min;
		}
		if(node->tree->root==node)	//se sono la radice finisco
			return;
	}
	//procedura vera e propria di propagazione bottom-up
	for(int i=0;i<DOMAINS_SIZE;i++){	//fisso la variabile del padre
		float curmax=-1.0f;
		for(int j=0;j<DOMAINS_SIZE;j++){
			float themin;
			if(node->dacUnaryConstraints[j]<=node->father->childConstraints[i][j])
				themin=node->dacUnaryConstraints[j];
			else
				themin=node->father->childConstraints[i][j];
			if(themin>curmax)
				curmax=themin;
		}
		fatherunary[i]=curmax;
	}
}*/


int main() {
	srand((unsigned)time(0));
	buildVarDomains();
	CTree albero(NUMVARS);
	buildTree(&albero);
	adjustTightness(&albero,0.7);

	//CTree albero(NUMVARS);
	//buildWomenGraph(&albero,0.4f);
	string st;
	char rootcall=1;
	//albero.DOTgraph(&st);
	albero.root->DOTSubtree(&st,&rootcall);
	std::cout << st << '\n';
	ofstream myfile;
	myfile.open ("graph.gv");
	myfile << st;
	myfile.close();
	return 0;
}
