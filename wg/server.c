#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <unistd.h>
#include <ctype.h>
#include <values.h>
#include <errno.h>

#include "global.h"
#include "auxiliar.h"
#include "vector.h"
#include "dynamic_list.h"
#include "dynamic_ordered_list.h"
#include "hash.h"
#include "dictionary.h"
#include "heap.h"
#include "strop.h"
#include "librefx.h"
#include "word_graph.h"
#include "ecparser.h"
#include "nbestsuffix.h"
#include "CAT.h"
#include <cat/cat_server.h>
#include <cat/daemon.h>
#include <strokes/strokes.h>
#include <libgen.h>

#define FALSE 0
#define TRUE 1
#define BEST_COMPLETION 1
#define NBEST_COMPLETIONS 1

#ifndef REVISION
#define REVISION " unknown"
#endif

float GSF=MAXFLOAT;
float WIP=MAXFLOAT; 
float ECF=-350;
float w_beam=MAXFLOAT;
int *int_trans = NULL;
char *config_fn = NULL;
char *old_prefix=NULL;
vector_t *first_word_last_suffix;
int may=1;

/*********************************************************************************/
return_code_t provide_completions(user_info_t *UNUSED(user), const char *prefix, cat_operation_t UNUSED(op), int nbest, suffix_info_t **completions) {
  completions_t ipe_completions;
  char *prefix_sp = NULL;
  int error, i;
  printf("setting prefix: '%s'\n", prefix);
  vector_t *first_word_last_suffix = create_vector(1);
  prefix_sp = (char *) malloc(strlen(prefix)+2);
  strcpy(prefix_sp, prefix);
  strcat(prefix_sp, " ");
  if(!(error = ipeSetTargetText(prefix_sp))) return CAT_UNK_ERROR;
  
  ipe_completions.completions=NULL;
  /* Set the maximum number of N-best translations */
  ipeSetMaxCompletions(nbest);

  /* Compute the posible sufix*/
  if(!(error = ipeProvideCompletions(&ipe_completions,strlen(prefix_sp),w_beam,first_word_last_suffix, FALSE, 1))) return CAT_UNK_ERROR;
  free(prefix_sp);
  
  for(i=0;i < nbest && ipe_completions.completions[i]!=NULL;i++){
    completions[i]->suffix=strdup(ipe_completions.completions[i]);
    printf("setting suffix[%d]: '%s'\n",i,completions[i]->suffix);
  }
  fflush(stdout);

  destroy_vector(first_word_last_suffix,1,0);
  int c=0;
  while(ipe_completions.completions[c]!=NULL){
	  free(ipe_completions.completions[c++]); 			
  }
  free(ipe_completions.completions);

  while(ipe_completions.alignments[c]!=NULL){
       free(ipe_completions.alignments[c]); 			
       c++;
  }
  free(ipe_completions.alignments);
  ipe_completions.alignments=NULL;
 
  return CAT_OK;
}

