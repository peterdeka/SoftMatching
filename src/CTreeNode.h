/*
 * CTreeNode.h
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#ifndef CTREENODE_H_
#define CTREENODE_H_

#include <string>
#include <vector>
#include "CTree.h"
#define DOT_TABLE_BEG "<<FONT POINT-SIZE=\"10\"><TABLE CELLBORDER=\"0\" CELLPADDING=\"1\" CELLSPACING=\"0\">"
#define DOT_TABLE_END "</TABLE></FONT>>];\n"
using namespace std;
class CTree;
//definisce un nodo dell'albero(una variabile quindi)
class CTreeNode{

public:
	//
	CTree *tree;
	int varId;	//nome della variabile
	char* domain;
	int domains_sz;
	float *unaryConstraints;	//constraint unari
	float *dacUnaryConstraints;
	CTreeNode *father;	//pointer al nodo padre
	CTreeNode **children;	//pointer ai nodi figli(array)
	int child_n;	//numero di figli
	float ***childConstraints;	//constraint sui figli(array di array bidimensionali,una matrice per ogni figlio)
	float **fatherConstraints; //punta alla tabella binary constraint tra me e mio padre (punta dentro childConstraints del padre)
	//strutture operative temporanee
	int value;	//quando si vuole assegnare un valore per istanziare
	int prefValue;	//per l'algoritmo alfacut
   	// float *weightedUnaryConstr; //ok unary are considered 0 cost
    float ***weightedChildConstr;
   // float **weightedFatherConstr;
    //float **unaryBucket;
    //int ***messages;	//constiene stringhe parziale con soluzione per i costi dell'unary bucket
    vector <vector<int*> > messages;
    vector <vector<float> > unaryBucket;
   // float ****binBucket;
    int n_in_bucket;
    	//void addChild (CTreeNode,CBinConstrTable);
    	virtual ~CTreeNode();
		void genUnaryConstraints(int domains_size);
    	int genBinaryConstraints(int domains_size);
    	void genChildren(int *curVarId);
    	void JSONSubtree(string *res );
    	void DOTSubtree(string *res,char* rootcall,int domains_size );
    	CTreeNode(int avarId,char *vardomain);
    	void alloc_weighted_tables();

};



#endif /* CTREENODE_H_ */
