/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d This file contains string operations */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "global.h"
#include "auxiliar.h"
#include "hash.h"
#include "dynamic_list.h"
#include "dictionary.h"
#include "strop.h"

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define DICT_HASH_TABLE_SIZE 1024

/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

/** \d integer vector structure */
typedef struct {int *v; int l;} int_v;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/*****************************************************************************/
/** \d Number of words in a string */
int nwords(char* string){
  int nw=0;
  int i=0;

  /* Ignore initial spaces */
  while(isspace(string[i])) i++;

  /* While not end of string */
  while (string[i]!='\0'){
    while((string[i]!='\0')&&(!isspace(string[i]))) i++;
    nw++;
    while(isspace(string[i])) i++;
  }

  return nw;
}


/*****************************************************************************/
/** \d Read a line overwriting the newline character */
ssize_t readline(FILE *fp, char **line){ 

  size_t n=0;
  ssize_t nc=0;

  nc=getline(line,&n,fp);
  if (nc>0){
    
    if((*line)[nc-1]=='\n') (*line)[nc-1]='\0';/* Overwrite \n character */
  }
  else *line=NULL;/* getline modifies *line even if nothing is read */

  return nc;
}

/*****************************************************************************/
/** \d Input in iso-8859 is converted into UTF-8 */
void convierteUTF(char *entrada, char *salida)
{

  int i,j;
  unsigned char aux;
  
  for(i=0,j=0;i<(int)strlen(entrada);i++,j++)
  {
    if((unsigned char)entrada[i]<192)
          salida[j]=entrada[i];
    
    else
    {
      aux=entrada[i]<<6;
      i++;
      salida[j]=aux+(entrada[i]&63);
    }
	
  }
    salida[j]=0;
}

/*****************************************************************************/
/** \d Tokenize a string modifying it */
char **tokenize(char *string){

char *delim,*tok,**tok_string;
int size;

/* Initialisation */
delim=strdup2(" \f\n\r\t\v");
tok_string=NULL;
size=0;

tok=strtok(string,delim);
while(tok){
   mt(tok_string=(char **) realloc(tok_string,sizeof(char *)*(size+1)));
   tok_string[size++]=tok;
   tok=strtok(NULL,delim);
}
mt(tok_string=(char **) realloc(tok_string,sizeof(char *)*(size+1)));
tok_string[size]=NULL;

free(delim);

return tok_string;
}

/*****************************************************************************/
/** \d Computes the minimum of three values*/
int minimum(int a,int b,int c){

  int min=a;

  if(b<min) min=b;
  if(c<min) min=c;

  return min;
}

/*****************************************************************************/
/** \d Implementation of Levenshtein distance*/
int leven_dist(int_v x, int_v y){

int **d;
int i,j,dist;

/* Initialisation */
/* Distance trellis memory initialisation */
d=(int **)malloc(sizeof(int *)*(x.l+1));
for(i=0;i<x.l+1;i++) d[i]=(int *)malloc(sizeof(int)*(y.l+1));
/* First row and first column initiliasation */
for(i=0;i<x.l+1;i++) d[i][0]=i;
for(j=0;j<y.l+1;j++) d[0][j]=j;

/* Easy-and-clean edit distance calculation */
for(j=1;j<y.l+1;j++) for(i=1;i<x.l+1;i++)
  /* Insertion x word, Substitution, Insertion y word */
  d[i][j]=minimum(d[i-1][j]+1, d[i-1][j-1]+(x.v[i-1]!=y.v[j-1]), d[i][j-1]+1);
dist=d[x.l][y.l];  

for(i=0;i<x.l+1;i++) free(d[i]); free(d); /* Free distance trellis */

return dist; /* return top-right-most trellis cell */
}


/*****************************************************************************/
/** \d Implementation of word Levenshtein distance */
int word_leven_dist(char *s, char *t){

char *cp_s,*cp_t,**tok_s,**tok_t;
int_v int_s, int_t;
int dist,i;
dict_t *dict;

/* Initialisation */
int_s.v=int_t.v=NULL;/* Initialise int vector */
int_s.l=int_t.l=0;   /* Initialise length vector */
cp_s=strdup2(s);
cp_t=strdup2(t);
dict=create_dict(DICT_HASH_TABLE_SIZE);

tok_s=tokenize(cp_s);
for(i=0;tok_s[i]!=NULL;i++){
   mt(int_s.v=(int *)realloc(int_s.v,sizeof(int)*(int_s.l+1)));
   int_s.v[int_s.l++]=set_token_dict(dict,tok_s[i]);
}
free(cp_s);

tok_t=tokenize(cp_t);
for(i=0;tok_t[i]!=NULL;i++){
   mt(int_t.v=(int *)realloc(int_t.v,sizeof(int)*(int_t.l+1)));
   int_t.v[int_t.l++]=set_token_dict(dict,tok_t[i]);
}
free(cp_t);

dist=leven_dist(int_s, int_t); /* Levenshtein distance */

free(int_s.v);free(int_t.v);
destroy_dict(dict);

return dist;
}


