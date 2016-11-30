/***************************************************************************
                          CATTI.h  -  description
                             -------------------
    begin                : Sun May 9 2004
    copyright            : (C) 2004 by Instituto Tecnologico de Informatica ***************************************************************************/
/** \file
\brief \d This file implements basic functions for computer-assisted transcription of handwritten text images.

*/
#ifndef CATTI_INCLUDED
#define CATTI_INCLUDED

#include "vector.h"
#include "global.h"
//#include "ecparser.h"
/*****************************************************************************/
/*******************************VARIABLE**************************************/
/*****************************************************************************/




/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Function that creates the word graph and initialize the ec parser and the n-best
suffix algorithm. We have destroyed any previous data structure employed in the
previous input sentence */
int set_input_sentence(char *name,float gsf, float wip,float ECF, wg_t **wg, int ECAlg);

/** \d Complete the prefix (prepro_prefix) by concatenating n-best suffices 

Given a prefix in its char version, that is, prepro_prefix, and its integer version 
int_prefix and last_word_in_prefix, that is, a pointer to a possible incomplete 
word at the end of the prepro_prefix:

  -# Obtain best states after error-correcting parsing.
  -# Check if we are able to complete the last word in the prefix, if so then 
     compute a step of error-correcting using this word and update best states.
    -# Check if last_word_in_prefix is found on any outgoing edge of best states.
    -# If not, search last_word_in_prefix in word graph vocabulary.
    -# If not, search last_word_in_prefix in transducer vocabulary.
    -# If not, consider last_word_in_prefix to be an isolated word.
  -# Compute n-best paths from best states to obtain n-best suffices.
  -# We have the prepro_prefix, the first_word_in_suffix and the n-best suffices, 
     so let's put them together to construct completions (full sentences).
*/

int set_online_word(char *path,char *prev, char *word, char *error,wg_t **wg);
int set_input_unigram(char *name,wg_t **wg);

int cost(int ref, int hip, dict_t *wg_dict,dict_t *unigram_dict);

completions_t complete_prefix(char *prefix, int *int_prefix, char *last_word_in_prefix, int number_of_completions, dict_t  *outdict, float beam,vector_t *first_word_last_suffix, int multimodality,int pot,unsigned char* errorCorrecting,wg_t *wg,wg_t *unigram,int ECAlg);

#endif
