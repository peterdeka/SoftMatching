/*
 * CTree.cpp
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#include "CTree.h"

void CTree::addNode(CTreeNode *node){
		//linearizedTree[node->varId]=node;
		//n_nodes++;
		linearizedTree[n_nodes++]=node;
	}

	void CTree::setRoot(CTreeNode *node){
		this->addNode(node);
		this->root=node;
	}

	CTree::CTree(){
		root=NULL;
		tightness=1;
		n_bin_constraints=0;
		this->linearizedTree=NULL;
		this->n_nodes=NULL;
		domains_size=0;
	}

	CTree::CTree(int nodes_num,int domainssize){
		root=NULL;
		tightness=1;
		n_bin_constraints=0;
		this->linearizedTree=(CTreeNode**)malloc(nodes_num*sizeof(CTreeNode*));
		this->n_nodes=0;
		domains_size=domainssize;
	}

	/*CTree::~CTree() //destructor
	{
	    	free(this->linearizedTree);
	}*/

//genera i figli di un nodo (breadth first) l'ordinamento originale delle variabili
void CTree::genChildren( CTreeNode *curNode,int child_limit, char **varDomains,int numvars){
	int childrenleft=child_limit;
	int child_n=0;
	if(childrenleft<2)
		child_n=(int)((float)rand()/(float)RAND_MAX);
	else
		child_n=rand() % 3;	//quanti figli? (0,1,2)

	curNode->child_n=child_n;
	curNode->children=(CTreeNode**)malloc(child_n*sizeof(CTreeNode*));

	for(int i=0;i<child_n;i++){
		/*//scelgo random var
		int randomVarId=-1;
		while(randomVarId<0){
			randomVarId=rand()%numvars;
			//vedo se c' giˆ

			if(this->linearizedTree[randomVarId]!=NULL)
				randomVarId=-1;
		}*/
		//CTreeNode* childnode=new CTreeNode(randomVarId,varDomains[randomVarId]);
		CTreeNode* childnode=new CTreeNode(n_nodes,varDomains[n_nodes]);
		curNode->children[i]=childnode;
		childnode->father=curNode;
		childnode->genUnaryConstraints(this->domains_size);
		this->addNode(childnode);
	}

	this->n_bin_constraints+=curNode->genBinaryConstraints(this->domains_size);
}

//produce grafo generico, non assume alcuna ipotesi sia albero (serve per donne)
void CTree::DOTgraph(string *res){
	cout << "Generating DOTgraph representation \n";
	*res+="digraph {";
	char svarid[650];
	char tmp[150];
	for(int j=0;j<this->n_nodes;j++){
		CTreeNode *curnode=this->linearizedTree[j];
		sprintf(svarid,"%d;",curnode->varId);
		string ssv(svarid);
		*res+=ssv;
		for(int i=0;i<curnode->child_n;i++){
			//EDGE (Senza semicolon)
			sprintf(svarid,"%d -> %d ",curnode->varId,curnode->children[i]->varId);
			//scrivo tabelle con binary constraints sugli edge
			sprintf(tmp,"[label=%s",DOT_TABLE_BEG);
			strcat(svarid,tmp);
			for(int d=0;d<domains_size;d++){
				for(int e=0;e<domains_size;e++){
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
