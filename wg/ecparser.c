/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library to error-correcting parse a string on a word graph */
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "global.h"
#include "auxiliar.h"
#include "dynamic_list.h"
#include "dynamic_ordered_list.h"
#include "hash.h"
#include "dictionary.h"
#include "word_graph.h"
#include "ecparser.h"
#include "strop.h"
#include "CAT.h"

/*****************************************************************************/
/*******************************VARIABLES*************************************/
/*****************************************************************************/

/** \d Word graph */
static wg_t *word_graph=NULL;
static wg_t *word_graph_online=NULL;

/** \d Default empty symbol whose deletion cost is ZERO */
static int empty_symbol=INT_MAX;
/** \d Default initial symbol*/
static int initial_symbol=INT_MAX;
/** \d Default final symbol*/
static int final_symbol=INT_MAX;

/** \d Error-correcting parser state, it stores the information related what 
    states are going to be explored and their best edit cost to reach any of them 
    and their probability */
static ec_state_t *next_ec_state=NULL; 


/**\d Error-correcting scale factor */
static float ecsf;

/****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Set word graph in error-correcting parser */
void set_word_graph_ecparser(wg_t *wg){
  word_graph=wg;
}

/***************************************************************************/
/** \d Set word graph online in error-correcting parser */
void set_word_graph_online_ecparser(wg_t *wg){
  word_graph_online=wg;
}

/***************************************************************************/
/** \d Set empty symbol id in error-correcting parser. 
    Deletion  cost of empty symbol is ZERO */
void set_empty_symbol_ecparser(int es){
  empty_symbol=es;
}
/***************************************************************************/
/** \d Set initial symbol id in error-correcting parser.*/
void set_initial_symbol_ecparser(int es){
  initial_symbol=es;
}

/***************************************************************************/
/** \d Set final symbol id in error-correcting parser.*/
void set_final_symbol_ecparser(int es){
  final_symbol=es;
}
/***************************************************************************/
/** \d Set the error-correcting scale factor */
void set_ecsf_ecparser(float ECF){
  ecsf=ECF;
}

/***************************************************************************/
/** \d Initialize error-correcting data structure */
ec_state_t *create_ec_state(void){

  ec_state_t *ec_state;
  int i;

  mt(ec_state = (ec_state_t *) malloc(sizeof(ec_state_t)));

  ec_state->active_states = create_forward_ordered_list(word_graph->state_order, word_graph->size);

  /* We haven't visited any state yet */
  mt(ec_state->in_active_states = (unsigned char *) malloc(word_graph->size*sizeof(unsigned char)));
  memset(ec_state->in_active_states, 0, word_graph->size*sizeof(unsigned char));

  mt(ec_state->ec_cost = (unsigned char *)malloc(word_graph->size*sizeof(unsigned char)));
  memset(ec_state->ec_cost, MAX_EDIT_COST, word_graph->size*sizeof(unsigned char));

  mt(ec_state->prob = (float *)malloc(word_graph->size*sizeof(float)));
  for(i=0;i<word_graph->size;i++)
    ec_state->prob[i] = LOG(0.0);

  return ec_state;
}

/***************************************************************************/
/** \d Start error-correcting parsing from the initial state computing 
    deletion step */
void reset_ecparser(int ECAlg){

ordered_list_t *initial_states_to_be_visited;
unsigned char *in_initial_active_states;

/* Creating vector that will contain the edit cost for each state in the word graph */
next_ec_state = create_ec_state();

/* The initial state has edit cost ZERO */
next_ec_state->ec_cost[istate] = 0;

/* The initial state prob is LOG(1.0) == 0 */
next_ec_state->prob[istate] = 0.0;

/* Create orderedlogical list to store initial state */
initial_states_to_be_visited = create_forward_ordered_list(word_graph->state_order, word_graph->size);
mt(in_initial_active_states = (unsigned char *) malloc(word_graph->size*sizeof(unsigned char)));

/* Store initial state in list and set belonging vector */
insert_into_ordered_list(initial_states_to_be_visited, istate, (void *) istate);
in_initial_active_states[istate] = 1;

/* Store initial state in list and set belonging vector */
insert_into_ordered_list(next_ec_state->active_states, istate, (void *) istate);
next_ec_state->in_active_states[istate] = 1;

if(tipo_CAT==TRANSCRIPCION)
     next_ec_state = compute_deletions_transcripcion(next_ec_state, initial_states_to_be_visited,  in_initial_active_states,ECAlg);
if(tipo_CAT==TRADUCCION)
     next_ec_state = compute_deletions_traduccion(next_ec_state, initial_states_to_be_visited,  in_initial_active_states, MAX_EDIT_COST);


}

/***************************************************************************/
/** \d Create a copy of the error-correcting parsing state */
ec_state_t *duplicate_ec_state(ec_state_t *ec_state){

  ec_state_t *new_ec_state;

  mt(new_ec_state = (ec_state_t *) malloc(sizeof(ec_state_t)));

  new_ec_state->active_states = duplicate_ordered_list(ec_state->active_states, word_graph->state_order, word_graph->size);

  /* We haven't visited any state yet */
  mt(new_ec_state->in_active_states = (unsigned char *) malloc(word_graph->size*sizeof(unsigned char)));
  memcpy(new_ec_state->in_active_states, ec_state->in_active_states, word_graph->size*sizeof(unsigned char));

  mt(new_ec_state->ec_cost = (unsigned char *)malloc(word_graph->size*sizeof(unsigned char)));
  memcpy(new_ec_state->ec_cost, ec_state->ec_cost, word_graph->size*sizeof(unsigned char));

  mt(new_ec_state->prob = (float *)malloc(word_graph->size*sizeof(float)));
  memcpy(new_ec_state->prob, ec_state->prob, word_graph->size*sizeof(float));
  
  return new_ec_state;
}

