/***************************************************************************
                          CAT.c  -  description
                             -------------------
    begin                : Sun May 9 2004
    copyright            : (C) 2004 by Instituto Tecnologico de Informatica
 ***************************************************************************/
/** \file
\brief \d This file implements basic functions for computer-assisted transcription of handwritten text images.
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
//#include "TT2.h"

#define FALSE 0
#define TRUE 1

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#ifdef DEMO
#define MAX_SUFFIX_LENGTH 40
#endif

/******************************************************************************/
/**************************VARIABLES*******************************************/
/******************************************************************************/


/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/*****************************************************************************/
/** \d This function destroy the data structures of the previous sentence and 
    creates the data structure for the current input_sentence, that is, 
    the word graph, error-correcting parsing and n-best search */
int set_input_sentence(char *name,float gsf, float wip,float ECF,wg_t **wg,int ECAlg){

  ec_state_t *set_ec_state;
  int empty_symbol;
  int initial_symbol;
  int final_symbol;


/*********************************************************************************/
/* DESTROY DATA STRUCTURES THAT WERE CREATED TO TRANSLATE THE PREVIOUS SENTENCE: */
/* ERROR-CORRECTING PARSING, N-BEST SEARCH AND THE WORD GRAPH                    */
/*********************************************************************************/

/* Destroy state of the ec parser */
   set_ec_state = get_ec_state();
 //  fprintf(stderr, "set_ec_state = 0x%X\n", set_ec_state);
   destroy_ec_state(&set_ec_state);

/* It destroy the word graph and all nsuffix data structures */
   destroy_nbestsuffix();

/* Destroy word graph to be explored */
   
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
   if(empty_symbol<0){
     empty_symbol=INT_MAX;
   }
   set_empty_symbol_word_graph(empty_symbol);
   

  /*Set the initial symbol in word graph */
   initial_symbol = get_id_dict((*wg)->dict,"<s>");
   if(initial_symbol < 0){ 
     initial_symbol=INT_MAX;
   }
   set_initial_symbol_word_graph(initial_symbol);

  /*Set the final symbol in word graph */
   final_symbol = get_id_dict((*wg)->dict,"</s>");
   if(final_symbol < 0) { 
     final_symbol=INT_MAX;
   }
   set_final_symbol_word_graph(final_symbol);

  /* Set the GSF and the WIP values*/
   if(gsf!=MAXFLOAT) set_GSF_word_graph(gsf,*wg);
   if(wip!=MAXFLOAT) set_WIP_word_graph(wip,*wg);

   compute_log_probability(*wg);
  /*sort the word graph dictionary*/ 
   sort_dict((*wg)->dict); 

   
    
   
   
/******************************************/
/* PREPARING THE ERROR-CORRECTING PARSER  */
/******************************************/
/* Initialize Error-Correcting Parser */
/* Setting word graph into Error-Correcting Parser */
  set_word_graph_ecparser(*wg);

  /* Setting empty symbol into Error-Correcting Parser
   The deletion cost of this symbol is ZERO */
  set_empty_symbol_ecparser(empty_symbol);

  /*Set the initial symbol in  Error-Correcting Parser*/
  set_initial_symbol_ecparser(initial_symbol);

  /*Set the final symbol in Error-Correcting Parser */
  set_final_symbol_ecparser(final_symbol);

/* Setting the error correcting scale fator */
  set_ecsf_ecparser(ECF);

/* Start ec parsing from the initial state and computing a deletion operation */
  reset_ecparser(ECAlg);



/******************************************/
/*      PREPARING THE N-BEST SEARCH       */
/******************************************/  
/* Initialize n-best search */
/* Setting word graph into Error-Correcting Parser */
  set_word_graph_nbestsuffix(*wg); 
 
 /* Setting empty symbol into N-best Search
    empty symbol will be ignored when generating translations */
  set_empty_symbol_nbestsuffix(empty_symbol);
 
  /*Set the initial symbol in  N-best Search*/
  set_initial_symbol_nbestsuffix(initial_symbol);

  /*Set the final symbol in N-best Search */
  set_final_symbol_nbestsuffix(final_symbol);

  /* Initialize suffix search, we just store all the information that we pass as parameters */
  create_nbestsuffix();
 
  /* Perform initial backward search over the word graph */
  initialize_nbestsuffix_search();
/****************************************************************/

 //word_graph2v2_format_file_simple((*wg)->dict,*wg);

  
  return 1;

}

