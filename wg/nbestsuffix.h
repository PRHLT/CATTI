/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library to compute n-best paths in a word graph */
#ifndef NBESTSUFFIX_INCLUDED
#define NBESTSUFFIX_INCLUDED

#include "heap.h"

/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

/** \d N-best hypothesis at each state to link this state with next state thru the edge 
    next_edge in the best path best_path_in_next_state */
typedef struct {  
  float prob;                 /**< \d Probability of current n-best path using this n-best hyp */
  edge_t *next_edge;           /**< \d Next edge in current n-best path using this n-best_hyp */
  int best_path_in_next_state; /**< \d Best path index in next state involved in current n-best path */
} nbest_hyp_t;

/** \d This data structure stores the information about the state of 
    the n-best algorithm for each state in the word graph */
typedef struct {
  vector_t *list;        /**< \d Best paths that have been already extracted from the heap of candidates */
  heap_t *candidates;    /**< \d Candidates for next best paths that can be exploited */
  unsigned char *shared; /**< \d Flag vector indicating if the hyp on the top of the candidate heap is shared by the n-best list */ 
} nbest_state_t;

/** \d This data structure stores the information about the state of the search while 
    we are computing n-best paths from the list of initial states in the word graph */
typedef struct {
  int *num_paths; /**< \d Number of n-best paths extracted for each state in the word graph for the current search */
  list_t *initial_states; /**< \d States from which the nbest search will be performed */
} nbest_search_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Set word graph in n-best search */
void set_word_graph_nbestsuffix(wg_t *wg);

/** \d Set empty symbol id in n-best search
    empty symbol will be ignored when generating best path */
void set_empty_symbol_nbestsuffix(int es);

/** \d Set initial symbol id in error-correcting parser.*/
void set_initial_symbol_nbestsuffix(int es);

/** \d Set final symbol id in error-correcting parser.*/
void set_final_symbol_nbestsuffix(int es);

/** \d This function initializes the data structures for the n-best state and 
    search. It is supposed to be called only once for each source sentence */
void create_nbestsuffix(void);

/** Setting states that will be the initial states to perform the n-best 
    suffix search and initialize num_paths computed for each state */
void set_initial_states_nbestsuffix(list_t *states_reached_after_ecparsing);

/** Initial backward pass to create 1-best paths */
void initialize_nbestsuffix_search(void);

/** Return the next best suffix */
float get_next_nbestsuffix(vector_t *suffix,float *prev_probs,vector_t *if_fwils, int *state_words, int multimodality,int *state,char *last_word_in_prefix,vector_t *alignment);

/** Destroy n-best state and search */
void destroy_nbestsuffix(void);

/** Destroy initial_states list by emptying it  */
void destroy_initial_states_nbestsuffix(void);

/** Get the best probs from every state */
float *get_prob_nbestsuffix(void);

#endif
