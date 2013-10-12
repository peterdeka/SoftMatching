/*
 * Women.cpp
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#include "Women.h"

Women::Women(int numvars,float connectedness,int domains_sz,char **varDomains){
		this->domains_size=domains_sz;
		this->prefGraph=new CTree(numvars,domains_sz);
		buildGraph(numvars,connectedness,varDomains);
}

//generazione grafo preferenze donne
void Women::buildGraph(int numvars,float connectedness,char **varDomains){ //TODO non e tightness altro nome
	CTree *tree=this->prefGraph;
	//int randomVarId= rand() % numvars;
	CTreeNode *root=new CTreeNode(0,varDomains[0]);
	root->genUnaryConstraints(domains_size);
	//ora aggiungo tutte le altre variabili (nodi)
	CTreeNode *node=root;
	node->children=(CTreeNode **)malloc(tree->n_nodes*sizeof(CTreeNode*));	//inizializzo con max num possibile di bin constr
	tree->setRoot(root);
	for(int i=1;i<numvars;i++){
		CTreeNode* node=new CTreeNode(i,varDomains[i]);
		node->genUnaryConstraints(domains_size);
		node->children=(CTreeNode **)malloc(tree->n_nodes*sizeof(CTreeNode*));	//inizializzo con max num possibile di bin constr
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
						j--;
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

void Women::DOT_representation(string *res){
	this->prefGraph->DOTgraph(res);
}