/*********************************************************************************/
return_code_t set_prefix(user_info_t * UNUSED(user), const char *prefix, cat_operation_t op, suffix_info_t *info) {
  completions_t completions;
  char *prefix_sp = NULL;
  int error;
  int pref_word_length,suf_word_length,i;
  char *word=NULL; 
  dict_t *wg_dict;
  int *int_completion = NULL;
  char *n_prefix=NULL;
 
  completions.completions=NULL;  

  n_prefix=strdup2(prefix);
  if(n_prefix!=NULL) {
     if(may==1) strupr(n_prefix);
     mt(prefix_sp = (char *) malloc(sizeof(char)*strlen(n_prefix)+2));
     strcpy(prefix_sp, n_prefix);
     strcat(prefix_sp, " ");
     
  }
  else{
    mt(prefix_sp = (char *) malloc(sizeof(char)*1));
    prefix_sp[0]='\0';
  }
  printf("prefix_sp: %s\n", prefix_sp);
  printf("setting prefix: '%s'\n", prefix);
  //printf("strcmp(old_prefix,prefix_sp) %d \n", strcmp(old_prefix,prefix_sp));
  if(old_prefix!=NULL){
      if( strcmp(old_prefix,prefix_sp)!=0) {
	 destroy_vector(first_word_last_suffix,1,0);
	 first_word_last_suffix = create_vector(1);
      }
      
  }
  old_prefix = (char *) malloc(sizeof(char)*strlen(prefix_sp)+1);
  strcpy(old_prefix,prefix_sp);
  

  /* Set the maximum number of N-best transcriptions */
  ipeSetMaxCompletions(1);

  /* Count number of words  */
  pref_word_length = nwords(prefix_sp); 
 
  
  /*Obtain de dictionary of the word_graph */
  wg_dict=get_word_graph_dict();
  
  switch (op) {
      case CO_REFRESH: printf("op: CO_NONE\n");
           if(int_trans!=NULL) free(int_trans);
           int_trans=str2seq(wg_dict,prefix_sp);
           return CAT_OK;
      case CO_NONE: printf("op: CO_NONE\n");
      case CO_SUBSTITUTE: printf("op: CO_SUBSTITUTE\n");
          break;
      case CO_INSERT: printf("op: CO_INSERT\n");
         if(int_trans[pref_word_length-1]!=-1){ 
		word=get_token_dict(wg_dict,int_trans[pref_word_length-1]);
         	prefix_sp=(char *)realloc(prefix_sp,sizeof(char)*strlen(prefix_sp)+strlen(word)+2);
         	strcat(prefix_sp,word);
         	strcat(prefix_sp, " ");
	 }
         break;
      case CO_DELETE: printf("op: CO_DELETE\n");
         if(int_trans[pref_word_length+1]!=-1) word=get_token_dict(wg_dict,int_trans[pref_word_length+1]);
         else word="</s>";
         prefix_sp=(char *)realloc(prefix_sp,sizeof(char)*(strlen(prefix_sp)+strlen(word)+2));
         strcat(prefix_sp,word);
         strcat(prefix_sp, " ");
         break; 
      case CO_REJECT: printf("op: CO_REJECT\n");
         mt(first_word_last_suffix->element = (void **) realloc(first_word_last_suffix->element, (first_word_last_suffix->size+1)*sizeof(void *)));
         first_word_last_suffix->size++;
         first_word_last_suffix->element[first_word_last_suffix->size-1] = (void *) int_trans[pref_word_length]; 
         break;
     case CO_ACCEPT: printf("op: CO_ACCEPT\n");
         return CAT_OK;
  }
  
  if(!(error = ipeSetTargetText(prefix_sp))) return CAT_UNK_ERROR;

   /* Compute the posible sufix*/
  if(!(error = ipeProvideCompletions(&completions,strlen(prefix_sp),w_beam,first_word_last_suffix, FALSE,1))) return CAT_UNK_ERROR;
  printf("completions[0]: %s\n", completions.completions[0]);
  if(word!=NULL && strcmp(word,"</s>")!=0){
     pref_word_length++;
     info->suffix=strdup(word);
     if(completions.completions[0]!=NULL){
	 info->suffix=(char *)realloc(info->suffix,sizeof(char)*(strlen(info->suffix)+strlen(completions.completions[0])+2));
         strcat(info->suffix," ");
	 strcat(info->suffix,completions.completions[0]);
	}	
  }
  else {
	if(completions.completions[0]!=NULL){
   	 info->suffix=strdup(completions.completions[0]);
  	}
  }
  
  if(may==1) strlow(info->suffix);
     /* Count number of words  */
  suf_word_length = nwords(completions.completions[0]);   

  if(int_completion!=NULL) free(int_completion);
  int_completion=str2seq(wg_dict,completions.completions[0]);
  
  if(int_trans!=NULL) free(int_trans);
  int_trans=str2seq(wg_dict,prefix_sp);
  int_trans=(int *) realloc(int_trans,sizeof(int)*(pref_word_length+suf_word_length+1));
  for(i=0;i<suf_word_length;i++) int_trans[pref_word_length+i]=int_completion[i];
  int_trans[pref_word_length+i]=-1;

  printf("setting suffix: '%s'\n", info->suffix);
  fflush(stdout);

  int c=0;
  while(completions.completions[c]!=NULL){
	  free(completions.completions[c++]); 			
  }
  free(completions.completions);

 
  c=0;
  while(completions.alignments[c]!=NULL){
       free(completions.alignments[c]); 			
       c++;
  }
  free(completions.alignments);
  completions.alignments=NULL;
  
  free(n_prefix);
  free(prefix_sp);
 
  return CAT_OK;
}