/***************************************************************************/
/** \d Destroy error-correcting parsing state */  
void destroy_ec_state(ec_state_t **ec_state){

  if(*ec_state!=NULL){
    destroy_ordered_list((*ec_state)->active_states);
    free((*ec_state)->in_active_states);
    free((*ec_state)->ec_cost);
    free((*ec_state)->prob);
    free(*ec_state);
    *ec_state = NULL;
  } 
}
/***************************************************************************/

/** \d Set error-correcting parsing state */
void set_ec_state(ec_state_t *ec_state){
 
  destroy_ec_state(&next_ec_state);

  next_ec_state = ec_state;
}

/***************************************************************************/
/** \d Return error-correcting parsing state */
ec_state_t *get_ec_state(void){
  return next_ec_state;
}

/***************************************************************************/
/** \d Return the probs of the different states*/
float *get_prob_ecparser(void){
 int i;
 float * probs;

 mt(probs = (float *)malloc(word_graph->size*sizeof(float)));
 for(i=0;i<word_graph->size;i++)
    probs[i]=next_ec_state->prob[i];

 return probs;
}

/***************************************************************************/
/** \d Convert ordered list into a conventional linked list and return it */
list_t *get_ec_state_list(void){
  return create_list_from_ordered_list(next_ec_state->active_states);
}

/***************************************************************************/
/** \d Compute a deletion step over the word graph */
ec_state_t *compute_deletions_transcripcion(ec_state_t *next_ec_state, ordered_list_t *states_to_be_visited_in_deletion_calculation, unsigned char *in_next_active_states,int ECAlg){

/* State id */
int curr_state_id,e;
/* Edit cost when calculating deletion operation */
unsigned char curr_edge_del_cost;
float curr_edge_del_prob;
/* Current edge :-) */
edge_t *curr_edge;

/* CALCULATING DELETION COST */
while(!empty_forward_ordered_list(states_to_be_visited_in_deletion_calculation)){

  curr_state_id = (int) extract_from_forward_ordered_list(states_to_be_visited_in_deletion_calculation);

  for (e=0;e<word_graph->wg[curr_state_id].to.l;e++){
    curr_edge = word_graph->wg[curr_state_id].to.edv[e];

    /* Calculating substitution cost */
    curr_edge_del_cost = (curr_edge->output == empty_symbol)?0:DELETION_COST;
    //curr_edge_del_cost = (curr_edge->output == empty_symbol)?0:strlen(get_token_dict(word_graph->dict,curr_edge->output));
    curr_edge_del_cost += next_ec_state->ec_cost[curr_state_id];
    

    /* Calculating substitution prob */
    //curr_edge_del_prob = curr_edge->prob + next_ec_state->prob[curr_state_id];
    if(ECAlg==0){
      if(curr_edge->output != empty_symbol)  curr_edge_del_prob = next_ec_state->prob[curr_state_id]+(strlen(get_token_dict(word_graph->dict,curr_edge->output))*ecsf)+curr_edge->prob;   
      else curr_edge_del_prob = next_ec_state->prob[curr_state_id]+curr_edge->prob;
    }
    else {
      if(ECAlg==1) curr_edge_del_prob = next_ec_state->prob[curr_state_id] + curr_edge->prob + ecsf;
      else curr_edge_del_prob = next_ec_state->prob[curr_state_id] + curr_edge->prob;
    }

    /* Did this state pass the EDIT BEAM ? */
   // if((curr_edge_del_cost - best_edit_cost) < EDIT_COST_BEAM){

      /* Let's see if the deletion cost of remaining in the same state is good enough */
    if(((ECAlg==2) && ((next_ec_state->ec_cost[curr_edge->to] > curr_edge_del_cost) || 
                       ((next_ec_state->ec_cost[curr_edge->to] == curr_edge_del_cost ) && (next_ec_state->prob[curr_edge->to] < curr_edge_del_prob))))  || 
      ((ECAlg==0 || ECAlg==1) && (next_ec_state->prob[curr_edge->to] < curr_edge_del_prob)))
        {
      
  
//       if((next_ec_state->ec_cost[curr_edge->to] > curr_edge_del_cost)||
//       ((next_ec_state->ec_cost[curr_edge->to] == curr_edge_del_cost) &&
//       (next_ec_state->prob[curr_edge->to] < curr_edge_del_prob))){

    //if(next_ec_state->prob[curr_edge->to] < curr_edge_del_prob){

        /* Updating edit cost for this state */
        if(next_ec_state->ec_cost[curr_edge->to] > curr_edge_del_cost) next_ec_state->ec_cost[curr_edge->to] = curr_edge_del_cost;

        /* Updating edit cost for this state */
        next_ec_state->prob[curr_edge->to] = curr_edge_del_prob;
        
       if(!next_ec_state->in_active_states[curr_edge->to]){

          /* Inserting state to be visited the next time */
          insert_into_ordered_list(next_ec_state->active_states, curr_edge->to, (void *) curr_edge->to);
          next_ec_state->in_active_states[curr_edge->to] = 1;

          /* Inserting state to be visited the next time */
          insert_into_ordered_list(states_to_be_visited_in_deletion_calculation, curr_edge->to, (void *) curr_edge->to);
          in_next_active_states[curr_edge->to] = 1;

        }

        /* Just in case we need to update the best_edit_cost */
        //best_edit_cost = (curr_edge_del_cost<best_edit_cost)?curr_edge_del_cost:best_edit_cost;

      }

    //}/* End of if((curr_edge_del_cost - best_edit_cost) < EDIT_COST_BEAM){ */

  }/* End of while((curr_edge = read_from_list(&edge_list))!=NULL){ */

}/* End of while((curr_state_id = (int) extract_from_ordered_list(states_to_be_visited_in_deletion_calculation, word_graph->state_order))!=NULL){ */

destroy_ordered_list(states_to_be_visited_in_deletion_calculation);
free(in_next_active_states);


return next_ec_state;
}

