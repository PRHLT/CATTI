%{
#include "lat-flex.h"
#include <float.h>
#define YYERROR_VERBOSE 1

//Global variables and definitions
int i, j;
int nodo;
edge_t *arco;
int state;
char *pal;
double ac_prob;
double lm_prob;
 
%}

%name-prefix="lat_"
%defines
%parse-param { wg_t *wg }


// Symbols.
%union
{
  int          ival;
  double       dval;
  char	      *sval;
}

/* Tokens from scanner (terminals) */
%token        END 0 "end of file"
%token        ENDL  "end of line"
%token <sval> STRING
%token <ival> INT
%token <dval> FLOAT
%token        SLF_VERSION UTTERANCE SUBLAT BASE LMNAME LMSCALE WDPENALTY
%token        ACSCALE AMSCALE VOCAB
%token        INITIAL_NODE FINAL_NODE
%token        NODES LINKS
%token        NODE TIME WORD SUBS VAR
%token        LINK START_NODE END_NODE DIV ACOUSTIC NGRAM LANGUAGE POSTERIOR
%token        UNK_OPTION
%token        UNEXPECTED

/* Type definition for rules (non-terminals) */
%type  <dval> number

%destructor { free($$); } STRING


%%

/*

The following rules define the syntax  of a lattice using the first character of 
the field names defined below to represent the entire field. Any unrecognised 
fields may be ignored and no user defined fields may share first characters with 
pre-defined field names. parentheses () indicate that terms within may appear 
in any order and braces {} indicate that terms within are optional.

lattice_file = header size_def terms
header       = ( V {\n} U {\n} S {\n} ) { base {\n} start {\n} end {\n} dir {\n}
                                   tscale {\n} lmscale {\n} lmpen {\n} }
size_def     = ( L N ) \n
terms        = node_def | link_def { terms }
node_def     = I { t W v d a } \n
link_def     = J ( S E { W v d a n l} ) \n

*/

//Axiom of the grammar
%start lattice_file;

/* File structure */
lattice_file: {@$;} /* the base case is empty line; @$ declares the lloc variable that stores the cursor position */
           | header 
             { /* Check compulsory paramenters */ }
             size_def 
             { /* Check valid lattice size */}
             terms 

/* header options */
header: { wg->GSF=1; wg->WIP=0.0;
    }/* empty */
  | header option

option: SLF_VERSION STRING {free($2);}
      | UTTERANCE   STRING {free($2);}
      | SUBLAT      STRING {free($2);}
      | BASE        number 
      | LMNAME      STRING {free($2);}
      | LMSCALE     number { wg->GSF=$2;  }    
      /* Word Penalty in log, as SRILM does */
      | WDPENALTY   number { wg->WIP=$2;  }
      | ACSCALE     number
      | AMSCALE     number
      | VOCAB       STRING {free($2);}
      | INITIAL_NODE  INT
      | FINAL_NODE    INT 
      | UNK_OPTION  STRING {free($2);}
      | ENDL

/* Size definitions */
size_def: /* empty */
        | size_def def

def: NODES INT { wg->size = $2;
                 wg->wg = (state_t *)calloc($2,sizeof(state_t)); 
                 wg->dict = create_dict(65536);    }
   | LINKS INT { /*wg->num_links = $2;*/}
   | ENDL


/* Nodes and links */
terms: /* empty */
     | terms term

term: node
    | link
    | ENDL 

/* Node */
node: NODE INT 
      {  nodo=$2;
       /*wg->wg[$2].final=-FLT_MAX;*/
         wg->wg[nodo].final=log(0.0);
	/* if ($2 == (wg->size - 1)) wg->wg[$2].final=0; */
        wg->wg[nodo].to.l=0;

        wg->wg[nodo].from.l=0;
        /*node_ptr = new Node(); 
        node_ptr->id = $2; */
      } 
      node_options 
      {
        /*lat->addNode(node_ptr);
        node_ptr = NULL;*/
      }
      ENDL
node_options: /* empty */
            | node_options node_option
node_option: TIME number { wg->wg[nodo].time=$2; /*node_ptr->time = int($2);*/ }
           | WORD STRING {      
               pal=$2;    
	       set_token_dict(wg->dict,$2);
               free($2);               /*node_ptr->word = $2;*/ }
           | SUBS STRING {free($2); /*node_ptr->sublat = $2;*/ }
           | VAR  INT    { /*node_ptr->var = $2;*/ }
           | UNK_OPTION  STRING {free($2);}

/* Links */
link: LINK INT 
      { 
	  arco= (edge_t *) malloc(sizeof(edge_t));
	  arco->prob=log(0.0);
	  arco->hmm_prob=0.0;
	  arco->lm_prob=0.0;
        /*link_ptr = new Link(lat->n_features); 
        link_ptr->id = $2; */
      } 
      link_options 
      {
        /*lat->addLink(link_ptr);
        link_ptr = NULL;*/
      }
      ENDL
link_options: /* empty */
            | link_options link_option
link_option: START_NODE INT    { 
	          state=$2;
         	  wg->wg[$2].to.edv=(edge_t **)realloc( wg->wg[$2].to.edv,(wg->wg[$2].to.l+1)*sizeof(edge_t *));
	          wg->wg[$2].to.edv[wg->wg[$2].to.l]=arco;
                  wg->wg[$2].to.l++;  
          	  arco->from=$2;
          }
           | END_NODE   INT    { 
	           state=$2;
                   wg->wg[$2].from.edv=(edge_t **)realloc( wg->wg[$2].from.edv,(wg->wg[$2].from.l+1)*sizeof(edge_t *));
                   wg->wg[$2].from.edv[wg->wg[$2].from.l]=arco;
                   wg->wg[$2].from.l++;  
                   arco->to=$2; }
           | WORD STRING { 
               pal=$2;    
	       set_token_dict(wg->dict,$2);
               arco->output=get_id_dict(wg->dict,$2);
               free($2);              }
           | VAR        INT    { /*link_ptr->var = $2;*/ }
           | DIV        STRING {free($2);/*link_ptr->div = $2;*/ }
           | ACOUSTIC   number {
	       ac_prob=$2; 
	       arco->hmm_prob=$2; }
           | NGRAM      number { /*link_ptr->ngram = lat->changeBase($2);*/ }
           | LANGUAGE   number {
	       lm_prob=$2; 
	       arco->lm_prob=$2; }
           | POSTERIOR  number { arco->prob=$2;/*link_ptr->posterior = log($2);*/ }
           | UNK_OPTION  STRING {free($2);}

/* Helper rules */
number:	FLOAT { $$ = $1; }
  | INT { $$ = (double) $1; }

%%

