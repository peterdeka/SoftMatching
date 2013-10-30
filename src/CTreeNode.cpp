/*
 * CTreeNode.cpp
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#include "CTreeNode.h"

CTreeNode::CTreeNode(int avarId,char *vardomain) //constructor
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
    	fatherConstraints=NULL;
    	value=-1;
    	prefValue=-1;
    }

/*CTreeNode::~CTreeNode() //destructor
    {
    	free(this->unaryConstraints);
    	free(this->dacUnaryConstraints);
    	free(this->children);

    }*/

//genera vincoli unari dato il dominio della variabile
void CTreeNode::genUnaryConstraints(int domains_size){
	this->unaryConstraints=(float*)malloc(domains_size*sizeof(float));
	this->dacUnaryConstraints=(float*)malloc(domains_size*sizeof(float));
	for(int i=0;i<domains_size;i++){	//assegno valori di preferenza random
		float r = (float)rand()/(float)RAND_MAX;
		if(r<0.1f)
			r+=0.1f;
		this->unaryConstraints[i]=r;
		this->dacUnaryConstraints[i]=r;
	}
}

//genera vincoli binari sui figli
int CTreeNode::genBinaryConstraints(int domains_size){
	this->childConstraints=(float***)malloc(this->child_n*sizeof(float**));
	int n_gen_constraints=0;
	for(int u=0;u<this->child_n;u++){	//per ogni figlio
		float** tp=(float**)malloc(domains_size*domains_size*sizeof(float*));
		for(int i=0;i<domains_size;i++){	//assegno valori di preferenza random come prodotto cartesiano
			tp[i]=(float*)malloc(domains_size*sizeof(float));
			for(int z=0;z<domains_size;z++){
				float r = (float)rand()/(float)RAND_MAX;
				if(r<0.1f)
					r+=0.1f;
				tp[i][z]=r;
				n_gen_constraints++;
			}
		}
		this->childConstraints[u]=tp;
		this->children[u]->fatherConstraints=tp;
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
void CTreeNode::DOTSubtree(string *res,char *rootcall,int domains_size){
	char isrootcall=0;
	if(*rootcall==1){
		isrootcall=1;
		*rootcall=0;
		*res+="digraph { node [fontsize=10];";
	}
	char svarid[650];
	char tmp[150];
	sprintf(svarid,"%d [label=\"{",this->varId);
	for(int i=0;i<domains_size;i++){
		sprintf(tmp,"%c:%.2f->%.2f,",this->domain[i],this->unaryConstraints[i],this->dacUnaryConstraints[i]);
		strcat(svarid,tmp);
	}
	svarid[strlen(svarid)-1]='\0';
	strcat(svarid,"}\"]; ");
	string ssv(svarid);
	*res+=ssv;
	for(int i=0;i<this->child_n;i++){
		//EDGE (Senza semicolon)
		sprintf(svarid,"%d -> %d ",this->varId,this->children[i]->varId);
		//scrivo tabelle con binary constraints sugli edge
		sprintf(tmp,"[label=%s",DOT_TABLE_BEG);
		strcat(svarid,tmp);
		for(int d=0;d<domains_size;d++){
			for(int e=0;e<domains_size;e++){
				sprintf(tmp,"<TR><TD>%c%c</TD><TD>%.2f</TD></TR>",this->domain[d],this->children[i]->domain[e],this->childConstraints[i][d][e]);
				strcat(svarid,tmp);
			}
		}
		strcat(svarid,DOT_TABLE_END);
		///FINE tabella binary constraints
		string ssv(svarid);
		*res+=ssv;
		this->children[i]->DOTSubtree(res,rootcall,domains_size);

	}
	if(isrootcall==1)
		*res+="}";
}