/***************************************************************************/
/** \d Compute a deletion step over the word graph */
ec_state_t *compute_deletions_traduccion(ec_state_t *next_ec_state, ordered_list_t *states_to_be_visited_in_deletion_calculation, unsigned char *in_next_active_states, unsigned char best_edit_cost){

/* State id */
int curr_state_id,e;
/* Edit cost when calculating deletion operation */
unsigned char curr_edge_del_cost;
float curr_edge_del_prob;
/* Current edge :-) */
edge_t *curr_edge;

/* CALCULATING DELETION COST */
while(!empty_forward_ordered_list(states_to_be_visited_in_deletion_calculation)){

  curr_state_id = (int) extract_from_forward_ordered_list(states_to_be_visited_in_deletion_calculation);

  for (e=0;e<word_graph->wg[curr_state_id].to.l;e++){
    curr_edge = word_graph->wg[curr_state_id].to.edv[e];

    /* Calculating substitution cost */
    curr_edge_del_cost = (curr_edge->output == empty_symbol)?0:DELETION_COST;
    //curr_edge_del_cost = (curr_edge->output == empty_symbol)?0:strlen(get_token_dict(word_graph->dict,curr_edge->output));
    //curr_edge_del_cost += next_ec_state->ec_cost[curr_state_id];
    

    /* Calculating substitution prob */
    curr_edge_del_prob = curr_edge->prob + next_ec_state->prob[curr_state_id];
    //curr_edge_del_prob = next_ec_state->prob[curr_state_id];   

    /* Did this state pass the EDIT BEAM ? */
   // if((curr_edge_del_cost - best_edit_cost) < EDIT_COST_BEAM){

      /* Let's see if the deletion cost of remaining in the same state is good enough */
      if((next_ec_state->ec_cost[curr_edge->to] > curr_edge_del_cost)||
      ((next_ec_state->ec_cost[curr_edge->to] == curr_edge_del_cost) &&
      (next_ec_state->prob[curr_edge->to] < curr_edge_del_prob))){

        /* Updating edit cost for this state */
        next_ec_state->ec_cost[curr_edge->to] = curr_edge_del_cost;

        /* Updating edit cost for this state */
        next_ec_state->prob[curr_edge->to] = curr_edge_del_prob;
        
       if(!next_ec_state->in_active_states[curr_edge->to]){

          /* Inserting state to be visited the next time */
          insert_into_ordered_list(next_ec_state->active_states, curr_edge->to, (void *) curr_edge->to);
          next_ec_state->in_active_states[curr_edge->to] = 1;

          /* Inserting state to be visited the next time */
          insert_into_ordered_list(states_to_be_visited_in_deletion_calculation, curr_edge->to, (void *) curr_edge->to);
          in_next_active_states[curr_edge->to] = 1;

        }

        /* Just in case we need to update the best_edit_cost */
        best_edit_cost = (curr_edge_del_cost<best_edit_cost)?curr_edge_del_cost:best_edit_cost;

      }

    //}/* End of if((curr_edge_del_cost - best_edit_cost) < EDIT_COST_BEAM){ */

  }/* End of while((curr_edge = read_from_list(&edge_list))!=NULL){ */

}/* End of while((curr_state_id = (int) extract_from_ordered_list(states_to_be_visited_in_deletion_calculation, word_graph->state_order))!=NULL){ */

destroy_ordered_list(states_to_be_visited_in_deletion_calculation);
free(in_next_active_states);

return next_ec_state;
}

/***************************************************************************/
/** \d Parse a word in the prefix over the word graph. The output is a list 
    of states from which we can keep searching and their costs */
//ec_state_t *advance_parse(int word){

