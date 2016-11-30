/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file
\brief \d File containing global constants, macros and variables that are used all over the place */
#ifndef GLOBAL_INCLUDED
#define GLOBAL_INCLUDED

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define NO_WORD -1


/******************************************************************************/
/*****************************MACROS*******************************************/
/******************************************************************************/

/** Compute maximum of two values */
#define MAX(A,B) (A>B)?A:B;

/** Compute minimum of two values */
#define MIN(A,B) (A<B)?A:B;

/** \d Log function employed as LOG */
#define LOG(X) log(X)


/*****************************************************************************/
/************************GLOBAL VARIABLES*************************************/
/*****************************************************************************/


enum tipoCAT { TRANSCRIPCION, TRADUCCION } tipo_CAT;
char **segments;   /**< \d Array of source sentences */
char **references; /**< \d Array of reference sentences */
char **wordgraphs; /**< \d Array of the name of the different wordgraphs */
int interface;     /**< \d Flag to indicate if we are interacting with a GUI */
int preprocess;    /**< \d Flag to indicate if preprocess is active  */
char *table;       /**< \d Recase table filename in XRCE and EU corpora */
char *exceptions;  /**< \d Proper noun table filename in EU corpus */
int disable_pospro;
int istate;

typedef struct {
  double start;
  double end;
} alignment_t;

/** completions info */
typedef struct {
  char **completions;
  alignment_t **alignments;
} completions_t;
#endif