int set_online_word(char *path,char *prev, char *word, char *error,wg_t **wg){

  //wg_t *wg;
  char *name;

/*********************************************************************************/
/* DESTROY DATA STRUCTURES THAT WERE CREATED TO TRANSLATE THE PREVIOUS SENTENCE: */
/* ERROR-CORRECTING PARSING, N-BEST SEARCH AND THE WORD GRAPH                    */
/*********************************************************************************/

/* Destroy word graph to be explored */
   destroy_word_graph(wg);


/**************************************************************/
/* OBTAIN WORD GRAPH TO PERFORM OPERATIONS ON IT AND PREPARING IT*/
/**************************************************************/
  //Create lat	
   *wg = lat_create();

  //create name of the online word-graph
   mt(name= (char *) malloc(sizeof(char)*1));
   name[0]='\0';
   name=(char *)realloc(name,sizeof(char)*strlen(path)+2);
   name=strcat(name,path);
   name=strcat(name,"/");
   name=(char *)realloc(name,sizeof(char)*strlen("file_")+strlen(name)+1);
   name=strcat(name,"FILE_");
   name=(char *)realloc(name,sizeof(char)*strlen(word)+strlen(name)+2);
   name=strcat(name,word);
   name=strcat(name,".");
   name=(char *)realloc(name,sizeof(char)*strlen(prev)+strlen(name)+2);
   name=strcat(name,prev);
   name=strcat(name,"_");
   name=(char *)realloc(name,sizeof(char)*strlen(error)+strlen(name)+1);
   name=strcat(name,error);
   //name=(char *)realloc(name,sizeof(char)*strlen(".lat")+strlen(name)+1);
   //name=strcat(name,".lat");
   name[strlen(name)]='\0';
   
  //Copy the information of the file in the structure lat        
   lat_load(*wg,zopen(name,"r"));
 
/* Let's assign to each state, its topological order. This operation MUST be carried out after setting the word graph */
   topo_order_word_graph(*wg);

  /*sort the word graph dictionary*/ 
   sort_dict((*wg)->dict);  

   set_word_graph_online_ecparser(*wg);


  return 1;

}

/** \d Set the unigram language model to carry out the CATTI at character level. If the word to complete the prefix word is not on the word-graph we look for on the unigram */

int set_input_unigram(char *name,wg_t **unigram){

  //wg_t *unigram;
  
/* Destroy the unigram word graph */
   destroy_word_graph(unigram);


  //Create lat	
   *unigram = lat_create();

   //Copy the information of the file in the structure lat        
   lat_load(*unigram,zopen(name,"r"));
 
  /*sort the word graph dictionary*/ 
   sort_dict((*unigram)->dict);  

   //set_unigram(unigram);


  return 1;

}
/***************************************************************************/
/** \d Set the new prefix to be parsed, taking into account if the previous 
    prefix is a prefix of the new prefix, so that, the error-correcting parsing
    is performed incrementally */
