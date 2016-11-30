/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Include the set of functions used by the GUI */
#ifndef LIBREFX_INCLUDED
#define LIBREFX_INCLUDED

#include "vector.h"
#include "ecparser.h"
#include "global.h"
/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Set maximum number of completions to be returned  */
int ipeSetMaxCompletions(int max_comps);

/** \d Initialize prediction engine by loading the parameter files,
     the transducer is loaded into memory in this function */
int ipeInitialisePE(const char *params);

/** \d This function reads the wordsentences to be translated */
int ipeLoadwordgraphs(char *names);
    
/** \d This function reads the sentences to be translated */
int ipeLoadText(char *source);

/** \d This function reads the reference translations */
int ipeLoadReference(char *reference);

/** \d This function sets the sentence to be translated */
int ipeSetSourceText (const char* source);

/** \d This function sets the user prefix */
int ipeSetTargetText(const char* target);

/** \d This function returns the best completions given the position of the cursor 
    in the user prefix */
int ipeProvideCompletions(completions_t *completions, int cursorPosition,float beam,vector_t *first_word_last_suffix,int multimodality,int pot,unsigned char *errorCorrecting,wg_t *wg,wg_t *unigram,int ECAlg);

void posproceso(char *best_completions,char *pospro);

#endif
