/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file
\brief \d Library to perform a set of operations on a word graph */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>

#include "global.h"
#include "auxiliar.h"
#include "dynamic_list.h"
#include "dynamic_ordered_list.h"
#include "hash.h"
#include "dictionary.h"
#include "word_graph.h"
#include "vector.h"
#include "strop.h"

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define DICT_HASH_TABLE_SIZE 65536
#define FALSE 0
#define TRUE 1
/*****************************************************************************/
/*****************************VARIABLES***************************************/
/*****************************************************************************/

/** \d Word graph */
//static wg_t *word_graph=NULL;
//static wg_t *word_graph_online=NULL;
//static wg_t *unigram=NULL;

/** \d Default empty symbol whose deletion cost is ZERO */
static int empty_symbol=INT_MAX;
/** \d Default initial symbol*/
static int initial_symbol=INT_MAX;
/** \d Default final symbol*/
static int final_symbol=INT_MAX;
/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/*****************************************************************************/
/** \d Set word graph and number of states */
//void set_word_graph(wg_t *wg){
//   wg_t *word_graph=NULL;
//  word_graph=wg;
//}

/*****************************************************************************/
/** \d Set word graph and number of states */
//void set_unigram(wg_t *wg){
//  unigram=wg;
//}

/***************************************************************************/
/** \d Set empty symbol id in error-correcting parser. 
    Deletion  cost of empty symbol is ZERO */
void set_empty_symbol_word_graph(int es){
  empty_symbol=es;
}

/***************************************************************************/
/** \d Set initial symbol id in error-correcting parser.*/
void set_initial_symbol_word_graph(int es){
  initial_symbol=es;
}

/***************************************************************************/
/** \d Set final symbol id in error-correcting parser.*/
void set_final_symbol_word_graph(int es){
  final_symbol=es;
}

/***************************************************************************/
/** \d Set the GSF value  */
void set_GSF_word_graph(float gsf,wg_t *word_graph){
  word_graph->GSF=gsf;
}

/***************************************************************************/
/** \d Set the WIP value  */
void set_WIP_word_graph(float wip,wg_t *word_graph){
  word_graph->WIP=wip;
}

/*****************************************************************************/
/** \d Returns the word graph data structure */
//wg_t *get_word_graph(void){ 

//  return word_graph;
//}

/*****************************************************************************/
/** \d Returns the dictionary data structure */
//dict_t *get_word_graph_dict(wg_t *word_graph){
//  return word_graph->dict;
//}

/*****************************************************************************/
/** \d Returns the dictionary data structure */
//dict_t *get_unigram_dict(void){
//  return unigram->dict;
//}

/*****************************************************************************/
/** \d Returns the final symbol data structure */
int get_word_graph_initial_symbol(void){
  return initial_symbol;
}

/*****************************************************************************/
/** \d Returns the final symbol data structure */
int get_word_graph_final_symbol(void){
  return final_symbol;
}

/*****************************************************************************/
/** This function returns a vector with the topological order of each state in 
    a word graph. This algorithm was extracted from "Efficient Error-Correcting 
    Parsing"  by J.C. Amengual and E. Vidal */
