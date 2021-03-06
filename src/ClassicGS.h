/*
 * ClassicGS.h
 *
 *  Created on: 16/ott/2013
 *      Author: deka
 *      Classico GS, costruisce prima le liste di preferenze a partire dal pref del SoftGS e poi applica GS classico
 */

#ifndef CLASSICGS_H_
#define CLASSICGS_H_


#include "Male.h"
#include "Female.h"

class Classic_GS {
	int num_individuals;
	Male **men;
	Female **women;
	int **menprefs;	//liste di preferenza di ogni individuo (indice=varid, contenuto=pref)
	float **womenprefs;

public:
	Classic_GS(int num_males, Male** menarray, Female** womenarray);
	virtual ~Classic_GS();
	int gale_shapley_men_opt(int *matching);
};

#endif /* CLASSICGS_H_ */
