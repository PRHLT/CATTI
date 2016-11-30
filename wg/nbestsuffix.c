/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library to compute n-best paths in a word graph */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "global.h"
#include "auxiliar.h"
#include "vector.h"
#include "dynamic_list.h"
#include "dynamic_ordered_list.h"
#include "hash.h"
#include "dictionary.h"
#include "heap.h"
#include "word_graph.h"
#include "nbestsuffix.h"

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define NO_STATE -1
#define NO_NEXT_PATH 0
#define YES_NEXT_PATH 1

/*****************************************************************************/
/*******************************VARIABLES*************************************/
/*****************************************************************************/

/** \d Word graph */
static wg_t *word_graph=NULL;

/** \d Default empty symbol whose deletion cost is ZERO */
static int empty_symbol=INT_MAX;
/** \d Default initial symbol*/
static int initial_symbol=INT_MAX;
/** \d Default final symbol*/
static int final_symbol=INT_MAX;

/** \d This data structure stores the information about the state of 
    the n-best algorithm for each state in the word graph*/
static nbest_state_t *nbest_state=NULL;

/** \d This data structure stores the information about the state of the search while 
    we are computing n-best paths from the list of initial states in the word graph */
static nbest_search_t *nbest_search=NULL;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/***************************************************************************/
/** \d Set word graph in n-best search */
void set_word_graph_nbestsuffix(wg_t *wg){
  word_graph=wg;
}

/***************************************************************************/
/** \d Set empty symbol id in n-best search
    empty symbol will be ignored when generating best path */
void set_empty_symbol_nbestsuffix(int es){
  empty_symbol=es;
}
/***************************************************************************/
/** \d Set initial symbol id in error-correcting parser.*/
void set_initial_symbol_nbestsuffix(int es){
  initial_symbol=es;
}

/***************************************************************************/
/** \d Set final symbol id in error-correcting parser.*/
void set_final_symbol_nbestsuffix(int es){
  final_symbol=es;
}
/***************************************************************************/
/** \d This function initializes the data structures for the n-best state and 
    search. It is supposed to be called only once for each source sentence */
void create_nbestsuffix(void){

  /**************************/
  /* Initialize nbest state */
  /**************************/
  mt(nbest_state=(nbest_state_t *)malloc(sizeof(nbest_state_t)));

  /* array of vectors containinig information about each nbest path in each state */
  nbest_state->list = create_vector(word_graph->size);

  /* array of heaps to store nbest path candidates */
  nbest_state->candidates = create_heap(word_graph->size,maxheap);

  /* Initialize nbest_state->shared not sharing any state since nbest list create its 1-best candidate
  independently from the candidates that we insert the first time in the heap */
  mt(nbest_state->shared = (unsigned char*) malloc(word_graph->size*sizeof(unsigned char)));
  memset(nbest_state->shared, 0, word_graph->size*sizeof(unsigned char));

  /***************************/
  /* Initialize nbest search */
  /***************************/
  mt(nbest_search=(nbest_search_t *)malloc(sizeof(nbest_search_t)));

  /* Array to indicate how many n-best paths have been already used in the n-best list */
  mt(nbest_search->num_paths = (int*) malloc(word_graph->size*sizeof(int)));
  
  /* Initialize lists of states to compute best paths from them */
  nbest_search->initial_states=create_list(1);

}

/***************************************************************************/
/** Setting states that will be the initial states to perform the n-best 
    suffix search and initialize num_paths computed for each state */
void set_initial_states_nbestsuffix(list_t *states_reached_after_ecparsing){


  /* As we starting with new search states, we haven't obtained any n-best translation */
  memset(nbest_search->num_paths, 0, word_graph->size*sizeof(int));

  /* States in the word graph from which the search of the suffix will start */
  COPY_LIST(states_reached_after_ecparsing, nbest_search->initial_states);

}