void topo_order_word_graph(wg_t *word_graph){
    
int *ro;
int *rod;
int i,e,max=-1;
edge_t *curr_edge;
list_t *states_to_be_visited;
int state_id;
int *state_order;
int find=FALSE;

mt(state_order = (int *) malloc(word_graph->size * sizeof(int)));
memset(state_order, 0, word_graph->size * sizeof(int));

mt(ro = (int *) malloc(word_graph->size * sizeof(int)));
memset(ro, 0, word_graph->size * sizeof(int));

mt(rod = (int *) malloc(word_graph->size * sizeof(int)));
memset(rod, 0, word_graph->size * sizeof(int));

/* Calculate the number of incident edges for each state (ro) and the number of differente incident edges (rod) */
for(i=0;i<word_graph->size;i++){
  ro[i] = word_graph->wg[i].from.l; 
  for (e=0;e<word_graph->wg[i].from.l;e++){
     if(word_graph->wg[i].from.edv[e]->from!=i) rod[i]++ ;
  }
}



i=0;
while(find==FALSE) {
  if(rod[i]==0) find=TRUE;
  else i++;
}

istate=i;

/* Let's insert the states in the word graph in a heap based on the
number of incident edges */
states_to_be_visited = create_list(1);

/* Initial state is the first one ordered obviously */
insert_into_list(states_to_be_visited, (void *) istate);

/* while we have state in the list to explore */
while(!EMPTY_LIST(states_to_be_visited)){

  state_id = (int ) extract_from_list(states_to_be_visited);

  /* Let's visit all edges of this state and decrease the number of incident edges of curr_edge->to */
/*  for (e=0;e<word_graph->wg[state_id].to.l;e++){
    curr_edge = word_graph->wg[state_id].to.edv[e];
    state_order[curr_edge->to] = MAX(state_order[curr_edge->to],state_order[state_id]+1);
    ro[curr_edge->to]--;
    if(ro[curr_edge->to] == 0)
      insert_into_list(states_to_be_visited, (void *) curr_edge->to);
  }*/

   for (e=0;e<word_graph->wg[state_id].to.l;e++){
    curr_edge = word_graph->wg[state_id].to.edv[e];
    if(curr_edge->to!=curr_edge->from){  
      state_order[curr_edge->to] = MAX(state_order[curr_edge->to],state_order[state_id]+1);
      rod[curr_edge->to]--;
      if(rod[curr_edge->to] == 0)
	insert_into_list(states_to_be_visited, (void *) curr_edge->to);
    }
   }

}

DESTROY_LIST(states_to_be_visited);
free(ro);
free(rod);

word_graph->state_order=state_order;

/* Ponemos la log-probabilidad final del estado final a 0*/
max=0; 
for(i=1;i<word_graph->size;i++) {
    if(state_order[max] < state_order[i]) max=i;
}
word_graph->wg[max].final=0;

}


/*****************************************************************************/
/** This function returns a vector with the topological order of each state in 
    a online word graph. This algorithm was extracted from "Efficient Error-Correcting 
    Parsing"  by J.C. Amengual and E. Vidal */
//void topo_order_online_word_graph(void){
    
//int *ro;
//int i,e,max=-1;
//edge_t *curr_edge;
//list_t *states_to_be_visited;
//int state_id;
//int *state_order;

//mt(state_order = (int *) malloc(word_graph_online->size * sizeof(int)));
//memset(state_order, 0, word_graph_online->size * sizeof(int));

//mt(ro = (int *) malloc(word_graph_online->size * sizeof(int)));
//memset(ro, 0, word_graph_online->size * sizeof(int));

/* Calculate the number of incident edges for each state */
//for(i=0;i<word_graph_online->size;i++)
//  ro[i] = word_graph_online->wg[i].from.l;

/* Let's insert the states in the word graph in a heap based on the
number of incident edges */
//states_to_be_visited = create_list(1);

/* Initial state is the first one ordered obviously */
//insert_into_list(states_to_be_visited, (void *) istate);

/* while we have state in the list to explore */
//while(!EMPTY_LIST(states_to_be_visited)){

//  state_id = (int ) extract_from_list(states_to_be_visited);

  /* Let's visit all edges of this state and decrease the number of incident edges of curr_edge->to */
//  for (e=0;e<word_graph_online->wg[state_id].to.l;e++){
//    curr_edge = word_graph_online->wg[state_id].to.edv[e];
//    state_order[curr_edge->to] = MAX(state_order[curr_edge->to],state_order[state_id]+1);
//    ro[curr_edge->to]--;
    /* It's time to visited curr_edge->to*/
//    if(ro[curr_edge->to] == 0)
//      insert_into_list(states_to_be_visited, (void *) curr_edge->to);
//  }
//}

//DESTROY_LIST(states_to_be_visited);
//free(ro);

//word_graph_online->state_order=state_order;

/* Ponemos la log-probabilidad final del estado final a 0*/
//max=0; 
//for(i=1;i<word_graph_online->size;i++) {
//    if(state_order[max] < state_order[i]) max=i;
//}
//word_graph_online->wg[max].final=0;

//}

/*****************************************************************************/
/** \d Compute the log probability for each edge*/

