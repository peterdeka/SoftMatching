//============================================================================
// Name        : SoftMatching.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

//definisce una tavola di binary constraints che lega valori di preferenza a
//valori della variabile
class CBinaryConstraintTable{
	int *value;	//array di valori possibili del dominio
	float *pref;	//array di preferenze associate
}CTable;

//definisce un nodo dell'albero(una variabile quindi)
class CTreeNode{
	int varId;	//nome della variabile
	float *myConstraints;	//constraint unari
	CTreeNode *father;	//pointer al nodo padre
	CTreeNode **children;	//pointer ai nodi figli(array)
	CTable *childConstraints;	//constraint sui figli(array)
	//strutture operative temporanee
	int value;	//quando si vuole assegnare un valore per istanziare
	int prefValue;	//per l'algoritmo alfacut
}TNode;

int main() {

	return 0;
}
