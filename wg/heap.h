/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Heap library implemetation */
#ifndef HEAP_INCLUDED
#define HEAP_INCLUDED

/*****************************************************************************/
/************************ TYPE DEFINITION ************************************/
/*****************************************************************************/

/** \d Data structure for heap element */
typedef struct {
  float   score;/**< \d Score of the heap element */
  void    *data;/**< \d Pointer to data of the heap element */
} heape_t;

/** \d Heap data structure */ 
typedef struct {
  int size;     /**< \d Used length */
  int capacity; /**< \d Real length */
  heape_t **v;  /**< \d Vector containing elements */
  int (*compare) (float a, float b); /**< \d Pointer to function that will be used to compare two elements */
} heap_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Function compare for min heap */
int minheap(float a, float b);

/** \d Function compare for max heap */
int maxheap(float a, float b);

/** \d Create and initilize num_heap heaps */
heap_t *create_heap(int num_heap, int (*compare)(float a, float b));

/** \d Destroy num_heap heaps*/
void destroy_heap(heap_t *h, int num_heaps, int is_pointer);

/** Conditional destroy heap from the element in position init. The data element 
    is supposed to be a pointer otherwise is meaningless to use this function */
void cond_destroy_heap(heap_t *h, int num_heap, unsigned char *init);

/** \d Insert an element into the heap */
void push_heap(heap_t *h, float score, void *data);

/** \d Extract element from the top of the heap */
void *pop_heap(heap_t *h);

/** \d Obtain element from the top of the heap */
void *top_heap(heap_t *h);

/** \d Check if heap is empty */
int isempty_heap(heap_t *h);

/** \d Return heap size */
int size_heap(heap_t *h);

/** \d Increase the key of a i-element in the heap and recover heap property */
void prioritize_key_heap(heap_t *h, float key, int i);

/** \d Insert an element into the heap taking into account that data element 
already exists and we are going to replace the key value */
void modify_key_heap(heap_t *h, float key, heape_t *he);

/** \d Delete element h->v[i] from heap h */
heape_t *delete_from_heap(heap_t *h, int i);

/** \d Obtain i-element from heap H */
heape_t *obtain_i_heap(heap_t *h, int i);

/** \d Create and initilize heap */
void copy_heap(heap_t *sh, heap_t *th);

#endif
