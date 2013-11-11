/*
 * ClassicGSNext.h
 *
 *  Created on: 02/nov/2013
 *      Author: deka
 *      Gale Shapley classico che prima costruisce le liste di preferenze utilizzando il next del softGS e poi applica gs classico
 */

#ifndef CLASSICGSNEXT_H_
#define CLASSICGSNEXT_H_
#include "Male.h"
#include "Female.h"
#include "FemaleContainer.h"

class ClassicGSNext {
	Male **men;
		Female **women;
		int **menprefs;	//liste di preferenza di ogni individuo (indice=varid, contenuto=pref)
		float **womenprefs;
		int num_individuals;
		FemaleContainer *womencont;
		int find_female_with_instance(int *instance);
public:
	ClassicGSNext(int num_males, Male** menarray, Female** womenarray);
	int gale_shapley_men_opt(int *matching);
	virtual ~ClassicGSNext();
};

#endif /* CLASSICGSNEXT_H_ */
