/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library that implements basic vector operations */
#ifndef VECTOR_INCLUDED
#define VECTOR_INCLUDED

/*****************************************************************************/
/************************ TYPE DEFINITION ************************************/
/*****************************************************************************/

/** \d Data structure that represents a vector of variable length */
typedef struct vectorCATTI{
  void **element; /**< \d The vector */
  int size;       /**< \d The size   */
} vector_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Create and initilize vector */
vector_t *create_vector(int number_of_vectors);

/** \d Destroy vector */
void destroy_vector(vector_t *V, int number_of_vectors, int is_pointer);

/** \d Return 1 if two vectors are equal, 0 otherwise */
int equal_vector(vector_t *v1, vector_t *v2);

/** \d Copy vector v1 on v2 */
void copy_vector(vector_t *v1, vector_t *v2);

#endif