/*********************************************************************************/
return_code_t set_prefix_online_strokes(user_info_t * UNUSED(user), const char *prefix, strokes_t *strokes, cat_operation_t op, suffix_info_t *info)
 {
  completions_t completions;
  char *prefix_sp = NULL;
  int error,i;
  int pref_word_length,suf_word_length;
  char *word=NULL; 
  dict_t *wg_dict=NULL;
  stroke_decoder_t decoder;
  char *word_decoded = NULL, *w=NULL;
  char *n_prefix=NULL;
  int *int_completion=NULL;

  completions.completions=NULL;

  n_prefix=strdup2(prefix);
  if(n_prefix!=NULL) {
    if(may==1)  strupr(n_prefix);
    prefix_sp = (char *) malloc(sizeof(char)*strlen(n_prefix)+2);
    strcpy(prefix_sp, n_prefix);
    strcat(prefix_sp, " ");
  }
  else{
    mt(prefix_sp = (char *) malloc(sizeof(char)*1));
    prefix_sp[0]='\0';
  }


  printf("setting prefix: '%s'\n", prefix);
  vector_t *first_word_last_suffix = create_vector(1);

 
  /* Set the maximum number of N-best transcriptions */
  ipeSetMaxCompletions(1);

  strcpy(decoder.config_fn, config_fn);
  strcpy(decoder.script_fn, "/home/demo/catti-server/catti/strokes_decode.sh");
 
  /* Count number of words  */
  pref_word_length = nwords(prefix_sp); 

  /*Obtain de dictionary of the word_graph */
  wg_dict=get_word_graph_dict();
  
  switch (op) {
      case CO_REFRESH: printf("op: CO_NONE\n");
          if(int_trans!=NULL) free(int_trans);
          int_trans=str2seq(wg_dict,prefix_sp);
          return CAT_OK;
      case CO_NONE: printf("op: CO_NONE\n");
      case CO_SUBSTITUTE: printf("op: CO_SUBSTITUTE\n");
          if(pref_word_length>0) w=get_token_dict(wg_dict,int_trans[pref_word_length-1]);
	  else w="";
          strokes_decode(&decoder, w , get_token_dict(wg_dict,int_trans[pref_word_length]) , strokes, &word_decoded);
          prefix_sp=(char *)realloc(prefix_sp,sizeof(char)*(strlen(prefix_sp)+strlen(word_decoded)+2));
          strcat(prefix_sp,word_decoded);
          strcat(prefix_sp, " ");
          break;
      case CO_INSERT: printf("op: CO_INSERT\n");
          strokes_decode(&decoder, get_token_dict(wg_dict,int_trans[pref_word_length-1]) , "" , strokes, &word_decoded);
          prefix_sp=(char *)realloc(prefix_sp,sizeof(char)*(strlen(prefix_sp)+strlen(word_decoded)+2));
          strcat(prefix_sp,word_decoded);
          strcat(prefix_sp, " ");
          if(int_trans[pref_word_length]!=-1){ 
             word=get_token_dict(wg_dict,int_trans[pref_word_length]);
             prefix_sp=(char *)realloc(prefix_sp,sizeof(char)*(strlen(prefix_sp)+strlen(word)+2));
             strcat(prefix_sp,word);
             strcat(prefix_sp, " ");
             
	  }
          break;
      default: printf("Operation code not allowed\n");
               return CAT_UNK_ERROR;
  }
  
  if(!(error = ipeSetTargetText(prefix_sp))) return CAT_UNK_ERROR;

  /* Compute the posible sufix*/
  if(!(error = ipeProvideCompletions(&completions,strlen(prefix_sp),w_beam,first_word_last_suffix, FALSE,1))) return CAT_UNK_ERROR;
  
  pref_word_length++;
  printf("Word decoded: %s\n", word_decoded);
  info->suffix=strdup(word_decoded);
  printf("completions[0]: %s\n", completions.completions[0]);
  if(word!=NULL){
     pref_word_length++;
     info->suffix=(char *)realloc(info->suffix,sizeof(char)*(strlen(info->suffix)+strlen(word)+2));
     strcat(info->suffix," ");
     strcat(info->suffix,word);
  }
  if(completions.completions[0]!=NULL){
     info->suffix=(char *)realloc(info->suffix,sizeof(char)*(strlen(info->suffix)+strlen(completions.completions[0])+2));
     strcat(info->suffix," ");
     strcat(info->suffix,completions.completions[0]);
  }	

 if(may==1) strlow(info->suffix);
    /* Count number of words  */
  suf_word_length = nwords(completions.completions[0]);   

  if(int_completion!=NULL) free(int_completion);
  int_completion=str2seq(wg_dict,completions.completions[0]);
  
  if(int_trans!=NULL) free(int_trans);
  int_trans=str2seq(wg_dict,prefix_sp);
  int_trans=(int *)realloc(int_trans,sizeof(int)*(pref_word_length+suf_word_length));
  for(i=0;i<suf_word_length;i++) int_trans[pref_word_length+i]=int_completion[i];

  printf("setting suffix: '%s'\n", info->suffix);
  fflush(stdout);

  int c=0;
  while(completions.completions[c]!=NULL){
	  free(completions.completions[c++]); 			
  }
  free(completions.completions);

  while(completions.alignments[c]!=NULL){
       free(completions.alignments[c]); 			
       c++;
  }
  free(completions.alignments);
  completions.alignments=NULL;


  free(n_prefix);
  free(prefix_sp);
 
  return CAT_OK;
}