int *set_output_prefix(int *prefix,wg_t *wg, int ECAlg){

  static int *prev_prefix=NULL;
  static int prev_prefix_length=0;
  int i, prefix_length;
  ec_state_t *set_ec_state;
  static int *states_order=NULL;

  //fprintf(stderr,"\ncatti-plugin: prev_prefix_length '%d'\n",prev_prefix_length);fflush(stderr); 
   
  if(states_order==NULL)  mt(states_order = (int *) malloc(wg->size*sizeof(int))); 
  /* Compute the length of the new prefix */
  for(prefix_length=0;prefix[prefix_length] != NO_WORD;prefix_length++);

  /* If the prefix is empty, just return, remembering that was empty*/
   if(prefix_length==0){
     prev_prefix_length = 0;
     if(prev_prefix) free(prev_prefix);
     mt(prev_prefix = (int *) malloc(sizeof(int)));
     prev_prefix[0] = NO_WORD;
     if(states_order) free(states_order);
     states_order=NULL;
     return states_order;
   }

  if(prefix_length==1){
     prev_prefix_length = 0;
     if(prev_prefix) free(prev_prefix);
     mt(prev_prefix = (int *) malloc(sizeof(int)));
     prev_prefix[0] = NO_WORD;
     reset_ecparser(ECAlg);
     if(states_order) free(states_order);
     states_order=NULL;
   }

  /* If we are adding words to the previous prefix */
  if(prefix_length >= prev_prefix_length){

    /* Move to the new part of the prefix */
    for(i=0;(i<prev_prefix_length) && (prev_prefix[i]==prefix[i]);i++);

    /* If we are dealing with a suffix of the previous prefix */
    if(i == prev_prefix_length)
      for(i=prev_prefix_length; i<prefix_length; i++){
         if(tipo_CAT==TRANSCRIPCION)
       	     advance_parse_transcripcion(prefix[i],states_order,ECAlg);
         if(tipo_CAT==TRADUCCION)
             advance_parse_traduccion(prefix[i]);
       }
  }
  /* We are dealing with a new prefix, so let's parse from the beginning */
  else{
    set_ec_state = get_ec_state();
    destroy_ec_state(&set_ec_state);
    reset_ecparser(ECAlg);
    for(i=0; i<prefix_length; i++)
         if(tipo_CAT==TRANSCRIPCION)
       	     advance_parse_transcripcion(prefix[i],states_order,ECAlg);
         if(tipo_CAT==TRADUCCION)
             advance_parse_traduccion(prefix[i]);       
  }/* End of else of if(prefix_lenth >= prev_prefix_length){ */

  /* prefix becomes prev_prefix */
  if(prev_prefix) free(prev_prefix);
  mt(prev_prefix = (int *) malloc((prefix_length+1)*sizeof(int)));
  memcpy(prev_prefix, prefix, (prefix_length+1)*sizeof(int));
  prev_prefix_length = prefix_length;

 // fprintf(stderr,"\ncatti-plugin: prev_prefix_length '%d'\n",prev_prefix_length);fflush(stderr); 
  
  return states_order;
}/* End of void set_output_prefix(int *prefix){ */


/***************************************************************************/
//This function computes the cost to transform the word hip on the word ref using automatic completed of the word

int cost(int ref, int hip, dict_t  *wg_dict,dict_t *unigram_dict){

  char *ref_word=NULL;
  char *hip_word=NULL;
  int prefix_character_position=0;
  int find=FALSE;
  int ks_count=0;
  char* last_word_in_prefix=NULL;
  //dict_t *wg_dict;
  int first_word_in_suffix = NO_WORD;

  //wg_dict=get_word_graph_dict();

  ref_word=get_token_dict(wg_dict,ref);
  if (hip!=NO_WORD) hip_word=get_token_dict(wg_dict,hip);

  
  if(search_in_dict(unigram_dict,ref_word)==NO_WORD)  {
    if(hip_word!=NULL) ks_count = char_leven_dist(ref_word,hip_word); 
    else ks_count=ks_count+ strlen(ref_word); 
    find=TRUE;
  }
  
  while(!find){

      if(hip_word!=NULL){
          while((ref_word[prefix_character_position]!='\0') && (hip_word[prefix_character_position]!='\0') && (ref_word[prefix_character_position]==hip_word[prefix_character_position])){
                         prefix_character_position++;
          }
      }
      prefix_character_position++;
      
      if((ref_word[prefix_character_position-1]=='\0') && (hip_word[prefix_character_position-1]=='\0')){
           find=TRUE;
      }
      else{
         if((ref_word[prefix_character_position-1]=='\0') && (hip_word[prefix_character_position-1]!='\0')) { //if the hip is bigger than the ref i add a space
             ks_count++;
             find=TRUE;
         }
         else{
	     ks_count++; 
             last_word_in_prefix=strndup2(ref_word,prefix_character_position);
             first_word_in_suffix = search_in_dict(unigram_dict, last_word_in_prefix);
//	     if(first_word_in_suffix==NO_WORD){
//	       if(hip_word!=NULL) ks_count = ks_count + char_leven_dist(ref_word,hip_word); 
//	       else ks_count=ks_count+ strlen(ref_word);
//	       first_word_in_suffix = set_token_dict(wg_dict,get_token_dict(get_unigram_dict(), set_token_dict(get_unigram_dict(),last_word_in_prefix)));
//	       find=TRUE;
//	     }
//	     else{
	     first_word_in_suffix = set_token_dict(wg_dict,get_token_dict(unigram_dict, first_word_in_suffix));
	     //};

             hip_word= get_token_dict(wg_dict,first_word_in_suffix);
         }
      }
   }
   return ks_count;

}


