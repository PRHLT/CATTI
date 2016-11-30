/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library that implements a dictionary to handle large vocabularies */
#ifndef DICTIONARY_INCLUDED
#define DICTIONARY_INCLUDED
#include "hash.h"
#include "vector.h"

/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

/** \d Data structure representing a dictionary */
typedef struct{
  hash_t *str2int;   /**< \d Hash table mapping tokens to ids */
  char   **int2str;  /**< \d Array mapping ids to tokens */
  double *idprob;    /**< \d Array that contain the probability of each word */ 
  char   **sint2str; /**< \d Sorted int2str vector on demand for binary search */
  int    size;       /**< \d Size of int2str array */
  int    ssize;      /**< \d Size of sint2str array, size of the dictionary when was sorted */
  int    capacity;   /**< \d Actual physical size of int2str array */
} dict_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Create dictionary with a hash table of hash_size */
dict_t *create_dict(int hash_size);

/** \d Destroy dictionary */
void destroy_dict(dict_t *dict);

/** \d Insert token into the dictionary, if already inserted returns id */
int set_token_dict(dict_t *dict, char *token);

/** \d Get id associated to token */
int get_id_dict(dict_t *dict, char *token);

/** \d Delete token from dictionary */
void del_token_dict(dict_t *dict, char *token);

/** \d Get token associated to id */
char *get_token_dict(dict_t *dict, int id);

/** \d Write out dictionary */
void write_dict(dict_t *dict);

/** \d Size of dictionary */
int size_dict(dict_t *dict);

/** \d Search token in sorted dictionary */
int search_in_dict(dict_t *dict, char *token);

/** \d Search token in sorted dictionary different to those contained in first_word_last_suffix*/
int search_in_dict_with_restrictions(dict_t *dict, char *token,vector_t *first_word_last_suffix);

/** \d sint2str vector is updated, that is, it is sorted */
void sort_dict(dict_t *dict);

/** \d Tokenize a string and convert tokens into its corresponding dict id */
int *str2seq(dict_t *dict, char *str);

/** \d Set prob associated to id */
int set_prob_dict(dict_t *dict, int id, double prob);
  
/** \d Get prob associated to id */  
double get_prob_dict(dict_t *dict, int id);

#endif
