/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library that implements an ordered list to explore elements according to an order */
#ifndef DYNAMIC_ORDERED_LIST_INCLUDED
#define DYNAMIC_ORDERED_LIST_INCLUDED

/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

/** \d Array of elements for a given bucket */
typedef struct ordered_vector{
  int l;         /**< \d Size of array */
  int *orderedv; /**< \d Array of elements inserted in the ordered list */
} orderedv_t;

/** \d Data structure representing a ordered list where elements 
    are explored according to order */
typedef struct ordered_list{
  orderedv_t *L; /**< \d The list is represented as an array of max_buckets arrays.
  The array in position i has as many elements as elements with order i 
  This list is explored in only one way, there is no movement backward */ 
  int max_buckets;  /**< \d Size of the array representing the list */
  int bucket_index; /**< \d Index pointing to the current bucket */
  int *order;       /**< \d Array containing the order of each element */
  int order_size;   /**< \d Size of array order */
} ordered_list_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Create a ordered list whose elements are explored forward according to order */
ordered_list_t *create_forward_ordered_list(int *order, int order_size);

/** \d Create an ordered list whose elements are explored backward according to order */
ordered_list_t *create_backward_ordered_list(int *order, int order_size);

/** \d Insert an element into an ordered list */
void insert_into_ordered_list(ordered_list_t *L, int index2order, void *data);

/** \d Extract one item from the ordered list moving forward */
void *extract_from_forward_ordered_list(ordered_list_t *ordered_L);

/** \d Extract one item from the ordered list moving backward */
void *extract_from_backward_ordered_list(ordered_list_t *ordered_L);

/** \d Check that ordered list is empty checking that every bucket is empty */
int empty_forward_ordered_list(ordered_list_t *ordered_L);

/** \d Check that ordered list is empty checking that every bucket is empty */
int empty_backward_ordered_list(ordered_list_t *ordered_L);

/** \d Destroy ordered list freeing every bucket */
void destroy_ordered_list(ordered_list_t *ordered_L);

/** \d Convert an ordered list into a conventional linked list */
list_t *create_list_from_ordered_list(ordered_list_t *ordered_L);

/** \d Creates a copy of the ordered list */
ordered_list_t *duplicate_ordered_list(ordered_list_t *ordered_L, int *order, int order_size);

#endif
