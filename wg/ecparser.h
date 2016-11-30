/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library to error-correcting parse a string on a word graph */
#ifndef ECPARSER_INCLUDED
#define ECPARSER_INCLUDED 

#include "dynamic_ordered_list.h"
#include "word_graph.h"
/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define MAX_EDIT_COST 255
#define INSERTION_COST 1
#define DELETION_COST 1
#define SUBSTITUTION_COST(X,Y) (X==Y)?0:1

/* Maximum number of edit operations that a state can be from the best_edit_cost */
#define EDIT_COST_BEAM 5


/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

/** \d Error-correcting data structure */
typedef struct{
   ordered_list_t *active_states;   /**< \d Ordered list of active states */
   unsigned char *in_active_states; /**< \d Flag vector indicating if a state is in the ordered list */
   unsigned char *ec_cost;          /**< \d Array of error-correcting costs, one for each state */
   float *prob;                    /**< \d Array of probability, one for each state */
} ec_state_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Set word graph in error-correcting parser */
void set_word_graph_ecparser(wg_t *wg);
void set_word_graph_online_ecparser(wg_t *wg);
/** \d Set empty symbol id in error-correcting parser. 
    Deletion  cost of empty symbol is ZERO */
void set_empty_symbol_ecparser(int es);

/** \d Set initial symbol id in error-correcting parser.*/
void set_initial_symbol_ecparser(int es);

/** \d Set final symbol id in error-correcting parser.*/
void set_final_symbol_ecparser(int es);

/** \d Start error-correcting parsing from the initial state computing 
    deletion step */
void reset_ecparser(int ECAlg);

/** \d Create a copy of the error-correcting parsing state */
ec_state_t *duplicate_ec_state(ec_state_t *ec_state);

/** \d Destroy error-correcting parsing state */
void destroy_ec_state(ec_state_t **ec_state);

/** \d Set the error-correcting scale factor */
void set_ecsf_ecparser(float ECF);

/** \d Set error-correcting parsing state */
void set_ec_state(ec_state_t *ec_state);

/** \d Return error-correcting parsing state */
ec_state_t *get_ec_state(void);

/** \d Return the probs of the different states*/
float *get_prob_ecparser(void);

/** \d Convert ordered list into a conventional linked list and return it */
list_t *get_ec_state_list(void);

/** \d Compute a deletion step over the word graph */
ec_state_t *compute_deletions_traduccion(ec_state_t *next_ec_state, ordered_list_t *states_to_be_visited_in_deletion_calculation, unsigned char *in_next_active_states, unsigned char best_edit_cost);

/** \d Compute a deletion step over the word graph */
ec_state_t *compute_deletions_transcripcion(ec_state_t *next_ec_state, ordered_list_t *states_to_be_visited_in_deletion_calculation, unsigned char *in_next_active_states, int ECAlg);

/** \d Parse a word in the prefix over the word graph. The output is a list 
    of states from which we can keep searching and their costs */
ec_state_t *advance_parse_traduccion(int word);

/** \d Parse a word in the prefix over the word graph. The output is a list 
    of states from which we can keep searching and their costs */
ec_state_t *advance_parse_transcripcion(int word,int *states_order, int ECAlg);

/** \d Parse a online word graph over the word graph. The output is a list 
    of states from which we can keep searching and the word used to arrive to each state */
ec_state_t *advance_parse_online(int **state_words, int pot);

/** \d We want to know what states are reached with the last word in prefix, but we don't
want to update the parser state, since this word will be replaced by other */
ec_state_t *tentative_advance(int word, int ECAlg);

/** \d Return a list of states with the best minimum edit cost 
    and in case of equal edit cost, those within the same prob beam */
list_t *best_states_after_ec_parsing_transcripcion(float beam);

/** \d Return a list of states with the best minimum edit cost 
    and in case of equal edit cost, those within the same prob beam */
list_t *best_states_after_ec_parsing_traduccion(float beam);


#endif