/***************************************************************************/
/** Initial backward pass to create 1-best paths */
void initialize_nbestsuffix_search(void){

  ordered_list_t *states_to_be_visited_backwards;
  unsigned char *states_to_be_visited_backwards_V;
  int state_id;
  edge_t *curr_edge;
  float curr_prob;
  float infinite = LOG(0.0);
  nbest_hyp_t *best;
  int i,e;

  states_to_be_visited_backwards = create_backward_ordered_list(word_graph->state_order, word_graph->size);
  mt(states_to_be_visited_backwards_V = (unsigned char *)malloc(word_graph->size*sizeof(unsigned char)));
  memset(states_to_be_visited_backwards_V, 0, word_graph->size*sizeof(unsigned char));
  
  for(i=0;i<word_graph->size;i++){
    mt(nbest_state->list[i].element = (void **) malloc(sizeof(void *)));
    nbest_state->list[i].size = 1;
    mt( best = (nbest_hyp_t *) malloc(sizeof(nbest_hyp_t)));
    best->prob = word_graph->wg[i].final;
    best->next_edge = NULL;
    best->best_path_in_next_state = 0;
    nbest_state->list[i].element[0] = best;
    /* If this state is final, let's store it as initial state to go backwards */
    if( word_graph->wg[i].final != infinite ){
      insert_into_ordered_list(states_to_be_visited_backwards, i, (void *) i);
      states_to_be_visited_backwards_V[i] = 1;
    }
  }

  /* Let's parse the word graph backwards from the final states 
     As a result we will have the Viterbi path from every state to a final state 
     in the n-best list that contains only one n-best hyp for each state */
  while(!empty_backward_ordered_list(states_to_be_visited_backwards)){
   
    state_id = (int) extract_from_backward_ordered_list(states_to_be_visited_backwards);

    for (e=0;e<word_graph->wg[state_id].from.l;e++){
      curr_edge = word_graph->wg[state_id].from.edv[e];
       
      curr_prob = curr_edge->prob + ((nbest_hyp_t *) nbest_state->list[state_id].element[0])->prob;
      /* Checking if new prob of reaching this state from curr_edge->from is better than any before */
      if( curr_prob > ((nbest_hyp_t *) nbest_state->list[curr_edge->from].element[0])->prob ){
        ((nbest_hyp_t *) nbest_state->list[curr_edge->from].element[0])->prob = curr_prob;
        ((nbest_hyp_t *) nbest_state->list[curr_edge->from].element[0])->next_edge = curr_edge;
        ((nbest_hyp_t *) nbest_state->list[curr_edge->from].element[0])->best_path_in_next_state = 0;
        if(!states_to_be_visited_backwards_V[curr_edge->from]){
          insert_into_ordered_list(states_to_be_visited_backwards, curr_edge->from, (void *) curr_edge->from);
          states_to_be_visited_backwards_V[curr_edge->from] = 1;
        }
      }
    }
  }/* End of while(!empty_ordered_list(states_to_be_visited_backwards, word_graph->state_order){ */
  
  destroy_ordered_list(states_to_be_visited_backwards);
  free(states_to_be_visited_backwards_V);
  
}/* End of void initialize_nbestsuffix_search(void){ */

/***************************************************************************/
/** Populate n-best candidate heap with hyps from outgoing edges and final state */
void initialize_nbest_candidate_heap(int state_id){

  nbest_hyp_t *candidate;
  edge_t *curr_edge;
  float infinite = LOG(0.0);
  int e;

  /* If this state has final probability, let's create a fake candidate
  to simulate a final candidate. Mind log probability */
  if(word_graph->wg[state_id].final != infinite){
    mt(candidate = (nbest_hyp_t *) malloc(sizeof(nbest_hyp_t)));
    candidate->prob = word_graph->wg[state_id].final;
    candidate->next_edge = NULL;
    candidate->best_path_in_next_state = 0;
    push_heap(&(nbest_state->candidates[state_id]), candidate->prob, candidate);
  }

  /* For each edge create a candidate to be inserted in the current state heap. 
     Taking the best path in the next state */
  for (e=0;e<word_graph->wg[state_id].to.l;e++){
    curr_edge = word_graph->wg[state_id].to.edv[e];
    mt(candidate = (nbest_hyp_t *) malloc(sizeof(nbest_hyp_t)));
    candidate->prob = curr_edge->prob + ((nbest_hyp_t *) nbest_state->list[curr_edge->to].element[0])->prob;
    candidate->next_edge = curr_edge;
    candidate->best_path_in_next_state = 0;
    push_heap(&(nbest_state->candidates[state_id]), candidate->prob, candidate);
  }
  /* Make sure that best hyp in nbest list is the max element in the heap 
     since this hyp is shared between these two data structures */
  free(nbest_state->list[state_id].element[0]);
  nbest_state->list[state_id].element[0]=top_heap(&(nbest_state->candidates[state_id]));
  nbest_state->shared[state_id] = 1;

}

/***************************************************************************/
/** Function that said if the word "word" is in the vector "v" */