/***************************************************************************/
/** \d Add n-best suffix to n-best suffices and sum probs if necessary */
void add2nbest_suffices(vector_t *suffix, float suffix_prob, 
   vector_t *suffices, float *suffices_prob, int *curr_completion, dict_t *outdict){

int i;
#ifdef DEMO
  int j,actual_suffix_length;
#endif  

    /* We need to check if this n-best suffix was already obtained */
#ifdef DEMO
    for(i=0;i<*curr_completion;i++){
      j=0;
      actual_suffix_length = 0;
      while((j<suffix->size)&&(j<suffices[i].size)&&(actual_suffix_length + strlen(get_token_dict(outdict,(int) suffix->element[j]))<=MAX_SUFFIX_LENGTH)
      &&(suffix->element[j]==suffices[i].element[j])){
        actual_suffix_length += strlen(get_token_dict(outdict,(int) suffix->element[j]))+1;
        j++;
      }

      if((j<suffix->size)&&(j<suffices[i].size)
      &&(actual_suffix_length + strlen(get_token_dict(outdict,(int) suffix->element[j]))>MAX_SUFFIX_LENGTH))
        j = suffix->size;

      if(suffix->size == j){
        suffices_prob[i] += exp(suffix_prob);
        break;
      }
    }
#else
    /* If suffices are equal, them sum their probabilities */
    for(i=0;i<*curr_completion;i++){
      if(equal_vector(suffix,&(suffices[i]))){
          suffices_prob[i] += exp(suffix_prob); break;
      }
    }
#endif
            
    /* We don't have this translation */  
    if(i == *curr_completion){
      copy_vector(suffix,&(suffices[i]));
      suffices_prob[i] = exp(suffix_prob);
      (*curr_completion)++;
    }
}

