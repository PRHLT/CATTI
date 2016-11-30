/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library that implements an ordered list to explore elements according to an order */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
 
#include "auxiliar.h"
#include "dynamic_list.h"
#include "dynamic_ordered_list.h" 

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/***************************************************************************/
/** \d Create a generic ordered list  */
ordered_list_t *create_ordered_list(int *order, int order_size){

  ordered_list_t *ordered_L;
  int *orderedc=NULL;/* Count how many elements of a given order exists */
  int i;

  mt(ordered_L = (ordered_list_t *)  malloc(sizeof(ordered_list_t)));
  ordered_L->L=NULL;
  ordered_L->max_buckets =-1;

  for(i=0;i<order_size;i++){
    if(order[i]>ordered_L->max_buckets){
      mt(orderedc = (int *) realloc(orderedc,(order[i]+1)*sizeof(int)));
      memset(orderedc+ordered_L->max_buckets+1, 0, (order[i]-ordered_L->max_buckets)*sizeof(int));
      mt(ordered_L->L = (orderedv_t *) realloc(ordered_L->L,(order[i]+1)*sizeof(orderedv_t)));
      ordered_L->max_buckets = order[i];
    }
    orderedc[order[i]]++;
  }

  for(i=0;i<=ordered_L->max_buckets;i++){
    mt(ordered_L->L[i].orderedv = (int *) malloc(orderedc[i]*sizeof(int)));
    ordered_L->L[i].l=0;
  }

  ordered_L->order = order;
  ordered_L->order_size = order_size;

  free(orderedc);

  return ordered_L;
}

/***************************************************************************/
/** \d Create a ordered list whose elements are explored forward according to order */
ordered_list_t *create_forward_ordered_list(int *order, int order_size){

  ordered_list_t *ordered_forward_L;

  ordered_forward_L = create_ordered_list(order,order_size);

  /* For forward list, we start from the first position, minimum order */
  ordered_forward_L->bucket_index = 0;

  return ordered_forward_L;

}

/***************************************************************************/
/** \d Create an ordered list whose elements are explored backward according to order */
ordered_list_t *create_backward_ordered_list(int *order, int order_size){

  ordered_list_t *ordered_backward_L;

  ordered_backward_L = create_ordered_list(order,order_size);

  /* For forward list, we start from last position, maximum order  */
  ordered_backward_L->bucket_index = ordered_backward_L->max_buckets;

  return ordered_backward_L;

}

/***************************************************************************/
/** \d Insert an element into an ordered list */
void insert_into_ordered_list(ordered_list_t *ordered_L, int index2order, void *data){

int bucket_index, index_in_bucket;

bucket_index=ordered_L->order[index2order];

index_in_bucket=ordered_L->L[bucket_index].l++;

ordered_L->L[bucket_index].orderedv[index_in_bucket] = (int) data;

}

/***************************************************************************/
/** \d Extract one item from the ordered list moving forward */
void *extract_from_forward_ordered_list(ordered_list_t *ordered_L){

  int index_in_bucket;

  while( (ordered_L->bucket_index<=ordered_L->max_buckets) && (!ordered_L->L[ordered_L->bucket_index].l) )
    ordered_L->bucket_index++;

   /* All buckets are empty */
  if(ordered_L->bucket_index == (ordered_L->max_buckets +1))
    return NULL;
  else{
    index_in_bucket=--ordered_L->L[ordered_L->bucket_index].l;
    return (void *) ordered_L->L[ordered_L->bucket_index].orderedv[index_in_bucket];
  } 
}

/***************************************************************************/
/** \d Extract one item from the ordered list moving backward */
void *extract_from_backward_ordered_list(ordered_list_t *ordered_L){

  int index_in_bucket;

  while( (ordered_L->bucket_index>=0) && (!ordered_L->L[ordered_L->bucket_index].l) )
    ordered_L->bucket_index--;

   /* All buckets are empty */
  if(ordered_L->bucket_index == -1)
    return NULL;
  else{
    index_in_bucket=--ordered_L->L[ordered_L->bucket_index].l;
    return (void *) ordered_L->L[ordered_L->bucket_index].orderedv[index_in_bucket];
  }

}

/***************************************************************************/
/** \d Check that ordered list is empty checking that every bucket is empty */
int empty_forward_ordered_list(ordered_list_t *ordered_L){

  while( (ordered_L->bucket_index<=ordered_L->max_buckets) && (!ordered_L->L[ordered_L->bucket_index].l) )
    ordered_L->bucket_index++;

   /* All buckets are empty */
  if(ordered_L->bucket_index == (ordered_L->max_buckets + 1))
    return (int) !NULL;
  else
    return (int) NULL;

}

/***************************************************************************/
/** \d Check that ordered list is empty checking that every bucket is empty */
int empty_backward_ordered_list(ordered_list_t *ordered_L){

  while( (ordered_L->bucket_index>=0) && (!ordered_L->L[ordered_L->bucket_index].l) )
    ordered_L->bucket_index--;

   /* All buckets are empty */
  if(ordered_L->bucket_index == -1)
    return (int) !NULL;
  else
    return (int) NULL;

}

/***************************************************************************/
/** \d Destroy ordered list freeing every bucket */
void destroy_ordered_list(ordered_list_t *ordered_L){

  int bucket_index;

  for(bucket_index=0;bucket_index<=ordered_L->max_buckets;bucket_index++){
    free(ordered_L->L[bucket_index].orderedv);
  }
  free(ordered_L->L);
  free(ordered_L);

}

/***************************************************************************/
/** \d Convert an ordered list into a conventional linked list */
list_t *create_list_from_ordered_list(ordered_list_t *ordered_L){

  list_t *L;
  int bucket_index,i;

  L = create_list(1);

  for(bucket_index=ordered_L->bucket_index;bucket_index < (ordered_L->max_buckets +1); bucket_index++)
    for(i=0;i<ordered_L->L[bucket_index].l;i++)
      insert_into_list(L, (void *) ordered_L->L[bucket_index].orderedv[i]);
      
  return L;

}

/***************************************************************************/
/** \d Creates a copy of the ordered list */
ordered_list_t *duplicate_ordered_list(ordered_list_t *ordered_L, int *order, int order_size){

  ordered_list_t *copy_ordered_L;
  int bucket_index;


  copy_ordered_L = create_ordered_list(order, order_size);

  for(bucket_index=ordered_L->bucket_index;bucket_index < (ordered_L->max_buckets +1); bucket_index++){
    memcpy(copy_ordered_L->L[bucket_index].orderedv, ordered_L->L[bucket_index].orderedv,ordered_L->L[bucket_index].l*sizeof(int));
    copy_ordered_L->L[bucket_index].l = ordered_L->L[bucket_index].l;
  }  

  copy_ordered_L->bucket_index = ordered_L->bucket_index;

  return copy_ordered_L;

}