/*********************************************************************************/

return_code_t set_source(user_info_t * user, const char *source, suffix_info_t *info) {
  int error;
  printf("setting source: '%s'\n", source);
  char * wg_fn = strdup(source);

  printf("wordgraph: '%s'\n", wg_fn);
  first_word_last_suffix = create_vector(1);
  if(!(error = set_input_sentence(wg_fn,GSF,WIP,ECF)))  return CAT_UNK_ERROR;
  set_prefix(user, "", CO_NONE, info);
  
  free(wg_fn);
  
  return CAT_OK;
}


void usage(const char *program_name) {
  fprintf(stderr, "Usage:\t%s -p port [ -f max_connextions ] [ -c config_file ] [ -v verbosity_level ][-m]\n", program_name);
  exit(EXIT_FAILURE);
}

/*********************************************************************************/
int main (int argc, char *argv[]) {

 
  bool is_daemon = false;
  int option;
  int port = -1;
  int fork_max_connections = 0;

  disable_pospro=TRUE;
  while ((option = getopt(argc, argv, "p:f:t:Bdc:v:m")) != -1) {
    switch (option) {
      case 'd':
        is_daemon = true;
        break;
      case 'v': {
        char *endptr;
        cat_api_verbosity = strtol(optarg, &endptr, 10);
        if (endptr == optarg) {
          fprintf(stderr, "ERROR: verbosity level must be an integer\n");
          usage(argv[0]);
        }
        else {
        }
        break;
      }
      case 'p': {
        char *endptr;
        port = strtol(optarg, &endptr, 10);
        if (endptr == optarg) {
          fprintf(stderr, "ERROR: port must be an integer\n");
          usage(argv[0]);
        }
        break;
      }
      case 'f': {
        char *endptr;
        fork_max_connections = strtol(optarg, &endptr, 10);
        if (endptr == optarg) {
          fprintf(stderr, "ERROR: the maximum number of forks must be an integer\n");
          usage(argv[0]);
        }
        break;
      }
      case 'm': {
        may=0;
        break;
      }
      case 'c': {
        config_fn = optarg;
        break;
      }
    }
  }

  if (port < 0) {
    usage(argv[0]);
  }

  cat_properties_t properties = {"Cat svn" REVISION, CAT_HTR_TRANSCRIPTION, CAT_WORDGRAPH_FILENAME, false, true};
  cat_server_api_t api = {set_source, provide_completions, set_prefix, set_prefix_online_strokes, NULL, NULL};

  cat_server_t *cat_server = cat_server_create(port, &properties, &api);
  if (cat_server == NULL) return EXIT_FAILURE;

  cat_server_set_encoding(cat_server, "ISO-8859-1");

  if (fork_max_connections > 0) {
    cat_server_set_multiuser_mode(cat_server, MU_FORK, fork_max_connections);
  }


  if (is_daemon) daemonize(NULL, NULL);

  fprintf(stderr, "Starting server '%s' ...\n", properties.name);
  cat_server_run(cat_server);


  cat_server_delete(cat_server);  
  ec_state_t *set_ec_state = get_ec_state();
  destroy_ec_state(&set_ec_state);
  destroy_nbestsuffix();
  destroy_word_graph(); 



  return EXIT_SUCCESS;
}




