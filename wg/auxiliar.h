/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Basic useful functions */
#ifndef AUXILIAR_INCLUDED
#define AUXILIAR_INCLUDED
#include "vector.h"
/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

typedef struct timeval timeval_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Check if memory was allocated to pointer ptr */
void mt(void *ptr);

/** \d Check if file was opened correctly */
void ft(void *p, char *fn);

/** \d Return elapsed time between t1 and t2 being t2 > t1 */
float elapsed_time(timeval_t *t1, timeval_t *t2);


char* strdup2(const char* string);

char* strndup2(const char* string, int length);

char* strupr(char* s);

char* strlow(char* s);

double add_log(double a, double b);
  
#endif