/***************************************************************************/
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
completions_t complete_prefix(char *prepro_prefix, int *int_prefix, char *last_word_in_prefix, int number_of_completions, dict_t  *outdict,float beam,vector_t *first_word_last_suffix, int multimodality,int pot,unsigned char* errorCorrecting,wg_t *wg,wg_t *unigram,int ECAlg){

  int last_word_length;
  int first_word_in_suffix = NO_WORD;
  ec_state_t *old_ec_state;
  ec_state_t *act_ec_state; 
  float infinite=LOG(0.0);
  int curr_completion;
  list_t *best_states;
  int best_state;
  vector_t *suffix;
  vector_t *alignment;
  float suffix_prob;
  int suffix_index;
  float max_suffix_prob;  
  vector_t *suffices;
  vector_t *alignments;
  float *suffices_prob;
  completions_t completions;
  int i,j;
  int *state_words;
  char *suffix_word;
  char *lword=NULL; 
  int suffix_length;
  float *probs=NULL;
  float *probs_suffix=NULL;
  int *states_order=NULL;
  int id_last_word_in_prefix = NO_WORD;
  vector_t *v; 
#ifdef DEMO
  int nbests=0;
#endif  

  /******************************************/
  /* ERROR-CORRECTING PARSING ON THE PREFIX */
  /******************************************/

  /* First, we are going to parse the prepro_prefix of the user but the last prepro_prefix word */
  states_order=set_output_prefix(int_prefix,wg,ECAlg);

  if(multimodality){
      old_ec_state = tentative_advance(NO_WORD,ECAlg);
      advance_parse_online(&state_words,pot);       
      if(tipo_CAT==TRANSCRIPCION)
         best_states = best_states_after_ec_parsing_transcripcion(beam);      
      if(tipo_CAT==TRADUCCION)
         best_states = best_states_after_ec_parsing_traduccion(beam);
      /* Recover error-correcting state after tentative parsing */
      probs=get_prob_ecparser();
      set_ec_state(old_ec_state);
  }
  else{ 
      /* Obtain best_states after error-correcting parsing from which start n-best search */
      if(tipo_CAT==TRANSCRIPCION)
         best_states = best_states_after_ec_parsing_transcripcion(beam);      
      if(tipo_CAT==TRADUCCION)
         best_states = best_states_after_ec_parsing_traduccion(beam); 
      probs=get_prob_ecparser();
  }

  lword=last_word_in_prefix;
   /* If the last word in the prepro_prefix is NOT a whole prepro_prefix, let's find a
  word that is a prepro_prefix for it and parse it updating ec state */
  if(((last_word_length = strlen(last_word_in_prefix))>0)){ /* && (first_word_last_suffix->size==0)  ){ /*Comprobamos si no estamos en clicks a nivel de caracter */
     /* We have seen that the last word is not a complete word, so let's see
    if we are lucky and we found that the output of edges of the active states contains
    a prepro_prefix of the last word in prepro_prefix. In case of draw, take that one with highest prob */
     /*Si estamos en clicks a nivel de caracteres first_word_last_suffix tendrá una lista de palabras que el usuario ha rechazado, hay que comprobar que no se devuelve ninguna de esas*/

    set_initial_states_nbestsuffix(best_states);     
    probs_suffix=get_prob_nbestsuffix();

    first_word_in_suffix = search_in_state_edges_word_graph(best_states,last_word_in_prefix,last_word_length,outdict,wg,first_word_last_suffix,probs,probs_suffix); 
    
      
    /* If we haven't found any word in the edges of the active states,
       let's try in the word graph */
    /*Si estamos en clicks a nivel de caracteres first_word_last_suffix tendrá una lista de palabras que el usuario ha rechazado, hay que comprobar que no se devuelve ninguna de esas*/
    if(first_word_in_suffix == NO_WORD)       
      first_word_in_suffix = search_in_dict_word_graph(outdict, last_word_in_prefix,wg,first_word_last_suffix);
      
    /* If we haven't found any word that is a prepro_prefix of last_word_in_prefix
       in the word graph, try now in the whole vocabulary */
     if(first_word_in_suffix == NO_WORD){
      v = create_vector(1); 
      for(i=0;i<first_word_last_suffix->size;i++){ 
	mt(v->element = (void **) realloc(v->element, (v->size+1)*sizeof(void *)));
        v->size++;
        v->element[v->size-1]=get_id_dict(unigram->dict,get_token_dict(outdict,(int)first_word_last_suffix->element[i]));
      }
      first_word_in_suffix = search_in_dict_with_restrictions(unigram->dict, last_word_in_prefix,v);
      if(first_word_in_suffix != NO_WORD){
	  first_word_in_suffix=set_token_dict(outdict,get_token_dict(unigram->dict, first_word_in_suffix));
      }
      destroy_vector(v,1,0);
    }
    /* We have found a word that is a prepro_prefix of the last_word_in_prefix */
    if(first_word_in_suffix != NO_WORD){

      /* Free best_state list  */
      while(!EMPTY_LIST(best_states)) extract_from_list(best_states);
      DESTROY_LIST(best_states);
      /* Parse first_word_in_suffix updating best_states in n-best search */
      old_ec_state = tentative_advance(first_word_in_suffix,ECAlg);

      if(tipo_CAT==TRANSCRIPCION)
         best_states = best_states_after_ec_parsing_transcripcion(beam);      
      if(tipo_CAT==TRADUCCION)
         best_states = best_states_after_ec_parsing_traduccion(beam); 
      if(probs!=NULL) free(probs);
      probs=get_prob_ecparser();
      /* Recover error-correcting state after tentative parsing */
      set_ec_state(old_ec_state);
     }
     else{
        //if(get_id_dict(outdict, last_word_in_prefix)==-2){//TOKEN_NOT_FOUND_IN_DICT=-2
              id_last_word_in_prefix=set_token_dict(outdict, last_word_in_prefix);
	             /* Free best_state list  */
              while(!EMPTY_LIST(best_states)) extract_from_list(best_states);
              DESTROY_LIST(best_states);
              /* Parse first_word_in_suffix updating best_states in n-best search */
              old_ec_state = tentative_advance(id_last_word_in_prefix,ECAlg);

              if(tipo_CAT==TRANSCRIPCION)
                 best_states = best_states_after_ec_parsing_transcripcion(beam);      
              if(tipo_CAT==TRADUCCION)
              best_states = best_states_after_ec_parsing_traduccion(beam); 
              if(probs!=NULL) free(probs);
              probs=get_prob_ecparser();
        /* Recover error-correcting state after tentative parsing */
              set_ec_state(old_ec_state);
	//}
     }
     lword = strdup2(""); 
  
   }

   act_ec_state=get_ec_state();

   /* Obtain those states from which we will start calculating the n-best suffices */
   set_initial_states_nbestsuffix(best_states);
   DESTROY_LIST(best_states);


  /******************************************/
  /*     RUNNING N-BEST SUFFIX SEARCH       */
  /******************************************/

  /* Start n-best suffix */
  suffices = create_vector(number_of_completions);
  mt(suffices_prob = (float *) malloc(sizeof(float)*number_of_completions));
  for(i=0;i<number_of_completions;i++) suffices_prob[i]=0.0;

  alignments = create_vector(number_of_completions);
  
  
#ifdef VERBOSE
    fprintf(stderr,"Calculating n-best paths...\n");fflush(stderr);
#endif

  /* Extract number_of_completions different n-best suffices */
  curr_completion = 0;
  while(curr_completion < number_of_completions){
    /* Obtaining next n-best completion */
    suffix = create_vector(1);
    alignment = create_vector(1);
    if((suffix_prob = get_next_nbestsuffix(suffix,probs,first_word_last_suffix,state_words,multimodality,&best_state,lword,alignment))==infinite){
        /* next best suffix was not found, finish extracting n-best paths */
        destroy_vector(suffix,1,0); break;
	destroy_vector(alignment,1,0);break;
    }  
    
    *errorCorrecting=act_ec_state->ec_cost[best_state];
    
#ifdef DEMO
    nbests++;
#endif

    /* Ignore empty completion and ask another one */  
    if(suffix->size==0){
      destroy_vector(suffix,1,0); continue;
      destroy_vector(alignment,1,0); continue;
    }

     /*Add a new n-best alignment to a vector of n-best alignments already computed*/ 
    copy_vector(alignment,&(alignments[curr_completion]));      
  
    /* Add a new n-best suffix to a vector of n-best suffices already computed 
       and sum their probabilities if n-best suffix already exists */
     add2nbest_suffices(suffix,suffix_prob,suffices,suffices_prob,&curr_completion,outdict);
    
    /* We have finished with the n-best suffix just computed */
    destroy_vector(suffix,1,0);
    destroy_vector(alignment,1,0);

#ifdef DEMO     
    if(nbests==5000) break;
#endif    

  }/*  while(curr_completion < number_of_completions){ */

#ifdef VERBOSE
    fprintf(stderr,"n-best paths calculated...\n");fflush(stderr);
#endif
  
  /* Now we have obtained all n-bests that we needed, free best state list */
  destroy_initial_states_nbestsuffix();


  /***********************************************************************************/
  /* CONSTRUCT COMPLETIONS USING prepro_prefix + first_word_in_prefix + suffices[i]  */
  /***********************************************************************************/
  
  completions.completions = NULL;
  completions.alignments = NULL;

  /* In case we don't have any suffix, let's add the first word in the suffix */
  curr_completion = (curr_completion!=number_of_completions)?curr_completion+1:number_of_completions;
  
  if(curr_completion>0){
   mt(completions.completions = (char **) malloc(sizeof(char *)));
   mt(completions.alignments = (alignment_t **) malloc(sizeof(alignment_t *))); 
  }
  
   /* We need to sort completions by probabilities */
  for(j=0;j<curr_completion;j++){
    max_suffix_prob = -1.0;
    suffix_index = -1;
    for(i=0;i<curr_completion;i++){
      if(suffices_prob[i] > max_suffix_prob){
        max_suffix_prob = suffices_prob[i];
        suffix_index = i;
      }
    }
    mt(completions.completions = (char **) realloc(completions.completions, (j+1)*sizeof(char *)));
    mt(completions.alignments = (alignment_t **) realloc(completions.alignments, (j+1)*sizeof(alignment_t *))); 
 
      
    /* Make sure we have a valid suffix_index */
    if(suffix_index !=-1){
      
      /* Make sure we don't select this suffix again */
      suffices_prob[suffix_index] = -1.0;


      /* The last word typed by the user will remain as it is */
      if((first_word_in_suffix == NO_WORD)){
	if((strlen(lword)==0)||(suffices[suffix_index].size==0)){
	  completions.completions[j] = strdup2(prepro_prefix);
	  suffix_length = strlen(completions.completions[j])+1;
        }else{ /*Estamos en clicks*/  /* Copy to completions the prepro_prefix leaving out the last_word_in_prefix */
	    mt(completions.completions[j] = (char *) malloc((last_word_in_prefix - prepro_prefix + 1)*sizeof(char)));
	    memcpy(completions.completions[j], prepro_prefix, (last_word_in_prefix - prepro_prefix)*sizeof(char));
	    completions.completions[j][last_word_in_prefix-prepro_prefix] = '\0';
	    suffix_length = strlen(completions.completions[j])+1;
	} 
      }
      /* We have a completion for the prepro_prefix of the last word */
      else{
        /* Copy to completions the prepro_prefix leaving out the last_word_in_prefix */
        mt(completions.completions[j] = (char *) malloc((last_word_in_prefix - prepro_prefix + 1)*sizeof(char)));
        memcpy(completions.completions[j], prepro_prefix, (last_word_in_prefix - prepro_prefix)*sizeof(char));
        completions.completions[j][last_word_in_prefix - prepro_prefix] = '\0';
        
        /* Then add the first_word_in_suffix word to the current completion */
        suffix_length = strlen(completions.completions[j]);
        suffix_length += strlen(get_token_dict(outdict,first_word_in_suffix)) + 1;
        mt(completions.completions[j] = (char *) realloc(completions.completions[j], suffix_length*sizeof(char)));
        strcat(completions.completions[j], get_token_dict(outdict,first_word_in_suffix));
      }
       
      mt(completions.alignments[j] = (alignment_t *) malloc((alignments[suffix_index].size + 1)*sizeof(alignment_t)));
                            
      for(i=0;i<suffices[suffix_index].size;i++){
        /* First check if we need to append a white space to the current completion 
           that is when is NOT the first word in the suffix or the prefix doesn't 
           end in a white space as it's supposed to be if first_word_in_suffix != NO_WORD */
        if((i!=0)||((strlen(prepro_prefix)>0)&&(completions.completions[j][strlen(prepro_prefix)-strlen(lword)-1]!=' '))){
          suffix_length++;
          mt(completions.completions[j] = (char *) realloc(completions.completions[j], suffix_length*sizeof(char)));
          strcat(completions.completions[j], " ");
	}

        /* Now append the next word in the suffix after the white space if that was the case */
        suffix_word = get_token_dict(outdict,(int) suffices[suffix_index].element[i]);
        /* Taking into account white space between words */
        suffix_length += strlen(suffix_word);
        mt(completions.completions[j] = (char *) realloc(completions.completions[j], suffix_length*sizeof(char)));
        strcat(completions.completions[j], suffix_word);        
      }
     // }
     for(i=0;i<alignments[suffix_index].size;i++){
	completions.alignments[j][i].start = ((alignment_t *) alignments[j].element[i])->start;
	completions.alignments[j][i].end = ((alignment_t *) alignments[j].element[i])->end;
     }
      //completions.alignments[j][i]=NULL;
      completions.alignments[j][alignments[suffix_index].size].start=-1.0;
      completions.alignments[j][alignments[suffix_index].size].end=-1.0;
    }/* End of if(suffix_index !=-1){ */
    else{
      mt(completions.completions[j] = (char *) malloc(sizeof(char)));
      completions.completions[j][0] = '\0';
    }    
  }
  mt(completions.completions = (char **) realloc(completions.completions, (j+1)*sizeof(char *)));
  completions.completions[j] = NULL;
  
  mt(completions.alignments = (alignment_t **) realloc(completions.alignments, (j+1)*sizeof(alignment_t *)));
  completions.alignments[j] = NULL;
  free(probs);
  free(suffices_prob);
  
  /* Free suffices */
  destroy_vector(suffices, number_of_completions, 0);
  destroy_vector(alignments, number_of_completions, 1);
  return completions;

}/* End of char **complete_prefix(char *last_word_in_prefix, char *first_word_in_suffix, int number_of_completions){ */