/*****************************************************************************/
/** \d Implementation of char Levenshtein distance */
int char_leven_dist(char *s, char *t){

int i,dist;
int_v int_s, int_t;

/* Initialisation */
int_s.v=int_t.v=NULL;/* Initialise int vector */
int_s.l=int_t.l=0;   /* Initialise length vector */

int_s.l=strlen(s);
mt(int_s.v=(int *)malloc(sizeof(int)*int_s.l));
for(i=0;i<int_s.l;i++) int_s.v[i]=s[i];

int_t.l=strlen(t);
mt(int_t.v=(int *)malloc(sizeof(int)*int_t.l));
for(i=0;i<int_t.l;i++) int_t.v[i]=t[i];

dist=leven_dist(int_s, int_t); /* Levenshtein distance */

free(int_s.v);free(int_t.v);

return dist;
}

/*****************************************************************************/
/** \d Quicksort to sort an array of string thanks to Alfons :-) */
void strqsort(char **v, int i, int j){

  char *s,*p; int l,r,c;

  if (j-i+1>=3) {
    c=(i+j)/2;
    if (strcmp(v[i],v[c])>0) {s=v[i]; v[i]=v[c]; v[c]=s;}
    if (strcmp(v[i],v[j])>0) {s=v[i]; v[i]=v[j]; v[j]=s;}
    if (strcmp(v[c],v[j])>0) {s=v[c]; v[c]=v[j]; v[j]=s;}
    p=v[c]; l=i; r=j-1; s=v[c]; v[c]=v[r]; v[r]=s;
    do {
      do l++; while (strcmp(v[l],p)<0);
      do r--; while (strcmp(v[r],p)>0);
      s=v[l]; v[l]=v[r]; v[r]=s;
    } while (l<r);
    s=v[l]; v[l]=v[r]; v[r]=s;
    r=j-1; s=v[l]; v[l]=v[r]; v[r]=s;
    strqsort(v,i,l-1); strqsort(v,l+1,j);
  }
  else if  ((j-i>0) && (strcmp(v[i],v[j])>0)) {s=v[i]; v[i]=v[j]; v[j]=s;}
}

/*****************************************************************************/
/** \d Binary search to find a string s in an array of strings v of length l */
int strbs(char **v, int len, char *s){
  int l=0,r=len-1,x;
  int sl=strlen(s);

  while (r>=l) {
    x=(l+r)/2;
    if (strncmp(s,v[x],sl)<0) r=x-1; else l=x+1;
    if (strncmp(s,v[x],sl)==0) return x;
  }
  return STRING_NOT_FOUND;
}


/*****************************************************************************/
/** \d Search to find a string s (different of those contained in p) in an array of strings v of length l*/
int strswr(char **v, int len, char *s,char **p,int lenp){
 // int l=0,r=len-1,
  int x;
  int sl=strlen(s);
  
  
  strqsort(p, 0, lenp-1);
  
  for(x=0;x<len;x++){
    if (strncmp(s,v[x],sl)==0){
          if(strbs(p,lenp,v[x])==STRING_NOT_FOUND) return x;
      }
  }

  return STRING_NOT_FOUND;
}



/*****************************************************************************/
/** \d Remove multiple consecutive white spaces */
char *remove_multiple_spaces(char *in_string){

int j,k;
int in_string_length;
char *out_string;

        j=0;
        while(isspace(in_string[j])) j++;
        out_string = strdup2(in_string + j);
        in_string_length = strlen(out_string);
        k=0;j=0;
        while(j<in_string_length){
          while((!isspace(out_string[j]))&&(j<in_string_length)){
            out_string[k] = out_string[j];
            k++;
            j++;
          }
          if(out_string[j] == '\0'){
            out_string[k] = out_string[j];
          }
          else{
            out_string[k] = out_string[j];
            j++;
            k++;
            while(isspace(out_string[j])) j++;
            /* Removing white spaces at the end of the in_string */
            if(out_string[j] == '\0')
              out_string[k-1] = out_string[j];
          }
        }

  return out_string;
}
