
/***************************************************************************
                          entropy.c  -  description
                             -------------------
    begin                : March 9 2012
    copyright            : PRHLT - UPV
***************************************************************************/
/** \file
\brief \d This file implements basic functions for compute the entropy of a word graph.

*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <values.h>

#include "global.h"
#include "auxiliar.h"
#include "vector.h"
#include "dynamic_list.h"
#include "dynamic_ordered_list.h"
#include "hash.h"
#include "dictionary.h"
#include "heap.h"
#include "librefx.h"
#include "word_graph.h"
#include "sfst.h"
#include "ecparser.h"
#include "nbestsuffix.h"
#include "CAT.h"
#include "zipio.h"
#include "strop.h"
#include "entropy.h"


#define FALSE 0
#define TRUE 1


/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Function that creates the word graph */
int set_input_WG(char *name,float gsf, float wip, wg_t **wg){
  
//  wg_t *wg;
  int empty_symbol;
  int initial_symbol;
  int final_symbol;


/*********************************************************************************/
/* DESTROY DATA STRUCTURES THAT WERE CREATED TO COMPUTE THE ENTROPY IN THE PREVIOUS SENTENCE: */
/*********************************************************************************/
   destroy_word_graph(wg);

   

/**************************************************************/
/* OBTAIN WORD GRAPH TO PERFORM OPERATIONS ON IT AND PREPARING IT*/
/**************************************************************/
  //Create lat	
   *wg = lat_create();
  //Copy the information of the file in the structure lat        
   lat_load(*wg,zopen(name,"r"));
 
/* Let's assign to each state, its topological order. This operation MUST be carried out after setting the word graph */
   topo_order_word_graph(*wg);
     /* Set empty symbol in word graph */
   empty_symbol = get_id_dict((*wg)->dict,"!NULL");
   set_empty_symbol_word_graph(empty_symbol);

  /*Set the initial symbol in word graph */
   initial_symbol = get_id_dict((*wg)->dict,"<s>");
   set_initial_symbol_word_graph(initial_symbol);

  /*Set the final symbol in word graph */
   final_symbol = get_id_dict((*wg)->dict,"</s>");
   if(final_symbol<0) final_symbol=set_token_dict((*wg)->dict,"</s>");
   set_final_symbol_word_graph(final_symbol);

  /* Set the GSF and the WIP values*/
   if(gsf!=MAXFLOAT) set_GSF_word_graph(gsf,*wg);
   if(wip!=MAXFLOAT) set_WIP_word_graph(wip,*wg);
   

   compute_log_probability(*wg);
   /*sort the word graph dictionary*/ 
   sort_dict((*wg)->dict); 


 /* Perform initial backward search over the word graph */
 // initialize_nbestsuffix_search(); MIRAR BACKWARD A LA VITERBI
/****************************************************************/

  return 1; 
  
  
}


/** \d Function that compute the entropy for a given word graph*/
float calcEntropy(wg_t *wg, float factEntropy){
 double valor=0.0;
 double entrop=0.0;
 double *forward=NULL;
 int i,e;
 edge_t *curr_edge;
    
 mt(forward = (double *) malloc(wg->size * sizeof(double)));
 for(i=0;i<wg->size;i++) forward[i]=LOG(0.0);
    
 if(factEntropy!=log(0.0)) 
   normalizeWG(wg,factEntropy);
 computeForward(forward,wg);
 
 for(i=0;i<wg->size;i++){
    for (e=0;e<wg->wg[i].to.l;e++){
        curr_edge = wg->wg[i].to.edv[e];
	valor=exp(curr_edge->prob)*curr_edge->prob*exp(forward[curr_edge->from]);
	entrop=entrop+valor;
	//printf("%e,%e,%e,%e,%e,%d,%d\n",exp(curr_edge->prob),curr_edge->prob,exp(forward[curr_edge->from]),forward[curr_edge->from],valor,i,e);
    }
 }
 return -entrop;
  
}