void compute_log_probability(wg_t *word_graph){

    int i,e;
    edge_t *curr_edge;

    for(i=0;i<word_graph->size;i++)
        for (e=0;e<word_graph->wg[i].to.l;e++){
           curr_edge = word_graph->wg[i].to.edv[e];
	   if(word_graph->wg[i].to.edv[e]->prob==log(0.0))
              word_graph->wg[i].to.edv[e]->prob=curr_edge->hmm_prob+(word_graph->GSF* curr_edge->lm_prob)+word_graph->WIP;
	  
	}

}


/*****************************************************************************/
/** \d Extract vocabulary from word graph and sort it for binary search */
void extract_and_sort_dict_word_graph(dict_t *outdict,wg_t *word_graph){

   int i,e;
   edge_t *curr_edge;

   /* Create dictionary for word graph vocabulary */
   word_graph->dict = create_dict(DICT_HASH_TABLE_SIZE);

   for(i=istate;i<word_graph->size;i++){

    /* We are going to explore all the edges for the current state */
    for (e=0;e<word_graph->wg[i].to.l;e++){
      curr_edge = word_graph->wg[i].to.edv[e];

      if(curr_edge->output!=empty_symbol){
         /* Get token from outdict dictionary and insert it into word graph dictionary */
         set_token_dict(word_graph->dict,get_token_dict(outdict,curr_edge->output));
      }
    } /* close for (e=0;e<word_graph->wg[i].to.l;e++){ */
  }/* close for(i=INITIAL_STATE;i<word_graph->size;i++){ */

/* Sort dictionary for binary search */
sort_dict(word_graph->dict);

}

/*****************************************************************************/
/** \d Perform binary search in word graph dictionary */
int search_in_dict_word_graph(dict_t *outdict, char *word, wg_t* word_graph,vector_t *first_word_last_suffix){

int word_id;


  word_id = search_in_dict_with_restrictions(word_graph->dict,word,first_word_last_suffix);


if (word_id==NO_WORD)
  return NO_WORD;
else
  /* Convert from word graph id to outdict id */
  return get_id_dict(outdict,get_token_dict(word_graph->dict,word_id));
}


/***************************************************************************/
/** Function that said if the word "word" is in the vector "v" */

int  contain(vector_t *v, int word){
    int i; 
  
    for(i=0;i < v->size; i++){
      if((int) v->element[i]==word) return 1;
    } 
    
    return 0;
}


/*****************************************************************************/
/** \d This function returns (if any) a word found on the outgoing edges of 
    the states in the state_list that match (as prefix) the "word" */
int search_in_state_edges_word_graph(list_t *state_list, char *word, int word_length, dict_t *outdict, wg_t *word_graph,vector_t *first_word_last_suffix,float *probs,float *probs_suffix){

  list_t aux_list;
  int state_id,e;
  float best_prob=LOG(0.0);
  int matching_word = NO_WORD;
  edge_t *curr_edge;

     COPY_LIST(state_list, (&aux_list));

     while(!EMPTY_LIST((&aux_list))){
       /* Obtain state from list of active states, those ones reached after ec parsing */
       state_id = (int ) read_from_list((&aux_list));

       for (e=0;e<word_graph->wg[state_id].to.l;e++){
         curr_edge = word_graph->wg[state_id].to.edv[e];
         /* Let's see if the "word" is a prefix of the output of this edge */
	 
         if((curr_edge->output != empty_symbol) && (curr_edge->output != initial_symbol) && (curr_edge->output != final_symbol) && (!contain(first_word_last_suffix,curr_edge->output)) && ((curr_edge->prob+probs[curr_edge->from]+probs_suffix[curr_edge->to]) > best_prob) &&
         (!strncmp(get_token_dict(outdict,curr_edge->output),word,word_length))){
            matching_word = curr_edge->output;
            best_prob = curr_edge->prob+probs[curr_edge->from]+probs_suffix[curr_edge->to];
         }
       }
     }
  return matching_word;
}

