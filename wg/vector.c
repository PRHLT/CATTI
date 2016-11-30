/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library that implements basic vector operations */
#include <stdlib.h>
#include <string.h>

#include "auxiliar.h"
#include "vector.h" 

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Create and initilize vector */
vector_t *create_vector(int number_of_vectors){

  vector_t *V;
  int i;

  mt(V = (vector_t *) malloc(sizeof(vector_t ) * number_of_vectors));

  for(i=0;i<number_of_vectors;i++){
    V[i].size = 0;
    V[i].element = NULL;
  }

  return V;

}

/** \d Destroy vector */
void destroy_vector(vector_t *V, int number_of_vectors, int is_pointer){

  int i,j;

  if(V!=NULL){
    for(i=0;i<number_of_vectors;i++)
      if(V[i].element != NULL){
        if(is_pointer)
          for(j=0;j<V[i].size;j++)
              free(V[i].element[j]);
        free(V[i].element);
      }  
    free(V);
  }
}

/** \d Return 1 if two vectors are equal, 0 otherwise */
int equal_vector(vector_t *v1, vector_t *v2){

  int j;

      if(v1->size == v2->size){
        for(j=0;(j<v1->size)&&(v1->element[j]==v2->element[j]);j++);
          if(v1->size == j) return 1;
          else              return 0;
      }
      else return 0;
}

/** \d Copy vector v1 on v2 */
void copy_vector(vector_t *v1, vector_t *v2){

      mt(v2->element = (void **) malloc(v1->size*sizeof(void *)));
      memcpy(v2->element, v1->element, v1->size*sizeof(void *));
      v2->size = v1->size;
}

