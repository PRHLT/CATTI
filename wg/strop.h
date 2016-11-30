/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d This file contains string operations */
#ifndef STROP_INCLUDED
#define STROP_INCLUDED

#include <unistd.h>
/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define STRING_NOT_FOUND -1

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Number of words of a string */
int nwords(char* string);

/** \d Read a line overwriting the newline character */
ssize_t readline(FILE *fp, char **line);

/** \d Input in iso-8859 is converted into UTF-8 */
void convierteUTF(char *entrada, char *salida);

/** \d Tokenize a string without modifying it */
char **tokenize(char *string);

/** \d Computes the minimum of three values*/
int minimum(int a,int b,int c);

/** \d Implementation of word Levenshtein distance */
int word_leven_dist(char *s, char *t);

/** \d Implementation of char Levenshtein distance */
int char_leven_dist(char *s, char *t);

/** \d Quicksort to sort an array of string thanks to Alfons :-) */
void strqsort(char **v, int i, int j);

/** \d Binary search to find a string s in an array of strings v of length l */
int strbs(char **v, int len, char *s);

/** \d Search to find a string s (different of those contained in p) in an array of strings v of length l*/
int strswr(char **v, int len, char *s,char **p,int lenp);

/** \d Remove multiple consecutive white spaces */
char *remove_multiple_spaces(char *in_string);




#endif
