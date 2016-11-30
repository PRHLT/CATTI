/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Basic useful functions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "auxiliar.h"
#include <ctype.h>
#include "global.h"
#include <math.h>

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d suma logaritmica  */

double add_log(double a, double b) {
 /* Make a the max value and b the min value */
 if (b > a) { double tmp = a; a = b; b = tmp; }
 /* If min is zero, return max */
 if (b == log(0.0)) { return a; }
 /* Robust sum */
 double resta=b-a;
 double expo=exp(resta);
 double sum1=1.0+expo;
 double logarit=log(sum1);
 double sum2=a+logarit;
 return sum2;
 //return a + log(1.0 + exp(b-a));
};

/******************************************************************************/
/** \d Check if memory was allocated to pointer ptr */
void mt(void *ptr){
  if (ptr==NULL) {
    fprintf(stderr,"%s:%d: insufficient memory\n",__FILE__,__LINE__);
    exit(EXIT_FAILURE);
  }
}

/******************************************************************************/
/** \d File test: abort if p is NULL */
void ft(void *p, char *fn){
  if (p==NULL) {
    fprintf(stderr,"ft: couldn't find %s\n",fn);
    exit(EXIT_FAILURE);
  }
}

/******************************************************************************/
/** \d Return elapsed time between t1 and t2, being t2 > t1 */
float elapsed_time(timeval_t *t1, timeval_t *t2){

  long sec;
  long usec;

  sec = t2->tv_sec - t1->tv_sec;
  if (t1->tv_usec < t2->tv_usec)
    usec = t2->tv_usec - t1->tv_usec;
  else{
    usec = 1000000 - t1->tv_usec + t2->tv_usec;
    sec--;
  }

  return (float) sec + ((float) usec / 1000000);
}



//#ifdef WIN

char* strdup2(const char* string){

char* string_copy=NULL;

if(string!=NULL){
  mt(string_copy=(char *)malloc(sizeof(char)*(strlen(string)+1)));
  memcpy(string_copy,string, sizeof(char)*strlen(string)+1);
}

return string_copy;
}


char* strndup2(const char* string, int length){

char* string_copy=NULL;

if(string!=NULL){
  mt(string_copy=(char *)malloc(sizeof(char)*(length+1)));
  memcpy(string_copy,string, sizeof(char)*(length));
  if(strlen(string)<length) string_copy[length-1]=' ';
  string_copy[length]='\0';
}

return string_copy;

}

/******************************************************************************/
/** \d Return elapsed time between t1 and t2, being t2 > t1 */

char* strupr(char* s){
  char *sp = s;
  while (*sp) {
   *sp = toupper(*sp);
   sp++;
  }
  return s;
}
//#endif

char* strlow(char* s){
  char *sp = s;
  while (*sp) {
   *sp = tolower(*sp);
   sp++;
  }
  return s;
}