/*****************************************************************************/
/** \d Function that free all data structures associated with the word graph */
void destroy_word_graph(wg_t **word_graph){

  int k,e;
  edge_t *edge;
  
  /* Just checking that we haven't already created this word graph in the previous call */
  if(*word_graph!=NULL){
    for(k=0;k<(*word_graph)->size;k++){
      /* Extracting and deleteing edges */
      for (e=0;e<(*word_graph)->wg[k].to.l;e++){
        edge = (*word_graph)->wg[k].to.edv[e];
        free(edge);
      }  
      free((*word_graph)->wg[k].to.edv);
      free((*word_graph)->wg[k].from.edv);
    }
    free((*word_graph)->wg);

    if((*word_graph)->state_order) free((*word_graph)->state_order);
    (*word_graph)->state_order = NULL;

    /* Destroy word graph dictionary */
    destroy_dict((*word_graph)->dict);

    free((*word_graph)); 
    (*word_graph) = NULL;
  }

}

/*****************************************************************************/
/** \d Function that free all data structures associated with the online word graph */
//void destroy_online_word_graph(void){

//  int k,e;
//  edge_t *edge;
  
  /* Just checking that we haven't already created this word graph in the previous call */
//  if(word_graph_online!=NULL){
//    for(k=0;k<word_graph_online->size;k++){
      /* Extracting and deleteing edges */
//      for (e=0;e<word_graph_online->wg[k].to.l;e++){
//        edge = word_graph_online->wg[k].to.edv[e];
//        free(edge);
//      }
//      free(word_graph_online->wg[k].to.edv);
//      free(word_graph_online->wg[k].from.edv);
//    }
//    free(word_graph_online->wg);

//    if(word_graph_online->state_order) free(word_graph_online->state_order);
//    word_graph_online->state_order = NULL;

    /* Destroy word graph dictionary */
//    destroy_dict(word_graph_online->dict);

//    free(word_graph_online); 
//    word_graph_online = NULL;
//  }

//}

/*****************************************************************************/
/** \d Function that free all data structures associated with the unigram word graph */
//void destroy_unigram(void){

//  int k,e;
//  edge_t *edge;
  
  /* Just checking that we haven't already created this word graph in the previous call */
//  if(unigram!=NULL){
//    for(k=0;k<unigram->size;k++){
      /* Extracting and deleteing edges */
//      for (e=0;e<unigram->wg[k].to.l;e++){
//        edge = unigram->wg[k].to.edv[e];
//        free(edge);
//      }
//      free(unigram->wg[k].to.edv);
//      free(unigram->wg[k].from.edv);
//    }
//    free(unigram->wg);

//    if(unigram->state_order) free(unigram->state_order);
//    unigram->state_order = NULL;

    /* Destroy word graph dictionary */
//    destroy_dict(unigram->dict);

//    free(unigram); 
//    unigram = NULL;
//  }

//}

/*****************************************************************************/
/** \d This function generates a file containing the word graph in v2 format */
void word_graph2v2_format_file_simple(dict_t *outdict,wg_t *word_graph){

int i,e;
int number_of_edges=0;
edge_t *curr_edge;
FILE *fp;

ft(fp = fopen("word_graph.v2","w"),"word_graph.v2");

for(i=istate;i<word_graph->size;i++){

  /* We are going to explore all the edges for the current state */
  number_of_edges+=word_graph->wg[i].to.l;

}/* close for(i=INITIAL_STATE;i<word_graph->size;i++){ */

/* Print v2 format header */
fprintf(fp,"Name .\n");
fprintf(fp,"NumStates %d\n",word_graph->size);
fprintf(fp,"NumEdges %d\n\n",number_of_edges);

for(i=istate;i<word_graph->size;i++){

  /* Print the current state and take into account if this state is final */
  fprintf(fp,"State %d f=%f\n",i,exp(word_graph->wg[i].final));

  /* We are going to explore all the edges for the current state */
  for (e=0;e<word_graph->wg[i].to.l;e++){
    curr_edge = word_graph->wg[i].to.edv[e];

    /* This edge is going to a valid state then print out the information relate with this edge */
  fprintf(fp,"\t%d\t%d\t\"%s\"\tp =\"%f\"\n", i,curr_edge->to, get_token_dict(outdict,curr_edge->output),exp(curr_edge->prob));
//  fprintf(fp,"\t%d\t%d\t\"%s\"\tp =\"%f\"\n", i,curr_edge->to, get_token_dict(outdict,curr_edge->output),curr_edge->prob);
    

  } /* close for (e=0;e<word_graph->wg[i].to.l;e++){ */

}/* close for(i=INITIAL_STATE;i<word_graph->size;i++){ */

fflush(fp);
/* Close file */
fclose(fp);

}