ec_state_t *advance_parse_traduccion(int word){
/* State id */
int curr_state_id;
/* Edit cost of remaining in the same state, eating the word being parsed */
unsigned char curr_state_ins_cost;
/* Edit cost when comparing the current word with the symbol in the word graph edge */
unsigned char curr_edge_subs_cost;
/* Best edit cost so far */
unsigned char best_edit_cost = MAX_EDIT_COST;
float curr_state_ins_prob;
float curr_edge_subs_prob;
char *pal;
/* Current edge :-) */
edge_t *curr_edge;

/* A vector to check what state has already been inserted in the list */
unsigned char *in_next_active_states;
/* This list is a copy of next_ec_state->active_state to be used in computing
deletion cost */
ordered_list_t *states_to_be_visited_in_deletion_calculation;
int e;
/* Current ec state parsing is the static next_ec_state */
ec_state_t *curr_ec_state = next_ec_state;

/* States that will be visited when calculating deletion */
states_to_be_visited_in_deletion_calculation = create_forward_ordered_list(word_graph->state_order, word_graph->size);

/* We haven't visited any state yet */
mt(in_next_active_states = (unsigned char *) malloc(word_graph->size*sizeof(unsigned char)));
memset(in_next_active_states, 0, word_graph->size*sizeof(unsigned char));

next_ec_state = create_ec_state();

/* CALCULATING INSERTION AND SUBSTITUTION COSTS */
while(!empty_forward_ordered_list(curr_ec_state->active_states)){

  curr_state_id = (int) extract_from_forward_ordered_list(curr_ec_state->active_states);

  /* For every state we calculate its insertion cost, before looking at any edge */
  if(tipo_CAT==TRANSCRIPCION)
     curr_state_ins_cost = curr_ec_state->ec_cost[curr_state_id] + strlen(get_token_dict(word_graph->dict,word));
  if(tipo_CAT==TRADUCCION)
     curr_state_ins_cost = curr_ec_state->ec_cost[curr_state_id] + INSERTION_COST;

  /* As we stay in the same state, the prob is the same */
  curr_state_ins_prob = curr_ec_state->prob[curr_state_id];
  if(tipo_CAT==TRANSCRIPCION)
      curr_state_ins_prob = curr_state_ins_prob + word_graph->WIP;


  /* Did this state pass the EDIT BEAM ? */
 // if((curr_state_ins_cost - best_edit_cost) < EDIT_COST_BEAM){
    
    /* Let's see if the insertion cost of remaining in the same state is good enough */
    if((next_ec_state->ec_cost[curr_state_id] > curr_state_ins_cost)||
    ((next_ec_state->ec_cost[curr_state_id] == curr_state_ins_cost) &&
    (next_ec_state->prob[curr_state_id] < curr_state_ins_prob))){

      /* Updating edit cost for this state */
      next_ec_state->ec_cost[curr_state_id] = curr_state_ins_cost;
      
      /* Updating prob for this state */
      next_ec_state->prob[curr_state_id] = curr_state_ins_prob;

      if(!next_ec_state->in_active_states[curr_state_id]){
        
        /* Inserting state to be visited the next time */
        insert_into_ordered_list(next_ec_state->active_states, curr_state_id, (void *) curr_state_id);
        next_ec_state->in_active_states[curr_state_id] = 1;

        /* Inserting states to be visited during deletion calculation */
        insert_into_ordered_list(states_to_be_visited_in_deletion_calculation, curr_state_id, (void *) curr_state_id);
        in_next_active_states[curr_state_id] = 1;
        
      }
      
      /* Just in case we need to update the best_edit_cost */
      best_edit_cost = (curr_state_ins_cost<best_edit_cost)?curr_state_ins_cost:best_edit_cost;

    }/* End of if(next_ec_state->ec_cost[curr_state_id] > ){ */
      
 // }/* End of if((curr_state_ins_cost - best_edit_cost) < EDIT_COST_BEAM){ */

  
  /* For every edge let's calculate its substitution cost */
  for (e=0;e<word_graph->wg[curr_state_id].to.l;e++){
    curr_edge = word_graph->wg[curr_state_id].to.edv[e];

    
    /* Lambda edge is ignored at the moment to be analyze when calculating deletion cost */
    if ((curr_edge->output != empty_symbol) && (curr_edge->output != initial_symbol) && (curr_edge->output != final_symbol)) {

      pal=get_token_dict(word_graph->dict,curr_edge->output);
      /* Calculating substitution cost */
      if(tipo_CAT==TRANSCRIPCION)
          curr_edge_subs_cost = char_leven_dist(get_token_dict(word_graph->dict, word),get_token_dict(word_graph->dict,curr_edge->output));
      if(tipo_CAT==TRADUCCION)
          curr_edge_subs_cost = SUBSTITUTION_COST(word,curr_edge->output);
      curr_edge_subs_cost += curr_ec_state->ec_cost[curr_state_id];      

      /* Calculating prob */
      curr_edge_subs_prob = curr_edge->prob + curr_ec_state->prob[curr_state_id];

      /* Did this state pass the EDIT BEAM ? */
 //     if((curr_edge_subs_cost - best_edit_cost) < EDIT_COST_BEAM){
        
        /* Let's see if the substitution cost of remaining in the same state is good enough */
        if((next_ec_state->ec_cost[curr_edge->to] > curr_edge_subs_cost)||
        ((next_ec_state->ec_cost[curr_edge->to] == curr_edge_subs_cost) &&
        (next_ec_state->prob[curr_edge->to] < curr_edge_subs_prob))){

          /* Updating edit cost for this state */
          next_ec_state->ec_cost[curr_edge->to] = curr_edge_subs_cost;
          
          /* Updating prob for this state */
          next_ec_state->prob[curr_edge->to] = curr_edge_subs_prob;
          
          if(!next_ec_state->in_active_states[curr_edge->to]){

            /* Inserting state to be visited the next time */
            insert_into_ordered_list(next_ec_state->active_states, curr_edge->to, (void *) curr_edge->to);
            next_ec_state->in_active_states[curr_edge->to] = 1;
            
            /* Inserting states to be visited during deletion calculation */
            insert_into_ordered_list(states_to_be_visited_in_deletion_calculation, curr_edge->to, (void *) curr_edge->to);
            in_next_active_states[curr_edge->to] = 1;
            
          }
          
          /* Just in case we need to update the best_edit_cost */
  //        best_edit_cost = (curr_edge_subs_cost<best_edit_cost)?curr_edge_subs_cost:best_edit_cost;

        }

     // }/* End of if((curr_edge_subs_cost - best_edit_cost) < EDIT_COST_BEAM){ */

    }/* End of if (curr_edge->output != empty_symbol){ */    
    
  }/* End of while((curr_edge = read_from_list(&edge_list))!=NULL){ */  
  
}/* End of while((curr_state_id = (int) extract_from_ordered_list(curr_ec_state->active_states, word_graph->state_order))!=NULL){ */

next_ec_state = compute_deletions_traduccion(next_ec_state, states_to_be_visited_in_deletion_calculation, in_next_active_states, best_edit_cost);

destroy_ec_state(&curr_ec_state);

return next_ec_state;

}

