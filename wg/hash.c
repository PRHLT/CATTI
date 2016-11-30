/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library implementing a hash data structure and its operations */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "auxiliar.h"
#include "hash.h"

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define INITVAL 23312819
#define HASHSIZE(n) ((ub4)1<<(n))
#define HASHMASK(n) (HASHSIZE(n)-1)

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Hash function parameters: key, length of the key, the previous hash or an arbitrary value */
ub4 hash_function( k, length, initval)
register ub1 *k;        /* the key */
register ub4  length;   /* the length of the key */
register ub4  initval;  /* the previous hash, or an arbitrary value */
{
   register ub4 a,b,c,len;
   
   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */ 
   c = initval;         /* the previous hash value */
   
   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
      b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
      c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }
  
   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
   case 11: c+=((ub4)k[10]<<24);
   case 10: c+=((ub4)k[9]<<16);
   case 9 : c+=((ub4)k[8]<<8);
      /* the first byte of c is reserved for the length */
   case 8 : b+=((ub4)k[7]<<24);
   case 7 : b+=((ub4)k[6]<<16);
   case 6 : b+=((ub4)k[5]<<8);
   case 5 : b+=k[4];
   case 4 : a+=((ub4)k[3]<<24);
   case 3 : a+=((ub4)k[2]<<16);
   case 2 : a+=((ub4)k[1]<<8);
   case 1 : a+=k[0];
     /* case 0: nothing left to add */
   }
   mix(a,b,c);   
   /*-------------------------------------------- report the result */
   return c;
}

/*****************************************************************************/
/** \d Return hash index from key of length key_length */
int hash_key(void *key, int key_length, int ubits){
  return hash_function((ub1 *)key,key_length,INITVAL) & HASHMASK(ubits);
}

/*****************************************************************************/
/** \d Create hash table of size table_size */
hash_t *create_hash(int table_size){

  hash_t *hs;
  int i;

  mt(hs = (hash_t *)malloc(sizeof(hash_t)));

  /* Compute table size power of 2 */
  hs->table_size = table_size;
  for(i=0;table_size!=0;i++) table_size=table_size>>1; i--;
  table_size = ((ub4)1<<(i));
  hs->useful_bits = i;

  /* Initialize hash list for each entry in the hash table */
  mt(hs->hash_table = (hlist_t **) malloc(sizeof(hlist_t *)*table_size));
  for(i=0;i<table_size;i++) hs->hash_table[i] = NULL;

  return hs;
}

/*****************************************************************************/
/** \d Return the pointer to the data with given key */
void *get_hash(void *key, int key_length, hash_t *hs){

  hlist_t *aux;
  int hash_index;

  /* Every hash list has been already initialized during its creation */
  hash_index = hash_key(key,key_length,hs->useful_bits);

  /* Searching the hash element that matches key in the collision list */
  for(aux=hs->hash_table[hash_index];aux!=NULL;aux=aux->next)
    if((aux->key_length==key_length)&&(!memcmp(aux->key,key,key_length)))
      return aux->data;

  /* If we arrive here the key we were searching was not found in the hash table */
  return (void *) HASH_KEY_NOT_FOUND;

}

/*****************************************************************************/
/** \d Insert data and return the pointer to it */
void *set_hash(void *key, int key_length, hash_t *hs, void *data){

  hlist_t *p,*pp;
  int hash_index;
 
  /* Every hash list has been already initialized during its creation */
  hash_index = hash_key(key,key_length,hs->useful_bits);

  /* Searching the hash element that matches key in the collision list */
  for(pp=NULL,p=hs->hash_table[hash_index];p!=NULL;pp=p,p=p->next)
    if((p->key_length==key_length)&&(!memcmp(p->key,key,key_length)))
      return p->data;

  /* Create a new hash element in the hash list */
  mt(p=(hlist_t *) malloc(sizeof(hlist_t)));

  /* Fill hash list element */
  mt(p->key = (void *) malloc(key_length));
  memcpy(p->key, key, key_length);
  p->key_length=key_length;
  p->data=data;

  /* Link new hash element in the list */
  if (pp==NULL) {p->next=NULL; hs->hash_table[hash_index]=p;} else {p->next=pp->next; pp->next=p;}

  return data;

}

/*****************************************************************************/
/** \d Remove element from hash table */
void *del_hash(hash_t *hs, char *key, int key_length){

  hlist_t *p,*pp;
  int hash_index;
  void *data;

  hash_index = hash_key(key,key_length,hs->useful_bits);

  for (pp=p=hs->hash_table[hash_index];p!=NULL;pp=p,p=p->next)
    if((p->key_length==key_length)&&(!memcmp(p->key,key,key_length))){
      data=p->data; pp->next=p->next;
      if(pp==p) hs->hash_table[hash_index]=p->next; /*one-element list*/
      free(p->key); free(p); return data;}

   return (void *) HASH_KEY_NOT_FOUND;
}

/*****************************************************************************/
/** \d Free hash table and elements that are pointers */
void destroy_hash(hash_t *hs, int is_pointer){

  hlist_t *p,*pp;
  int i;

  /* Go over the hash table a free link lists */
  for(i=0;i<hs->table_size;i++)
    for(p=hs->hash_table[i],pp=NULL;p!=NULL;pp=p,p=p->next,free(pp)){
      /* Just in case we are using this void* as an integer */
      if(p->data&&is_pointer) free(p->data);
      free(p->key);
    }
  free(hs->hash_table);
  free(hs);
}
