/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file
\brief \d Library to perform a set of operations on a word graph */
#ifndef WORD_GRAPH_INCLUDED
#define WORD_GRAPH_INCLUDED 


#include <stdio.h>
#include <stdlib.h>
#include "dictionary.h"
#include "dynamic_list.h"
#include "vector.h"
#include "auxiliar.h"
/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define INITIAL_STATE 0

/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

/** \d Word graph edge data structure */
typedef struct {
    int from;    /**< \d Id of target state */
    int to;      /**< \d Id of source state */
    int output;  /**< \d Id of output symbol */
    double hmm_prob;  /**< \d HMM log probability */
    double lm_prob;  /**< \d LM log probability */
    double prob;  /**< \d log probability */
} edge_t;

/** \d Word graph edges data structure */
typedef struct edge_vector_t{
  int l;        /**< \d Number of edges */
  edge_t **edv; /**< \d Array of edges */
} edv_t;

/** \d Word graph state data structure */
typedef struct {
    edv_t to;     /**< \d Array of outgoing edges */
    edv_t from;   /**< \d Array of incoming edges */
    double time; /**< \d Time corresponding to this state */
    float final;  /**< \d Final state log probability */
} state_t;

/** \d Word graph data structure */
typedef struct {
    state_t *wg;       /**< \d Array of states, that is, the word graph itself */
    dict_t *dict;      /**< \d Dictionary to handle word graph vocabulary */
    int *state_order;  /**< \d Array indicating the order of each state */
    int size;          /**< \d Number of states in word graph */
    float GSF;           /**< \d Grammar scale factor used to construct the word graph */
    float WIP;           /**< \d Word Insertion Penalty used to construct the word graph */ 
} wg_t;



/***************************** Functions for lat ****************************/
#include "lat-parser/lat-driver.h"
/** \d Save in the word_graph the informacion of the file */
void lat_save(FILE *file,wg_t *wg);

/** \d Create and initiate the word_graph */
wg_t * lat_create();
//wg_t * lat_create_online();
//wg_t * lat_create_unigram();

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/



/** \d Set word graph data structure  */
//void set_word_graph(wg_t *wg);

/** \d Set the unigram data structure  */
//void set_unigram(wg_t *unigram);

/** \d Set empty symbol id in error-correcting parser. 
    Deletion  cost of empty symbol is ZERO */
void set_empty_symbol_word_graph(int es);

/** \d Set initial symbol id in error-correcting parser.*/
void set_initial_symbol_word_graph(int es);

/** \d Set final symbol id in error-correcting parser.*/
void set_final_symbol_word_graph(int es);

/** \d Returns the word graph data structure */
//wg_t *get_word_graph(void);

/** \d Returns the dictionary data structure */
//dict_t *get_word_graph_dict(wg_t *wg);

/** \d Returns the dictionary of the unigram data structure */
//dict_t *get_unigram_dict(void);

/** \d Returns the initial symbol data structure */
int get_word_graph_initial_symbol(void);

/** \d Returns the final symbol data structure */
int get_word_graph_final_symbol(void);

/** \d This function computes the topological order of each state and store that 
    information in the word graph data structure. This algorithm was extracted 
    from "Efficient Error-Correcting Parsing" by J.C. Amengual and E. Vidal */
void topo_order_word_graph(wg_t *wg);

//void topo_order_online_word_graph(void);

/** \d Compute the log probability for each edge*/
void compute_log_probability(wg_t *wg);

/** \d Extract vocabulary from word graph and sort it for binary search */
void extract_and_sort_dict_word_graph(dict_t *outdict,wg_t *wg);

/** \d Perform binary search in word graph dictionary */
int search_in_dict_word_graph(dict_t *outdict, char *word,wg_t* wg,vector_t *first_word_last_suffix);

/** \d This function returns (if any) a word found on the outgoing edges of 
    the states in the state_list that match (as prefix) the "word" */
int search_in_state_edges_word_graph(list_t *state_list, char *word, int word_length, dict_t *outdict,wg_t *wg,vector_t *first_word_last_suffix,float *probs,float *probs_suffix);

/** \d Function that free all data structures associated with the word graph */
void destroy_word_graph(wg_t **wg);

//void destroy_online_word_graph(void);

//void destroy_unigram(void);

/** \d This function generates a file containing the word graph in v2 format */
void word_graph2v2_format_file_simple(dict_t *outdict,wg_t *wg);

/** \d Set the GSF value */
void set_GSF_word_graph(float gsf,wg_t *wg);

/** \d Set the WIP value */
void set_WIP_word_graph(float wip,wg_t *wg);


/** \d This function compute the forward probability for each state */
void computeForward(double *forward,wg_t *wg);

/** \d This function compute the forward probability for each state */
void computeBackward(double *backward,wg_t *wg);


/** \d normalize the word graph wg */
void normalizeWG(wg_t *wg,float factEntropy);

#endif