/*****************************************************************************/
/** \d This function compute the forward probability for each state */

void computeForward(double *forward,wg_t *wg){
  
  
  ordered_list_t *states_to_be_visited_forwards;
  int state_id;
  edge_t *curr_edge;
  float curr_prob;
  //float infinite = LOG(0.0);
  int i,e;
  float cycle_prob=0.0;
  
  states_to_be_visited_forwards = create_forward_ordered_list(wg->state_order, wg->size);
  
   
  for(i=0;i<wg->size;i++){
    insert_into_ordered_list(states_to_be_visited_forwards, i, (void *) i);
    /* If this state is the initial state, let's put its forwards probability to 1.*/
    if( wg->state_order[i]== 0){
      forward[i]=0;
    }
  }

  /* Let's parse the word graph forward from the final states */
  while(!empty_forward_ordered_list(states_to_be_visited_forwards)){
   
    state_id = (int) extract_from_forward_ordered_list(states_to_be_visited_forwards);
    
    for (e=0;e<wg->wg[state_id].from.l;e++){
      curr_edge = wg->wg[state_id].from.edv[e];
      if(curr_edge->from==curr_edge->to){ /*Se calcula la probabilidad de los ciclos*/
	if(cycle_prob==0.0) cycle_prob=curr_edge->prob;
	else cycle_prob=add_log(curr_edge->prob,cycle_prob);
      }
      else{ 
	curr_prob = curr_edge->prob + forward[curr_edge->from];
	forward[state_id]=add_log(forward[state_id],curr_prob);     
      }
           
    }
    if(cycle_prob !=0.0)
      forward[state_id]=forward[state_id]-log(1-exp(cycle_prob));
    cycle_prob=0.0;
    
  }
    
  
  destroy_ordered_list(states_to_be_visited_forwards);
   
}


/*****************************************************************************/
/** \d This function compute the backward probability for each state */

void computeBackward(double *backward,wg_t *wg){
  
  
  ordered_list_t *states_to_be_visited_backwards;
  int state_id;
  edge_t *curr_edge;
  float curr_prob;
  float infinite = LOG(0.0);
  int i,e;
  double cycle_prob=0.0;
  
  states_to_be_visited_backwards = create_backward_ordered_list(wg->state_order, wg->size);
  
   
  for(i=0;i<wg->size;i++){
    insert_into_ordered_list(states_to_be_visited_backwards, i, (void *) i);
    /* If this state is final, let's put its backward probability to 1.*/
    if( wg->wg[i].final != infinite ){
      backward[i]=0.0;
    }
  }

  /* Let's parse the word graph backwards from the final states */
  while(!empty_backward_ordered_list(states_to_be_visited_backwards)){
   
    state_id = (int) extract_from_backward_ordered_list(states_to_be_visited_backwards);
    
    for (e=0;e<wg->wg[state_id].to.l;e++){
      curr_edge = wg->wg[state_id].to.edv[e];
      if(curr_edge->from==curr_edge->to){ /*Se calcula la probabilidad de los ciclos*/
 	if(cycle_prob!=0.0)
 	    cycle_prob=add_log(curr_edge->prob,cycle_prob);
 	else
 	    cycle_prob=curr_edge->prob;
      }
     else{ 
      curr_prob = curr_edge->prob + backward[curr_edge->to]; 
      backward[state_id]=add_log(backward[state_id],curr_prob);     
      }
    }
    if(cycle_prob !=0.0){ 
      double val1=exp(cycle_prob);
      double val2=1-val1;
      double val3=log(val2);
      backward[state_id]=-val3+backward[state_id];
    }
    cycle_prob=0.0;
    
  }
    
  
  destroy_ordered_list(states_to_be_visited_backwards);
   
}
/*****************************************************************************/
/** \d This function compute the backward probability for each state */

