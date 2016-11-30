/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Heap library implemetation */
#include <stdio.h>
#include <stdlib.h>

#include "auxiliar.h"
#include "heap.h"

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

/* DEFAULT HEAP CAPACITY */
#define DF_HEAP_CAPACITY 8

/* Heap accessors */
#define PARENT(i) (i-1)/2
#define LEFT(i)   2*i+1
#define RIGHT(i)  2*i+2
#define ROOT_HEAP 0

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/******************************************************************************/
/** \d Function compare for min heap */
int minheap(float a, float b){return a>b;}

/******************************************************************************/
/** \d Function compare for max heap */
int maxheap(float a, float b){return a<b;}

/******************************************************************************/
/** \d Create and initilize num_heap heaps */
heap_t *create_heap(int num_heap, int (*compare)(float a, float b)){
  heap_t *h;
  int i;

  mt(h=(heap_t *)malloc(sizeof(heap_t)*num_heap));
  for(i=0;i<num_heap;i++){
    h[i].capacity=DF_HEAP_CAPACITY;
    h[i].size=0;
    h[i].compare=compare;
    mt(h[i].v=(heape_t **)malloc(h->capacity*sizeof(heape_t *)));
  }
  return h;
}

/*****************************************************************************/
/** \d Destroy num_heap heaps*/
void destroy_heap(heap_t *h, int num_heap, int is_pointer){

  int i,j;

  if(h){
    for(i=0;i<num_heap;i++){
      for(j=0;j<h[i].size;j++){
        if(h[i].v[j]->data&&is_pointer)
          free(h[i].v[j]->data);
        free(h[i].v[j]);
      }
      free(h[i].v);
    }
    free(h);
  }

}

/***************************************************************************/
/** \d Conditional destroy heap from the element in position init. The data element 
    is supposed to be a pointer otherwise is meaningless to use this function */
void cond_destroy_heap(heap_t *h, int num_heap, unsigned char *init){

  int i,j;

  if(h!=NULL){
    for(i=0;i<num_heap;i++){
      for(j=init[i];j<h[i].size;j++){
        free(h[i].v[j]->data);
        free(h[i].v[j]);
      }
      if((init[i])&&(h[i].size>0))
        free(h[i].v[ROOT_HEAP]);
      free(h[i].v);
    }
    free(h);
  }
}

/*****************************************************************************/
/** \d Recover the property of heap */
void heapfy(heap_t *h, int i){
  int l, r, aux=i;
  heape_t *he;

  l=LEFT(i);r=RIGHT(i);
  /* Find maximum among elements of this subtree */
  if (l<h->size){
    if (h->compare(h->v[l]->score,h->v[i]->score)) aux=i;
    else                                           aux=l;
  }
  if (r<h->size && h->compare(h->v[aux]->score,h->v[r]->score)) aux=r;
  /* If heap condition was violated replace root of this subtree
  and call heapfy recursively */
  if (aux!=i){
    he=h->v[i];h->v[i]=h->v[aux];h->v[aux]=he;
    heapfy(h,aux);
  }
}

/*****************************************************************************/
/** \d Insert an element into the heap */
void push_heap(heap_t *h, float score, void *data){
  int i;
  heape_t *he;

  /* Resize heap vector if necessary */
  if(h->size==h->capacity){h->capacity*=2;
    mt(h->v=(heape_t **)realloc(h->v,h->capacity*sizeof(heape_t *)));
  }
  /* Create a new heap element and fill it */
  mt(he=(heape_t *)malloc(sizeof(heape_t)));
  he->data=data;he->score=score;

  /* Initial position of inserted element */
  i = h->size; h->size++;

  /* Find position for new element */
  while (i>ROOT_HEAP && h->compare(h->v[PARENT(i)]->score,he->score)){
    h->v[i] = h->v[PARENT(i)];
    i=PARENT(i);
  }
  /* Assign heap element to its final position */
  h->v[i]=he;
}

/*****************************************************************************/
/** \d Extract element from the top of the heap */
void *pop_heap(heap_t *h){
  heape_t *he;
  void *data;

  if (h->size<1) return NULL;

  /* Save max element and replace it by last element */
  he=h->v[ROOT_HEAP]; h->v[ROOT_HEAP]=h->v[h->size-1];

  if((--h->size)*2==h->capacity){ 
    h->capacity/=2;
    mt(h->v=(heape_t **)realloc(h->v,h->capacity*sizeof(heape_t *)));
  }

  /* Recover heap property */
  heapfy(h,ROOT_HEAP);

  /* Return extracted element */
  data=he->data; free(he);
  return data;
}

/*****************************************************************************/
/** \d Obtain element from the top of the heap */
void *top_heap(heap_t *h){return h->size>0?h->v[ROOT_HEAP]->data:NULL;}

/*****************************************************************************/
/** \d Check if heap is empty */
int isempty_heap(heap_t *h){return !h->size;}

/*****************************************************************************/
/** \d Return heap size */
int size_heap(heap_t *h){return h->size;}

/*****************************************************************************/
/** \d Increase the priority of a i-element moving it up in the heap */
void prioritize_key_heap(heap_t *h, float key, int i){

  heape_t *aux;

  h->v[i]->score=key;
  
  while ( (i > 0) && h->compare(h->v[PARENT(i)]->score,h->v[i]->score)){
    /* Exchange parent and child */
    aux=h->v[i]; h->v[i]=h->v[PARENT(i)]; h->v[PARENT(i)]=aux;
    i = PARENT(i);
  }
}

/*****************************************************************************/
/** \d Insert an element into the heap taking into account that data element 
already exists and we are going to prioritize the key value */
void modify_key_heap(heap_t *h, float key, heape_t *he){

  int i;

  /* Let's check if this data element is already in the heap */
  for(i=ROOT_HEAP;i<h->size;i++){
    if(h->v[i] == he){
      prioritize_key_heap(h, key, i);
      return;
    }
  }
}

/*****************************************************************************/
/** \d Delete element h->v[i] from heap h */
heape_t *delete_from_heap(heap_t *h, int i){

  heape_t *aux;

  /* Exchange h->v[i] with last element in the heap */
  aux=h->v[i]; h->v[i]=h->v[h->size-1]; h->v[h->size-1]=aux;

  if((--h->size)*2==h->capacity){ 
    h->capacity/=2;
    mt(h->v=(heape_t **)realloc(h->v,h->capacity*sizeof(heape_t *)));
  }

  /* Recover heap property if heap_size > 1*/
  if(h->size > 1) heapfy(h,i);

  /* Return destroy element */
  return aux;  
}

/*****************************************************************************/
/** \d Obtain i-element from heap H */
heape_t *obtain_i_heap(heap_t *h, int i){
  return (i<h->size)?h->v[i]:NULL;
}

/*****************************************************************************/
/** \d Create and initilize heap */
void copy_heap(heap_t *sh, heap_t *th){
  th->v=sh->v;
  th->size=sh->size;
  th->capacity=sh->capacity;
  th->compare=sh->compare;
}