int belong(vector_t *v, int word){
    int i; 
  
    for(i=0;i < v->size; i++){
      if((int) v->element[i]==word) return 1;
    } 
    
    return 0;
}

/***************************************************************************/
/** Function that said if the string "int_word" starts with the word "subword" */

int starts(char* last_word_in_prefix,int int_word){
  
   dict_t *wg_dict;
   char *word; 
    
   wg_dict = word_graph->dict;
   
   word=get_token_dict(wg_dict,int_word); 
   
   if(strlen(last_word_in_prefix)==0) return 1; 
   if(strncmp(word,last_word_in_prefix,strlen(last_word_in_prefix))==0) return 1;
   
   return 0;
}


/***************************************************************************/
/** REA function that calculates the n-best paths in a word graph */
int next_path(int state_id,vector_t *id_fwils,char *last_word_in_prefix){

  nbest_hyp_t *candidate;
  nbest_hyp_t *new_candidate;
  vector_t *v;

  /* We just have the first candidate from the backward search */
  /* Let's start adding candidates to this state, as many as outgoing edges */
  if(nbest_state->list[state_id].size == 1)
    initialize_nbest_candidate_heap (state_id);

  /* If we still have candidates in the current state, let's extract one from
  the top of the heap and start creating the nbest */
  if(!isempty_heap((&(nbest_state->candidates[state_id])))){
  
    /* Obtain the candidate from the top of the current state heap */
    candidate = (nbest_hyp_t *) top_heap(&(nbest_state->candidates[state_id]));

    /* If there is not next edge, that is, we are in a final state, then remove element from heap 
       Final states are treated in a special way since we have a special nbest hyp to 
       indicate we are at the end of a path */
    if(candidate->next_edge == NULL){
      /* Mind that we are extracting the previous best hyp, the one that is being used to return 
         the current best translation, this hyp was shared with the nbest list */
      pop_heap(&(nbest_state->candidates[state_id]));
      /* Consequently this nbest hyp is not shared by the heap and the list */
      nbest_state->shared[state_id] = 0;
    }  
    /* If this candidate is not final, then let's extract it from the heap
       and start creating the next best path, otherwise just leave it where it was */
    else{
      if (!belong(id_fwils,candidate->next_edge->output) && starts(last_word_in_prefix,candidate->next_edge->output)){//candidate->next_edge->output != id_fwils) {
	/* Let's see if the next best path is already calculated */
	/* If not, we will need to calculate it recursively */
        if(nbest_state->list[candidate->next_edge->to].size <= candidate->best_path_in_next_state + 1){
          /* if the current output is not the empty word, then it is truly != id_fwils.
	     In that case, we do not want to propagate id_fwils (first_word_in_last_suffix
	     may appear in the rest of the path */
          if (candidate->next_edge->output != empty_symbol){

            v = create_vector(1);
	    next_path(candidate->next_edge->to,v,last_word_in_prefix); 
            destroy_vector(v,1,0);
          }
 	  /* if it was the empty word, then propagate id_fwils until we find a word */
	  else
 	    next_path(candidate->next_edge->to, id_fwils,last_word_in_prefix);
	}
      };
	

	    /* Mind that we are extracting the previous best hyp, the one that is being used to return 
	       the current best translation, this hyp was shared with the nbest list */
       pop_heap(&(nbest_state->candidates[state_id]));

	    /* Consequently this nbest hyp is not shared by the heap and the nbest list */
       nbest_state->shared[state_id] = 0;

      /* If this next nbest path is calculated, then insert it in the heap as new candidate */
      if(nbest_state->list[candidate->next_edge->to].size > candidate->best_path_in_next_state + 1){
        /* Construct next best hyp going thru the same edge that we used in the previous 
           best hyp, that is, candidate. We know that the new best hyp in the next state 
           is the following to that of candidate in the nbest list, that is, 
           candidate->best_path_in_next_state + 1 since we just computed in next_path and 
           we checked that there was a new best hyp in the next state in the if condition above */
	mt(new_candidate = (nbest_hyp_t *) malloc(sizeof(nbest_hyp_t)));
        new_candidate->prob = candidate->next_edge->prob + ((nbest_hyp_t *) nbest_state->list[candidate->next_edge->to].element[candidate->best_path_in_next_state + 1])->prob;
        new_candidate->next_edge = candidate->next_edge;
        new_candidate->best_path_in_next_state = candidate->best_path_in_next_state + 1;

        push_heap(&(nbest_state->candidates[state_id]), new_candidate->prob, new_candidate);

      }

    }/* End of if(candidate->next_edge != NULL){ */

    /* Finally move best candidate in heap to nbest list, we are sharing a nbest hyp */
    if(!isempty_heap((&(nbest_state->candidates[state_id])))){
      mt(nbest_state->list[state_id].element = (void **) realloc(nbest_state->list[state_id].element, (nbest_state->list[state_id].size + 1)*sizeof(void *)));
      nbest_state->list[state_id].size++;
      nbest_state->list[state_id].element[nbest_state->list[state_id].size - 1] = top_heap(&(nbest_state->candidates[state_id]));
      nbest_state->shared[state_id] = 1;
      return YES_NEXT_PATH;
    }
    else{
      return NO_NEXT_PATH;
    }
    
  }/* End of if(!isempty_heap(nbest_state->candidates[state_id])){ */
  else{
    return NO_NEXT_PATH; 
  }
  
}/* End of void next_path(int state_id){ */