/***************************************************************************/
/** \d Parse a word in the prefix over the word graph. The output is a list 
    of states from which we can keep searching and their costs */
//ec_state_t *advance_parse(int word){

ec_state_t *advance_parse_online(int **state_words,int pot){
/* State id */
int curr_state_id;
int id_word;
int find=0;
float curr_edge_subs_prob,curr_state_ins_prob;
float *online_prob;
int i=0;
char *word;
edge_t *curr_edge;
edge_t *next_curr_edge;
unsigned char *in_next_active_states; /* A vector to check what state has already been inserted in the list */
int e;
int entero;

ec_state_t *curr_ec_state = next_ec_state;/* Current ec state parsing is the static next_ec_state */
ordered_list_t *states_to_be_visited_in_deletion_calculation;

states_to_be_visited_in_deletion_calculation = create_forward_ordered_list(word_graph->state_order, word_graph->size);
/* We haven't visited any state yet */
mt(in_next_active_states = (unsigned char *) malloc(word_graph->size*sizeof(unsigned char)));
memset(in_next_active_states, 0, word_graph->size*sizeof(unsigned char));

/*The words of the online wordgraph that are not in the offline wordgraph are included in the dict*/
for(i=0;i<word_graph_online->dict->size;i++){
   word=get_token_dict(word_graph_online->dict,i);
   set_token_dict(word_graph->dict,word);
}

/*The vector online_prob is generated and initialized*/
mt(online_prob = (float *) malloc(word_graph->dict->size*sizeof(float)));
for(i=0;i<word_graph->dict->size;i++){
    online_prob[i]=LOG(0.0);
}
/*The vector that store the word used to arrive to each state is generated and initialized*/
//if (*state_words != NULL) free(*state_words);
mt((* state_words) = (int *) malloc(word_graph->size*sizeof(int)));
memset((*state_words), NO_WORD, word_graph->size*sizeof(int));

/*The online probability is computed*/
/*First we look for the first state*/
for(i=0;i<word_graph->size && !find;i++){
 curr_state_id=i;
 for (e=0;e<word_graph_online->wg[curr_state_id].to.l;e++){
   curr_edge = word_graph_online->wg[curr_state_id].to.edv[e];
   if (get_id_dict(word_graph->dict,get_token_dict(word_graph_online->dict,curr_edge->output)) == initial_symbol){ 
      curr_state_id=curr_edge->to;
      find=1;
      break;
   }
 }
}
 
/*We compute the probability for each posible word*/
for (e=0;e<word_graph_online->wg[curr_state_id].to.l;e++){
    curr_edge = word_graph_online->wg[curr_state_id].to.edv[e];
    next_curr_edge=word_graph_online->wg[curr_edge->to].to.edv[0];
    id_word=get_id_dict(word_graph->dict,get_token_dict(word_graph_online->dict,curr_edge->output));
    if(get_id_dict(word_graph->dict,get_token_dict(word_graph_online->dict,next_curr_edge->output))==final_symbol){
       entero=pot*(curr_edge->hmm_prob+word_graph_online->GSF*curr_edge->lm_prob);
       online_prob[id_word]=entero;
    }
}
 

next_ec_state = create_ec_state();

while(!empty_forward_ordered_list(curr_ec_state->active_states)){
  curr_state_id = (int) extract_from_forward_ordered_list(curr_ec_state->active_states);

  for(id_word=0; id_word<word_graph->dict->size;id_word++){
    if(online_prob[id_word]>LOG(0.0)){
    
      /* As we stay in the same state, the prob is the same */
      curr_state_ins_prob = curr_ec_state->prob[curr_state_id];
      curr_state_ins_prob = curr_state_ins_prob + word_graph->WIP + (strlen(get_token_dict(word_graph->dict,id_word))*ecsf)+ online_prob[id_word] ;

      if(next_ec_state->prob[curr_state_id] < curr_state_ins_prob){
         /* Updating prob for this state */
         next_ec_state->prob[curr_state_id] = curr_state_ins_prob;
         (*state_words)[curr_state_id]=id_word;

         if(!next_ec_state->in_active_states[curr_state_id]){
        
            /* Inserting state to be visited the next time */
            insert_into_ordered_list(next_ec_state->active_states, curr_state_id, (void *) curr_state_id);
            next_ec_state->in_active_states[curr_state_id] = 1;

            /* Inserting states to be visited during deletion calculation */
            insert_into_ordered_list(states_to_be_visited_in_deletion_calculation, curr_state_id, (void *) curr_state_id);
            in_next_active_states[curr_state_id] = 1;
         }
       }/* End of if(next_ec_state->ec_cost[curr_state_id] > ){ */
      
      /* For every edge let's calculate its substitution cost */
      for (e=0;e<word_graph->wg[curr_state_id].to.l;e++){
         curr_edge = word_graph->wg[curr_state_id].to.edv[e];
         /* Lambda edge is ignored at the moment to be analyze when calculating deletion cost */
         if ((curr_edge->output != empty_symbol) && (curr_edge->output != initial_symbol) && (curr_edge->output != final_symbol)) {
             /* Calculating prob */
             curr_edge_subs_prob = curr_edge->prob + curr_ec_state->prob[curr_state_id] + online_prob[id_word] + (char_leven_dist(get_token_dict(word_graph->dict, id_word),get_token_dict(word_graph->dict,curr_edge->output))*ecsf);

            if(next_ec_state->prob[curr_edge->to] < curr_edge_subs_prob){
            /* Updating edit cost for this state */
          
             /* Updating prob for this state */
              next_ec_state->prob[curr_edge->to] = curr_edge_subs_prob;
              (*state_words)[curr_edge->to]=id_word;
          
              if(!next_ec_state->in_active_states[curr_edge->to]){

                /* Inserting state to be visited the next time */
                insert_into_ordered_list(next_ec_state->active_states, curr_edge->to, (void *) curr_edge->to);
                next_ec_state->in_active_states[curr_edge->to] = 1;
            
                /* Inserting states to be visited during deletion calculation */
                insert_into_ordered_list(states_to_be_visited_in_deletion_calculation, curr_edge->to, (void *) curr_edge->to);
                in_next_active_states[curr_edge->to] = 1;
             }
           }
        }/* End of if (curr_edge->output != empty_symbol){ */    
     }/* End of while((curr_edge = read_from_list(&edge_list))!=NULL){ */  
   }
  }
} 
next_ec_state = compute_deletions_transcripcion(next_ec_state, states_to_be_visited_in_deletion_calculation, in_next_active_states,0);

destroy_ec_state(&curr_ec_state);

return next_ec_state;

}