/*void computeBackward(double *backward,wg_t *wg){
  
  
  ordered_list_t *states_to_be_visited_backwards;
  int state_id;
  edge_t *curr_edge;
  float curr_prob;
  float infinite = LOG(0.0);
  int i,e;
  
  states_to_be_visited_backwards = create_backward_ordered_list(wg->state_order, wg->size);
  
   
  for(i=0;i<wg->size;i++){
    insert_into_ordered_list(states_to_be_visited_backwards, i, (void *) i);*/
    /* If this state is final, let's put its backward probability to 1.*/
/*    if( wg->wg[i].final != infinite ){
      backward[i]=0;
    }
  }

  /* Let's parse the word graph backwards from the final states */
/*  while(!empty_backward_ordered_list(states_to_be_visited_backwards)){
   
    state_id = (int) extract_from_backward_ordered_list(states_to_be_visited_backwards);
    
    for (e=0;e<wg->wg[state_id].to.l;e++){
      curr_edge = wg->wg[state_id].to.edv[e];
        
      curr_prob = curr_edge->prob + backward[curr_edge->to];
      backward[state_id]=add_log(backward[state_id],curr_prob);     
           
    }
  }
    
  
  destroy_ordered_list(states_to_be_visited_backwards);
   
}*/
  
 /*****************************************************************************/

/** \d normalize the word graph wg */
void normalizeWG(wg_t *wg,float factEntropy){
  
  double *backward=NULL;
  edge_t *curr_edge;
  int i,e;
  
  mt(backward = (double *) malloc(wg->size * sizeof(double)));
  for(i=0;i<wg->size;i++) backward[i]=LOG(0.0);
   
  for(i=0;i<wg->size;i++){
     for (e=0;e<wg->wg[i].to.l;e++){
         curr_edge = wg->wg[i].to.edv[e];
         wg->wg[i].to.edv[e]->prob=curr_edge->prob*factEntropy;
        }
  }
  
  
  computeBackward(backward,wg);
  
  
  for(i=0;i<wg->size;i++){
     for (e=0;e<wg->wg[i].to.l;e++){
         curr_edge = wg->wg[i].to.edv[e];
         wg->wg[i].to.edv[e]->prob=curr_edge->prob+backward[curr_edge->to]-backward[curr_edge->from];
        }
  }
    
}




/************************* Functions of lat ********************************/

/** \d Save lat */

void lat_save(FILE *file,wg_t *word_graph) {
  
    int i,e;
    edge_t *curr_edge;

    fprintf(file, "Lattice statistics: \n");
    fprintf(file, "Number of nodes: %d\n", word_graph->size);

    for(i=0;i<word_graph->size;i++){
        fprintf(file, "I=%d  f=%f\n",i,word_graph->wg[i].final);   
    }
    for(i=0;i<word_graph->size;i++){
        for (e=0;e<word_graph->wg[i].to.l;e++){
	    curr_edge = word_graph->wg[i].to.edv[e];
	    fprintf(file, "S=%d  E=%d  W=%s  a=%f l=%f\n",curr_edge->from,curr_edge->to,get_token_dict(word_graph->dict, curr_edge->output),curr_edge->hmm_prob,curr_edge->lm_prob);   
   
	}
    }
 
}

/** \d Function to create the lat */

wg_t *lat_create() {
  wg_t *word_graph=NULL;
  
  word_graph = (wg_t *)calloc(1,sizeof(wg_t)); 
 
  return word_graph;
}

/** \d Function to create the online lat */

//wg_t *lat_create_online() {
  
//  word_graph_online = (wg_t *)calloc(1,sizeof(wg_t)); 
 
//  return word_graph_online;
//}

/** \d Function to create the unigram */

//wg_t *lat_create_unigram() {
  
//  unigram = (wg_t *)calloc(1,sizeof(wg_t)); 
 
//  return unigram;
//}

/** \d Function to create the online lat */

//wg_t *online_lat_create() {
  
//  word_graph_online = (wg_t *)calloc(1,sizeof(wg_t)); 
 
//  return word_graph_online;
//}
