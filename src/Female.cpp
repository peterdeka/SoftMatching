/*
 * Female.cpp
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#include "Female.h"

Female::Female(int numvars,float connectedness,int domains_sz,char **varDomains,int *instance){
		this->domains_size=domains_sz;
		this->prefGraph=new CTree(numvars,domains_sz);
		this->myInstance=(int*)malloc(numvars*sizeof(int));
		this->numvars=numvars;
		memcpy(this->myInstance,instance,numvars*sizeof(int));
		buildGraph(numvars,connectedness,varDomains);
}

Female::Female(){

}

//generazione grafo preferenze donne
void Female::buildGraph(int numvars,float connectedness,char **varDomains){ //TODO non e tightness altro nome
	CTree *tree=this->prefGraph;
	//int randomVarId= rand() % numvars;
	CTreeNode *root=new CTreeNode(0,varDomains[0]);
	root->genUnaryConstraints(domains_size);
	//ora aggiungo tutte le altre variabili (nodi)
	CTreeNode *node=root;
	node->children=(CTreeNode **)malloc(numvars*sizeof(CTreeNode*));	//inizializzo con max num possibile di bin constr
	tree->setRoot(root);
	for(int i=1;i<numvars;i++){
		CTreeNode* node=new CTreeNode(i,varDomains[i]);
		node->genUnaryConstraints(domains_size);
		node->children=(CTreeNode **)malloc(numvars*sizeof(CTreeNode*));	//inizializzo con max num possibile di bin constr
		tree->addNode(node);
	}
	//ora genero i vincoli binari (non  un albero quindi faccio un sottoprodotto cartesiano)
	//ha rappresentazione albero ma non lo e, conto sul fatto che quando valuto un'istanziazione procedo in ordine
	//sui nodi quindi non mi interessa propagare il binary anche nel figlio (rappresentato directed ma uso come undirected)
	int n_constr=((float)((tree->n_nodes*(tree->n_nodes-1))/2))*connectedness;
	std::cout << "Building women graph with "<<tree->n_nodes<< " nodes and " <<n_constr<<" constraints that's "<<connectedness<<" connected.\n";
	int rndId2;
	while(n_constr>0){
		for(int i=0;i<tree->n_nodes;i++){	//nessun problema con random perche non e albero
			node=tree->linearizedTree[i];
			if(node->child_n>=tree->n_nodes) // verifica che: numero figli attuale<NUMVARS
				continue;

			int nchild;

			if(n_constr==0)
				break;
			else if(n_constr==1)
				nchild=1;
			else
				nchild=(rand()%3+1)%n_constr;

			rndId2=rand()%tree->n_nodes;
			for(int j=0;j<nchild;j++){
				int skip=2;
				while(skip>0){
					skip=0;
					rndId2=(rndId2+1)%tree->n_nodes;
					if(rndId2==i)	//no self constraint
						rndId2=(rndId2+1)%tree->n_nodes;

					for(int h=0;h<node->child_n;h++){	//check it's not already child
						CTreeNode *curchild=node->children[h];
						if(rndId2==curchild->varId){
							skip=2;
							break;
						}

					}
					CTreeNode *rndchild=tree->linearizedTree[rndId2];
					for(int h=0;h<rndchild->child_n;h++){	//check current node it's not chil dof candidate rnd children
						CTreeNode *curchild=rndchild->children[h];
						if(node->varId==curchild->varId){
							skip=2;
							break;
						}

					}

					if(skip>0){
						//std::cout << "SKIP" << skip << '\n';
						//j--;
						continue;
					}
					CTreeNode *child=tree->linearizedTree[rndId2];
					node->children[node->child_n++]=child;
					n_constr-=1;
				}
			}
		}
	}

	//ora che ho messo tutti i legami, genero i binary constraint
	for(int i=0;i<tree->n_nodes;i++){
		CTreeNode *node=tree->linearizedTree[i];
		node->genBinaryConstraints(domains_size);
	}
	std::cout << "Women graph built\n";
}

void Female::DOT_representation(string *res){
	this->prefGraph->DOTgraph(res);
}

//valuta il valore di preferenza di un'istanza (un uomo)
float Female::instance_pref(int *instance){ 	//TODO aggiungi in uscita tupla che da preferenza della soluzione (tupla minima)
	float pref=10.0f;
	for(int i=0;i<prefGraph->n_nodes;i++){
		int curval=instance[i];
		CTreeNode *curnode=prefGraph->linearizedTree[i];
		pref=min(pref,curnode->unaryConstraints[curval]);
		for(int k=0;k<curnode->child_n;k++){
			CTreeNode* curchild=curnode->children[k];
			pref=min(pref,curnode->childConstraints[k][curval][instance[curchild->varId]]);
		}
	}
	return pref;
}

int Female::compare(Male *m1, Male *m2){
	float pref1,pref2;
	pref1=instance_pref(m1->myInstance);
	pref2=instance_pref(m2->myInstance);
	if(pref1==pref2){	//tie
		//rompo tie secondo ordinamento linearizzazione
		for(int i=0;i< numvars;i++){
			if(m1->myInstance[i]>m2->myInstance[i])
				return -1;
			else if(m1->myInstance[i]<m2->myInstance[i])
				return 1;
		}
		return 0;
	}
	else if(pref1>pref2)
		return 1;
	else
		return -1;

}