/***************************************************************************/
/** \d Parse a word in the prefix over the word graph. The output is a list 
    of states from which we can keep searching and their costs */
//ec_state_t *advance_parse(int word){

ec_state_t *advance_parse_transcripcion(int word,int *states_order,int ECAlg){
/* State id */
int curr_state_id;
float curr_state_ins_prob;
float curr_edge_subs_prob;
edge_t *curr_edge;
/* Edit cost of remaining in the same state, eating the word being parsed */
unsigned char curr_state_ins_cost;
/* Edit cost when comparing the current word with the symbol in the word graph edge */
unsigned char curr_edge_subs_cost;

/* A vector to check what state has already been inserted in the list */
unsigned char *in_next_active_states;
/* This list is a copy of next_ec_state->active_state to be used in computing
deletion cost */
ordered_list_t *states_to_be_visited_in_deletion_calculation;
int e;
/* Current ec state parsing is the static next_ec_state */

ec_state_t *curr_ec_state = next_ec_state;

/* States that will be visited when calculating deletion */
states_to_be_visited_in_deletion_calculation = create_forward_ordered_list(word_graph->state_order, word_graph->size);

/* We haven't visited any state yet */
mt(in_next_active_states = (unsigned char *) malloc(word_graph->size*sizeof(unsigned char)));
memset(in_next_active_states, 0, word_graph->size*sizeof(unsigned char));

next_ec_state = create_ec_state();

/* CALCULATING INSERTION AND SUBSTITUTION COSTS */
while(!empty_forward_ordered_list(curr_ec_state->active_states)){

  curr_state_id = (int) extract_from_forward_ordered_list(curr_ec_state->active_states);

  /* For every state we calculate its insertion cost, before looking at any edge */
//   if(tipo_CAT==TRANSCRIPCION)
//     curr_state_ins_cost = curr_ec_state->ec_cost[curr_state_id] + strlen(get_token_dict(word_graph->dict,word));
//   if(tipo_CAT==TRADUCCION)
  curr_state_ins_cost = curr_ec_state->ec_cost[curr_state_id] + INSERTION_COST;

  /* As we stay in the same state, the prob is the same */
  curr_state_ins_prob = curr_ec_state->prob[curr_state_id];
  //if(tipo_CAT==TRANSCRIPCION)
  if(ECAlg==0) curr_state_ins_prob = curr_state_ins_prob + word_graph->WIP + (strlen(get_token_dict(word_graph->dict,word))*ecsf) ;
  else {
    if(ECAlg==1) curr_state_ins_prob = curr_state_ins_prob + word_graph->WIP + ecsf;
    else curr_state_ins_prob = curr_state_ins_prob + word_graph->WIP;
  }
      
     

  /* Did this state pass the EDIT BEAM ? */
 // if((curr_state_ins_cost - best_edit_cost) < EDIT_COST_BEAM){
    
    /* Let's see if the insertion cost of remaining in the same state is good enough */
//     if((next_ec_state->ec_cost[curr_state_id] > curr_state_ins_cost)||
//     ((next_ec_state->ec_cost[curr_state_id] == curr_state_ins_cost) &&
//     (next_ec_state->prob[curr_state_id] < curr_state_ins_prob))){
   
 
   if(((ECAlg==2) && ((next_ec_state->ec_cost[curr_state_id] > curr_state_ins_cost) || 
		    ((next_ec_state->ec_cost[curr_state_id] == curr_state_ins_cost) && (next_ec_state->prob[curr_state_id] < curr_edge_subs_prob)))) ||
      ((ECAlg==0 || ECAlg==1) && (next_ec_state->prob[curr_state_id] < curr_state_ins_prob)))
   {
 
 
 
   //if(next_ec_state->prob[curr_state_id] < curr_state_ins_prob){
      /* Updating edit cost for this state */
      if(next_ec_state->ec_cost[curr_state_id] > curr_state_ins_cost) next_ec_state->ec_cost[curr_state_id] = curr_state_ins_cost;
      
      /* Updating prob for this state */
      next_ec_state->prob[curr_state_id] = curr_state_ins_prob;

      if(!next_ec_state->in_active_states[curr_state_id]){
        
        /* Inserting state to be visited the next time */
        insert_into_ordered_list(next_ec_state->active_states, curr_state_id, (void *) curr_state_id);
        next_ec_state->in_active_states[curr_state_id] = 1;

        /* Inserting states to be visited during deletion calculation */
        insert_into_ordered_list(states_to_be_visited_in_deletion_calculation, curr_state_id, (void *) curr_state_id);
        in_next_active_states[curr_state_id] = 1;
        
      }
      
      /* Just in case we need to update the best_edit_cost */
     // best_edit_cost = (curr_state_ins_cost<best_edit_cost)?curr_state_ins_cost:best_edit_cost;

    }/* End of if(next_ec_state->ec_cost[curr_state_id] > ){ */
      
 // }/* End of if((curr_state_ins_cost - best_edit_cost) < EDIT_COST_BEAM){ */

  
  /* For every edge let's calculate its substitution cost */
  for (e=0;e<word_graph->wg[curr_state_id].to.l;e++){
    curr_edge = word_graph->wg[curr_state_id].to.edv[e];

    
    /* Lambda edge is ignored at the moment to be analyze when calculating deletion cost */
    //if ((curr_edge->output != empty_symbol) && (curr_edge->output != initial_symbol) && (curr_edge->output != final_symbol)) {
    if ((curr_edge->output != empty_symbol) &&  (curr_edge->output != final_symbol)) { 
       /* Calculating substitution cost */
      //if(tipo_CAT==TRANSCRIPCION)
      //    curr_edge_subs_cost = char_leven_dist(get_token_dict(word_graph->dict, word),get_token_dict(word_graph->dict,curr_edge->output));
      //if(tipo_CAT==TRADUCCION)
      curr_edge_subs_cost = SUBSTITUTION_COST(word,curr_edge->output);
      curr_edge_subs_cost += curr_ec_state->ec_cost[curr_state_id];      

      /* Calculating prob */
      /*curr_edge_subs_prob = curr_edge->prob + curr_ec_state->prob[curr_state_id] + (char_leven_dist(get_token_dict(word_graph->dict, word),get_token_dict(word_graph->dict,curr_edge->output))*ecsf);*/

      if(ECAlg==0) curr_edge_subs_prob = curr_edge->prob + curr_ec_state->prob[curr_state_id] + (char_leven_dist(get_token_dict(word_graph->dict, word),get_token_dict(word_graph->dict,curr_edge->output))*ecsf);
      else{
	if(ECAlg==1){ 
	   if(strcmp(get_token_dict(word_graph->dict, word),get_token_dict(word_graph->dict,curr_edge->output))==0) curr_edge_subs_prob = curr_edge->prob + curr_ec_state->prob[curr_state_id];
	   else curr_edge_subs_prob = curr_edge->prob + curr_ec_state->prob[curr_state_id] + ecsf;
	}
	else curr_edge_subs_prob= curr_edge->prob + curr_ec_state->prob[curr_state_id]; //ECAlg==2
      }
	   
      
      
      
      
      /* Did this state pass the EDIT BEAM ? */
 //     if((curr_edge_subs_cost - best_edit_cost) < EDIT_COST_BEAM){
        
        /* Let's see if the substitution cost of remaining in the same state is good enough */
//         if((next_ec_state->ec_cost[curr_edge->to] > curr_edge_subs_cost)||
//         ((next_ec_state->ec_cost[curr_edge->to] == curr_edge_subs_cost) &&
//         (next_ec_state->prob[curr_edge->to] < curr_edge_subs_prob))){
       
    
  if(((ECAlg==2) && ( (next_ec_state->ec_cost[curr_edge->to] > curr_edge_subs_cost) ||
                      ((next_ec_state->ec_cost[curr_edge->to] == curr_edge_subs_cost) && (next_ec_state->prob[curr_edge->to] < curr_edge_subs_prob)))) ||
    ((ECAlg==0 || ECAlg==1)&&(next_ec_state->prob[curr_edge->to] < curr_edge_subs_prob))){
 
  //    if(next_ec_state->prob[curr_edge->to] < curr_edge_subs_prob){
          /* Updating edit cost for this state */
	  
	  if(next_ec_state->ec_cost[curr_edge->to] > curr_edge_subs_cost) next_ec_state->ec_cost[curr_edge->to] = curr_edge_subs_cost;
          
          /* Updating prob for this state */
          next_ec_state->prob[curr_edge->to] = curr_edge_subs_prob;
          
          if(!next_ec_state->in_active_states[curr_edge->to]){

            /* Inserting state to be visited the next time */
            insert_into_ordered_list(next_ec_state->active_states, curr_edge->to, (void *) curr_edge->to);
            next_ec_state->in_active_states[curr_edge->to] = 1;
            
            /* Inserting states to be visited during deletion calculation */
            insert_into_ordered_list(states_to_be_visited_in_deletion_calculation, curr_edge->to, (void *) curr_edge->to);
            in_next_active_states[curr_edge->to] = 1;
            
          }
          
          /* Just in case we need to update the best_edit_cost */
  //        best_edit_cost = (curr_edge_subs_cost<best_edit_cost)?curr_edge_subs_cost:best_edit_cost;

        }

     // }/* End of if((curr_edge_subs_cost - best_edit_cost) < EDIT_COST_BEAM){ */

    }/* End of if (curr_edge->output != empty_symbol){ */    
    
  }/* End of while((curr_edge = read_from_list(&edge_list))!=NULL){ */  
  
}/* End of while((curr_state_id = (int) extract_from_ordered_list(curr_ec_state->active_states, word_graph->state_order))!=NULL){ */

next_ec_state = compute_deletions_transcripcion(next_ec_state, states_to_be_visited_in_deletion_calculation, in_next_active_states,ECAlg);

destroy_ec_state(&curr_ec_state);
return next_ec_state;

}