/***************************************************************************/

float *get_prob_nbestsuffix(void){
   list_t state_list;
   float *probs;
   nbest_hyp_t *act_hyp;
   int state_id;
   
   COPY_LIST(nbest_search->initial_states, (&state_list));
   
   mt(probs = (float *)malloc(word_graph->size*sizeof(float)));
   
   while(!EMPTY_LIST((&state_list))){
      state_id = (int) read_from_list(&state_list);
      act_hyp = (nbest_hyp_t *) nbest_state->list[state_id].element[nbest_search->num_paths[state_id]];
      probs[state_id]=act_hyp->prob;
   }

  return probs;
  
}



/***************************************************************************/
/** Return the next best suffix */
float get_next_nbestsuffix(vector_t *suffix,float *prev_probs,vector_t *id_fwils, int *state_words, int multimodality, int *state,char *last_word_in_prefix, vector_t *alignment){
  
    float best_prob = LOG(0.0);
    float infinite = LOG(0.0);
    int best_state = NO_STATE;
    list_t state_list;
    int state_id;
    edge_t *curr_edge;
    int curr_best_index;
    int next_best_index;
    nbest_hyp_t *tmp_hyp, *act_hyp;
    vector_t *v;
               
    COPY_LIST(nbest_search->initial_states, (&state_list));

    /**************************************************************************/
    /*** FIRST FIND THE BEST STATE (BEST PROB) IN THE LIST OF INITIAL STATES  */
    /**************************************************************************/

    /* Let's see which state has the best probability to reach a final state */
    while(!EMPTY_LIST((&state_list))){

      state_id = (int) read_from_list(&state_list);

      /* The first condition is neccessary to check that we really have that next best path 
         among the list of states from which we can compute the best paths. It might happen 
         that the global 1-best path we take is the best path from state i and then the 
         global 2-best path is the best path from state j */

      act_hyp = (nbest_hyp_t *) nbest_state->list[state_id].element[nbest_search->num_paths[state_id]];

      while(best_prob < act_hyp->prob+prev_probs[state_id]) {
        tmp_hyp=act_hyp;
        /* search the output until we find a word */
        while (tmp_hyp->next_edge != NULL && tmp_hyp->next_edge->output == empty_symbol)
	  tmp_hyp = (nbest_hyp_t *) nbest_state->list[tmp_hyp->next_edge->to].element[tmp_hyp->best_path_in_next_state];
        /* if we did not find a word or the next word is NOT first_word_in_last_suffix, consider this state */
	if (tmp_hyp->next_edge == NULL || (!belong(id_fwils,tmp_hyp->next_edge->output) && (starts(last_word_in_prefix,tmp_hyp->next_edge->output)))){//tmp_hyp->next_edge->output != id_fwils) {
	   best_prob = ((nbest_hyp_t *) nbest_state->list[state_id].element[nbest_search->num_paths[state_id]])->prob+prev_probs[state_id];
           best_state = state_id;
	   break; /* we like this path! save it! */
	}
        else {
	  if(next_path(state_id, id_fwils, last_word_in_prefix) == YES_NEXT_PATH){
	    nbest_search->num_paths[state_id]++;
	    act_hyp = (nbest_hyp_t *) nbest_state->list[state_id].element[nbest_search->num_paths[state_id]];
	  }
	  else break;  /* we did not find any valid paths for this node */
	}
      }
   }


    /***********************************************************************************************************/
    /*** EXTRACT CURRENT BEST PATH FROM best_state AND COMPUTE A NEXT BEST PATH FROM best_state FOR NEXT TIME  */
    /***********************************************************************************************************/
   

   /* If we have a state that reaches a final state, let's extract it */
   if(best_state != NO_STATE){
     // printf("\n The best state is %d\n", best_state);
     if(multimodality){
       mt(suffix->element = (void **) realloc(suffix->element, (suffix->size+1)*sizeof(void *)));
       suffix->size++;
       suffix->element[suffix->size-1] = (void *) state_words[best_state];
       /*HAY QUE AÑADIR EL ALINEAMIENTO CUANDO SE HACE LA MULTIMODALIDAD*/
     }
     
     /* Edge departing from best state and best path in next state, mind the importance of nbest_search->num_paths 
      to select the n-best path that haven't been used yet */
     curr_edge = ((nbest_hyp_t *) nbest_state->list[best_state].element[nbest_search->num_paths[best_state]])->next_edge;
     curr_best_index = ((nbest_hyp_t *) nbest_state->list[best_state].element[nbest_search->num_paths[best_state]])->best_path_in_next_state;

    
     /* Checking first that this state is already not a final state */
     while(curr_edge != NULL){
       /* Ignore empty_symbol, initial_symbol and final_symbol transitions to construct the suffix */
       if((curr_edge->output != empty_symbol)&&(curr_edge->output != initial_symbol)){
         mt(suffix->element = (void **) realloc(suffix->element, (suffix->size+1)*sizeof(void *)));
         suffix->size++;
         suffix->element[suffix->size-1] = (void *) curr_edge->output;
	 
	 if(curr_edge->output != final_symbol){
	    mt(alignment->element = (alignment_t **) realloc(alignment->element, (alignment->size+1)*sizeof(alignment_t *) ) );
	    alignment->size++;
	    mt(alignment->element[alignment->size-1] = (alignment_t *) malloc(sizeof(alignment_t)));
	    ((alignment_t *) alignment->element[alignment->size-1])->start  =word_graph->wg[curr_edge->from].time ;
	    ((alignment_t *) alignment->element[alignment->size-1])->end =word_graph->wg[curr_edge->to].time;
	 }
       }
       /* Let's move to the next state in the current best path until we reach a final state, that is, curr_edge == NULL */
       next_best_index = ((nbest_hyp_t *) nbest_state->list[curr_edge->to].element[curr_best_index])->best_path_in_next_state;       
       curr_edge = ((nbest_hyp_t *) nbest_state->list[curr_edge->to].element[curr_best_index])->next_edge;
       curr_best_index = next_best_index;
     }

     /* We have used the current best path in best_state, let's calculate the next one for this state */
     /* If there exists, then increase the number of best paths calculated */     
     v = create_vector(1);
     if(next_path(best_state, v,last_word_in_prefix) == YES_NEXT_PATH){
       nbest_search->num_paths[best_state]++;
     }  
     /* Otherwise remove this best state from the list of states from which we will start
     the search of suffices for the current search */
     else{
       remove_from_list(nbest_search->initial_states, (void *) best_state);
     }
     destroy_vector(v,1,0);  
     
   }/* End of if(best_state != NO_STATE){ */
   /* We haven't found any state from which extract n-best paths, state_list was empty! 
      best_prob == infinite means that no best path was found */
   else{
     best_prob = infinite;
   }

   *state=best_state;
    
   return best_prob;
   
} /* End of float get_next_nbestsuffix(vector_t *suffix){ */

/***************************************************************************/
/** Destroy n-best state and search */
void destroy_nbestsuffix(void){

  if(nbest_state){
    /* array of vectors containinig information about each nbest path in each state */
    destroy_vector(nbest_state->list, word_graph->size, 1);

    /* array of heaps to store nbest path candidates */
    cond_destroy_heap(nbest_state->candidates, word_graph->size, nbest_state->shared);

    if(nbest_state->shared) free(nbest_state->shared);

    free(nbest_state);
    nbest_state=NULL;
  }

  
  if(nbest_search){
    /* array to indicate how many nbest have been already stored in nbest */
    if(nbest_search->num_paths) free(nbest_search->num_paths);
    /* Free state_list */ 
    DESTROY_LIST(nbest_search->initial_states);

    free(nbest_search);
    nbest_search=NULL;
  }
}

/***************************************************************************/
/** Destroy initial_states list by emptying it  */
void destroy_initial_states_nbestsuffix(void){

  while(!EMPTY_LIST(nbest_search->initial_states))
    extract_from_list(nbest_search->initial_states);

}
