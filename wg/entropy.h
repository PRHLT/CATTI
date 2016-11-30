/***************************************************************************
                          entropy.h  -  description
                             -------------------
    begin                : March 9 2012
    copyright            : PRHLT - UPV
***************************************************************************/
/** \file
\brief \d This file implements basic functions for compute the entropy of a word graph.

*/
#ifndef ENTROPY_INCLUDED
#define ENTROPY_INCLUDED

#include "word_graph.h"
/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Function that creates the word graph */
int set_input_WG(char *name,float gsf, float wip, wg_t **wg);

/** \d Function that compute the entropy for a given word graph*/
float calcEntropy(wg_t *wg,float factEntropy);


#endif

    