/***************************************************************************/

/** \d We want to know what states are reached with the last word in prefix, but we don't
want to update the parser state, since this word will be replaced by other */
ec_state_t *tentative_advance(int word,int ECAlg){

  ec_state_t *copy_ec_state;
  int* state_order;

  copy_ec_state = duplicate_ec_state(next_ec_state);
  if(word!=NO_WORD){
    if(tipo_CAT==TRANSCRIPCION)
        advance_parse_transcripcion(word,state_order,ECAlg);
    if(tipo_CAT==TRADUCCION)
        advance_parse_traduccion(word);
  }
  return copy_ec_state;

}


/** \d Return a list of states with the best minimum edit cost 
    and in case of equal edit cost, those within the same prob beam */
list_t *best_states_after_ec_parsing_transcripcion(float beam){

  list_t *active_states;
  list_t copy_active_states;
  list_t *best_states;
//  unsigned char best_edit_cost = MAX_EDIT_COST;
  float best_prob = LOG(0.0);
  int state_id;


  active_states = create_list_from_ordered_list(next_ec_state->active_states);

  COPY_LIST(active_states, (&copy_active_states));

  /* Let's see what states have better edit cost */
  while(!EMPTY_LIST((&copy_active_states))){
    
    state_id = (int) read_from_list(&copy_active_states);
    
//      if((next_ec_state->ec_cost[state_id] < best_edit_cost)||
//      ((next_ec_state->ec_cost[state_id] == best_edit_cost) &&
//      (next_ec_state->prob[state_id] > best_prob))){    
     if(next_ec_state->prob[state_id] > best_prob){
     //  best_edit_cost = next_ec_state->ec_cost[state_id];
       best_prob = next_ec_state->prob[state_id];
     }

  }

  best_states = create_list(1);
  
  /* Let's see what states have better edit cost */
  while(!EMPTY_LIST(active_states)){

    state_id = (int) extract_from_list(active_states);

    if (next_ec_state->prob[state_id] + beam  >= best_prob )
      insert_into_list(best_states,(void *) state_id);  

  }

  DESTROY_LIST(active_states);
  
  return best_states;
  
}




