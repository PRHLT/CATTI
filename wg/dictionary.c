/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library that implements a dictionary to handle large vocabularies */
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <limits.h>

#include "global.h"
#include "auxiliar.h"
#include "hash.h"
#include "strop.h"
#include "dictionary.h"
#include "vector.h"

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

/* Id for token not found in dictionary */
#define TOKEN_NOT_FOUND_IN_DICT -2

/* DEFAULT INT2STR TABLE CAPACITY */
#define DF_INT2STR_TABLE_CAPACITY 1024

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/******************************************************************************/
/** \d Create dictionary with a hash table of hash_size */
dict_t *create_dict(int hash_size){

  dict_t *dict;

  mt(dict = (dict_t *) malloc(sizeof(dict_t)));

  dict->size=dict->ssize=0;
  dict->capacity=DF_INT2STR_TABLE_CAPACITY;
  mt(dict->int2str=(char **) malloc(sizeof(char *)*(dict->capacity)));
  mt(dict->idprob=(double *) malloc(sizeof(double)*(dict->capacity)));
  /* Create hash table that map tokens into codes */
  dict->str2int=create_hash(hash_size);
  dict->sint2str=NULL;
   

  return(dict);
}

/*****************************************************************************/
/** \d Destroy dictionary */
void destroy_dict(dict_t *dict){

  int id;

  if(dict){
    /* Free int to string array */
    for(id=0;id<dict->size;id++) free(dict->int2str[id]);
    free(dict->int2str);
    if(dict->sint2str) free(dict->sint2str);
    free(dict->idprob);   
    /* Free hash table */
    destroy_hash(dict->str2int,0);

    free(dict);
    dict=NULL;
  }
}

/*****************************************************************************/
/** \d Insert token into the dictionary, if already inserted returns id */
int set_token_dict(dict_t *dict, char *token){

  int id;

  if ((id= (int) get_hash(token,strlen(token),dict->str2int))==HASH_KEY_NOT_FOUND){
    /* The token is not found, then insert it */
    id= (int) set_hash(token,strlen(token),dict->str2int,(void *) dict->size);
    /* Resize int2str array, duplicating capacity */
    if(dict->size==dict->capacity){
      dict->capacity*=2;
      mt(dict->int2str=(char **) realloc(dict->int2str,sizeof(char *)*(dict->capacity)));
      mt(dict->idprob=(double *) realloc(dict->idprob,sizeof(double)*(dict->capacity)));
    }
    /* Add new token at the end */
    dict->int2str[dict->size]=strdup2(token);
    dict->size++;
  }

  return id;
}

/*****************************************************************************/
/** \d Get id associated to token */
int get_id_dict(dict_t *dict, char *token){

  int id;

  if(dict!=NULL){
    id= (int) get_hash(token,strlen(token),dict->str2int);

    if(id!=HASH_KEY_NOT_FOUND) return id;
    else                       return(TOKEN_NOT_FOUND_IN_DICT);
    }
  else return(TOKEN_NOT_FOUND_IN_DICT);
  
}

/*****************************************************************************/
/** \d Delete token from dictionary */
void del_token_dict(dict_t *dict, char *token){

  int id;

  id= (int) get_hash(token,strlen(token),dict->str2int);
  del_hash(dict->str2int,token,strlen(token));
  free(dict->int2str[id]);dict->int2str[id]=NULL;
}

/*****************************************************************************/
/** \d Get token associated to id */
char *get_token_dict(dict_t *dict, int id){

  if ((id >= 0)&&(id < dict->size)) 
    return(dict->int2str[id]);
  else 
    return(NULL);

}

/*****************************************************************************/
/** \d Set prob associated to id */
int set_prob_dict(dict_t *dict, int id, double prob){
  if((id >= 0)&&(id < dict->size)){ 
    dict->idprob[id]=prob;
    return id;
  }
  else 
    return(INT_MIN);
       
}

/*****************************************************************************/
/** \d Get prob associated to id */
double get_prob_dict(dict_t *dict, int id){
  if ((id >= 0)&&(id < dict->size)) 
    return(dict->idprob[id]);
  else 
    return(1);
  
}


/*****************************************************************************/
/** \d Write out dictionary */
void write_dict(dict_t *dict){

  int id;

  fprintf(stderr,"Dictionary:\n    Id Token\n------ -----------\n");
  for(id=0;id<dict->size;id++) 
    if(dict->int2str[id]) 
      fprintf(stderr,"%6d %s\n",id,dict->int2str[id]);

}

/*****************************************************************************/
/** \d Size of dictionary */
int size_dict(dict_t *dict){
  return(dict->size);
}


/*****************************************************************************/
/** \d Search token in sorted dictionary */
int search_in_dict(dict_t *dict, char *token){

 int token_in_dict;

  /* if dictionary is not sorted, do it now */
  if (!dict->sint2str) sort_dict(dict);

  /* Binary search in sorted token graph vocabulary */
  token_in_dict = strbs(dict->sint2str,dict->ssize, token);

  /* If there is no token in the dictionary whose */
  if(token_in_dict == STRING_NOT_FOUND)
    return NO_WORD;
  else 
    return get_id_dict(dict,dict->sint2str[token_in_dict]);
}

/*****************************************************************************/
/** \d Search token in sorted dictionary different to those in first_word_last_suffix*/
int search_in_dict_with_restrictions(dict_t *dict, char *token,vector_t *first_word_last_suffix){


 int token_in_dict,i;
 char **rejected_words=NULL;

  /* if dictionary is not sorted, do it now */
  //if (!dict->sint2str) sort_dict(dict);
   
  
  if(first_word_last_suffix->size>0)  mt(rejected_words=(char **) malloc(sizeof(char *)*(first_word_last_suffix->size)));
  for(i=0;i < first_word_last_suffix->size; i++){
     rejected_words[i]=get_token_dict(dict,(int) first_word_last_suffix->element[i]);
  } 
    /* Binary search in token graph vocabulary */
  token_in_dict = strswr(dict->int2str,dict->size, token,rejected_words,first_word_last_suffix->size);

   /* If there is no token in the dictionary whose */
  if(token_in_dict == STRING_NOT_FOUND)
    return NO_WORD;
  else 
    return get_id_dict(dict,dict->int2str[token_in_dict]);
}


/*****************************************************************************/
/** \d sint2str vector is updated, that is, it is sorted */
void sort_dict(dict_t *dict){

  int i;
  mt(dict->sint2str = (char **) realloc(dict->sint2str,(dict->size)*sizeof(char *)));
  for(i=0;i<dict->size;i++) dict->sint2str[i] = dict->int2str[i];
  dict->ssize=dict->size;

  strqsort(dict->sint2str, 0, dict->size-1);

}

/*****************************************************************************/
/** \d Tokenize a string and convert tokens into its corresponding dict id */
int *str2seq(dict_t *dict, char *str){

char *str_cp,**str_tok;
int *seq=NULL,seql=0,i;

/* Initialisation */
str_cp=strdup2(str);

/* Tokenize and map token into vocab id */
str_tok=tokenize(str_cp);
for(i=0;str_tok[i]!=NULL;i++){
  mt(seq=(int *)realloc(seq,sizeof(int)*(seql+1)));
  seq[seql++]=set_token_dict(dict,str_tok[i]);
}
free(str_cp);
free(str_tok);
/* Add end symbol */
mt(seq=(int *)realloc(seq,sizeof(int)*(seql+1)));
seq[seql++]=NO_WORD;

return seq;
}

