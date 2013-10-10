/*
 * CTree.h
 *
 *  Created on: 10/ott/2013
 *      Author: deka
 */

#ifndef CTREE_H_
#define CTREE_H_

#include <iostream>
#include <string>
#include "CTreeNode.h"
#define DOT_TABLE_BEG "<<FONT POINT-SIZE=\"10\"><TABLE CELLBORDER=\"0\" CELLPADDING=\"1\" CELLSPACING=\"0\">"
#define DOT_TABLE_END "</TABLE></FONT>>];\n"

class CTreeNode;
using namespace std;
class CTree{
public:
	//
	CTreeNode *root;
	CTreeNode **linearizedTree;
	int n_nodes;
	int n_bin_constraints;
	float tightness;
	int domains_size;


	CTree();
	CTree(int nodes_num,int domainssize);
	void addNode(CTreeNode *node);
	void setRoot(CTreeNode *node);
	void genChildren( CTreeNode *curNode,int child_limit,char **varDomains,int numvars);
	void DOTgraph(string *res);

};


#endif /* CTREE_H_ */