/** \d Return a list of states with the best minimum edit cost 
    and in case of equal edit cost, those within the same prob beam */
list_t *best_states_after_ec_parsing_traduccion(float beam){

  list_t *active_states;
  list_t copy_active_states;
  list_t *best_states;
  unsigned char best_edit_cost = MAX_EDIT_COST;
  float best_prob = LOG(0.0);
  int state_id;


  active_states = create_list_from_ordered_list(next_ec_state->active_states);

  COPY_LIST(active_states, (&copy_active_states));

  /* Let's see what states have better edit cost */
  while(!EMPTY_LIST((&copy_active_states))){
    
    state_id = (int) read_from_list(&copy_active_states);
    
     if((next_ec_state->ec_cost[state_id] < best_edit_cost)||
     ((next_ec_state->ec_cost[state_id] == best_edit_cost) &&
     (next_ec_state->prob[state_id] > best_prob))){    
       best_edit_cost = next_ec_state->ec_cost[state_id];
       best_prob = next_ec_state->prob[state_id];
     }

  }

  best_states = create_list(1);
  
  /* Let's see what states have better edit cost */
  while(!EMPTY_LIST(active_states)){

    state_id = (int) extract_from_list(active_states);

    if((next_ec_state->ec_cost[state_id] == best_edit_cost) && (next_ec_state->prob[state_id] + beam  >= best_prob ))
      insert_into_list(best_states,(void *) state_id);  

  }

  DESTROY_LIST(active_states);
  
  return best_states;
  
}

