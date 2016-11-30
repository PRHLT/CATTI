/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file
\brief \d Library to handle stochastic finite-state transducers */
#ifndef SFST_INCLUDED
#define SFST_INCLUDED

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define MAXLINE 10000

/*****************************************************************************/
/************************ TYPE DEFINITION ************************************/
/*****************************************************************************/

/*------- Types to represent a generic finite-state transducer --------*/

/** \d  Data structure representing a state in a stochastic finite-state transducer */
typedef struct{
  int     pt;      /**< \d Index to the first output word in array out */
  int     psal;    /**< \d Index to the first edge in array edge */
  char    ot;      /**< \d Number of output symbols */
  double  checksum;/**< \d Probability checksum */
  double  initial; /**< \d Initial state probability */
  double  final;   /**< \d Final state probability */
} sfst_state_t;

/** \d Data structure representing an edge in a stochastic finite-state transducer */
typedef struct{
  int        pt;  /**< \d Index to the first word in array out */
  int        dest;/**< \d Destiny state */
  int        ssal;/**< \d Index to next edge in array edge */
  int        simb;/**< \d Input symbol in alfaent */
  char       ot;  /**< \d Number of output symbols */
  double     prob;/**< \d Transition probability */
} sfst_edge_t;

/** \d Data structure representing a stochastic finite-state transducer */
typedef struct
{
  char            name[MAXLINE];/**< \d Name of the SFST */
  int             type;         /**< \d Type of SFST */
  int             typeA;        /**< \d Type of edges */
  int             typeE;        /**< \d Type of states */
  int             numstate;     /**< \d Number of states */
  int             numedge;      /**< \d Number of edges */
  int             ns;           /**< \d Number of states */
  int             ne;           /**< \d Number of edges */
  int             no;           /**< \d Number of output states */
  int             ni;           /**< \d Number of initial states */
  int             nf;           /**< \d Number of final states */
  int             maxE;         /**< \d Maximum number of states */
  int             maxA;         /**< \d Maximum number of edges */
  int             maxO;         /**< \d Maximum number of output states */
  int             maxI;         /**< \d Maximum number of initial states */
  int             maxF;         /**< \d Maximum number of final states */
  double          totchk;       /**< \d Total checksum */
  sfst_state_t    *state;       /**< \d Array of states */
  sfst_edge_t     *edge;        /**< \d Array of edges */
  int             *out;         /**< \d Array of output symbols */
  int             *ini;         /**< \d Array of initial states */
  int             *fin;         /**< \d Array of final states */
} sfst_t;

/** \d Data structure for SFST edges for a given input symbol */
typedef struct{
  int l;     /**< \d Size of array of output symbols */
  int *edsym;/**< \d Array of output symbols */
} edgesym_t;

/** \d Data structure for SFST state */
typedef struct{
  int l;            /**< \d Size of sorted arrays */
  int *insym;       /**< \d Sorted array of input symbols of outgoing edges of this state for binary search */
  edgesym_t *edsymv;/**< \d Array of edges for each input symbol */
} edstate_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/
/** \d Read a SFST in binary format */
int read_bin_sfst(char* filename, dict_t *indict, dict_t *outdict);

/** \d Construct a word graph given a source sentence and a transducer */
wg_t *sfst2word_graph(int *sentence, dict_t *indict, dict_t *outdict);

#endif
