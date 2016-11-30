/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file
\brief \d Library to handle stochastic finite-state transducers */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "auxiliar.h"
#include "zipio.h"
#include "dynamic_list.h"
#include "hash.h"
#include "dictionary.h"
#include "word_graph.h"
#include "sfst.h"


/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define INITIAL_SFST_STATE 0

/*****************************************************************************/
/*****************************VARIABLES***************************************/
/*****************************************************************************/

static sfst_t *sfst=NULL;
static edstate_t *edstate=NULL;

/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/

/** \d Structure that relates each state in the transducer with one state in the word graph */
typedef struct {
  int sfst_state; /**< \d SFST state id */
  int wg_state;   /**< \d Word graph state id */
} sfst2wg_state_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/*****************************************************************************/
/** \d Read a SFST in binary format */
int read_bin_sfst(char* filename, dict_t *indict, dict_t *outdict){

    FILE *fp;
    int i,s;
    int     lon;
    int error=1;
    int dict_size;
    char *word;

    if((fp = zopen(filename,"r"))==NULL){
    	fprintf(stderr,"Error opening transducer file %s\n",filename);
	    error = 0;
    	return error;
    }    

    mt(sfst=(sfst_t *)malloc(sizeof(sfst_t)));

    /* Stochastic Finite-State Transducers */
#ifdef VERBOSE
    fprintf(stderr,"Creating network...\n");fflush(stderr);
#endif    
    
    mt(sfst=(sfst_t *) malloc(sizeof(sfst_t)*(1)));
    fread(sfst,sizeof(sfst_t),1,fp);

#ifdef VERBOSE
    fprintf(stderr,"Creating state structure...\n");fflush(stderr);
#endif
    
    /* Estados */
    mt(sfst->state = (sfst_state_t *) malloc(sizeof(sfst_state_t)*(sfst->numstate)));
    fread(sfst->state,sizeof(sfst_state_t),sfst->numstate,fp);

#ifdef VERBOSE
    fprintf(stderr,"Creating edge structure...\n");fflush(stderr);
#endif
            
    /* Edges */
    mt(sfst->edge = (sfst_edge_t *) malloc(sizeof(sfst_edge_t)*(sfst->numedge)));    
    fread(sfst->edge,sizeof(sfst_edge_t),sfst->numedge,fp);

#ifdef VERBOSE    
    fprintf(stderr,"Creating output structure...\n");fflush(stderr);
#endif

    /* Output symbols */
    mt(sfst->out = (int *) malloc(sizeof(int)*(sfst->no)));
    fread(sfst->out,sizeof(i),sfst->no,fp);

#ifdef VERBOSE        
    fprintf(stderr,"Initial state structure...\n");fflush(stderr);
#endif

    /*Estados iniciales */
    mt(sfst->ini=(int *) malloc(sizeof(int)*(sfst->ni)));
    fread(sfst->ini,sizeof(i),sfst->ni,fp);

#ifdef VERBOSE    
    fprintf(stderr,"Final state structure...\n");fflush(stderr);
#endif

    /* Final states */
    mt(sfst->fin=(int *) malloc(sizeof(int)*(sfst->nf)));
    fread(sfst->fin,sizeof(i),sfst->nf,fp);

#ifdef VERBOSE    
    fprintf(stderr,"Input alphabet...\n");fflush(stderr);
#endif

    /* Dictionary indict */
    fread(&dict_size,sizeof(int),1,fp);
   
    for(i=0;i<dict_size;i++){
	fread(&lon,sizeof(int),1,fp);
	mt(word= (char *) malloc(sizeof(char)*(lon+1)));
	fread(word,sizeof(char),lon+1,fp);
	set_token_dict(indict,word);
    }

#ifdef VERBOSE    
    fprintf(stderr,"Output alphabet...\n");fflush(stderr);
#endif
      
    /* Dictionary outdict */    
    fread(&dict_size,sizeof(int),1,fp);
   
    for(i=0;i<dict_size;i++){
	fread(&lon,sizeof(int),1,fp);
	mt(word= (char *) malloc(sizeof(char)*(lon+1)));
	fread(word,sizeof(char),lon+1,fp);
	set_token_dict(outdict,word);
    }

#ifdef VERBOSE    
    fprintf(stderr,"Creating sorted vector for edges of each state...\n");fflush(stderr);
#endif

    /*Conversion of sequential arcs into sorted vector of arcs */
    mt(edstate=(edstate_t *)malloc(sfst->numstate*sizeof(edstate_t)));
    for(s=0;s<sfst->numstate;s++){
      fread(&(edstate[s].l),sizeof(int),1,fp);
      mt(edstate[s].insym=(int *)malloc(sizeof(int)*edstate[s].l));
      fread(edstate[s].insym,sizeof(int),edstate[s].l,fp);
      mt(edstate[s].edsymv=(edgesym_t *)malloc(sizeof(edgesym_t)*edstate[s].l));
      for(i=0;i<edstate[s].l;i++){
        fread(&(edstate[s].edsymv[i].l),sizeof(int),1,fp);
        mt(edstate[s].edsymv[i].edsym=(int *)malloc(sizeof(int)*edstate[s].edsymv[i].l));
        fread(edstate[s].edsymv[i].edsym,sizeof(int),edstate[s].edsymv[i].l,fp);
      }
    }

#ifdef VERBOSE    
    fprintf(stderr,"Creating sorted vector for edges of each state created...\n");fflush(stderr);
#endif
        
    fclose(fp);
    
#ifdef VERBOSE
    fprintf(stderr,"Transducer file was closed...\n");fflush(stderr);
#endif

    return error;
    
}

/****************************************************************************/
/*                          Binary search algorithm                         */
/****************************************************************************/
int bs(int *iv, int len, int i){

  int l=0,r=len-1,x;

  while(r>=l){
    x=(l+r)/2;
    if(i<iv[x]) r=x-1; else l=x+1;
    if(i==iv[x]) return x;
  }
  return -1;
}

/*****************************************************************************/
/** \d Create a state of the word graph */
state_t *create_word_graph_state(state_t *word_graph, int number_of_states, float final_state_prob){

    mt(word_graph = (state_t *) realloc(word_graph,((number_of_states)+1)*sizeof(state_t)));
    word_graph[number_of_states].final = LOG(final_state_prob);
    word_graph[number_of_states].to.l = 0;
    word_graph[number_of_states].to.edv = NULL;
    word_graph[number_of_states].from.l = 0;
    word_graph[number_of_states].from.edv = NULL;
    return word_graph;
}

/*****************************************************************************/
/** \d Create an edge of the word graph */
edge_t *create_word_graph_edge(int origin_state, int destiny_state, float prob, int output){

    edge_t *edge;

    mt(edge = (edge_t *) malloc(sizeof(edge_t)));
    edge->from = origin_state;
    edge->to = destiny_state;
    edge->prob = LOG(prob);
    edge->output = output;

    return edge;
}

/*****************************************************************************/
/** \d Add an outgoing edge to a state in the word graph */
void add_edge2origin_state(state_t *state, edge_t *edge){

  mt(state->to.edv = (edge_t **) realloc(state->to.edv,(state->to.l+1)*sizeof(edge_t *)));
  state->to.edv[state->to.l++] = edge;
  
}

/*****************************************************************************/
/** \d Add an incoming edge to a state in the word graph */
void add_edge2destiny_state(state_t *state, edge_t *edge){

  mt(state->from.edv = (edge_t **) realloc(state->from.edv,(state->from.l+1)*sizeof(edge_t *)));
  state->from.edv[state->from.l++] = edge;

}

/*****************************************************************************/
/** \d Create a sfst2wg_state_t structure given sfst and word graph state ids */
sfst2wg_state_t *create_sfst2wg_state(int sfst_state, int wg_state){

  sfst2wg_state_t *sfst2wg_state;

  mt(sfst2wg_state = (sfst2wg_state_t *) malloc(sizeof(sfst2wg_state_t)));
  sfst2wg_state->sfst_state = sfst_state;
  sfst2wg_state->wg_state = wg_state;

  return sfst2wg_state;
}

/*****************************************************************************/
/** \d Create a sfst2wg_state_t structure given sfst and word graph state ids */
void extract_sfst2wg_state(sfst2wg_state_t *sfst2wg_state, int *sfst_state, int *wg_state){

        *sfst_state = sfst2wg_state->sfst_state;
        *wg_state = sfst2wg_state->wg_state;
        free(sfst2wg_state);
}

/*****************************************************************************/
/** \d Construct a word graph given a source sentence and a transducer */
wg_t *sfst2word_graph(int *sentence, dict_t *indict, dict_t *outdict){

  int j, length;
  int Symbol_Empty,Symbol_Back_Off,Symbol_Unk;
  edge_t *edge=NULL;
  wg_t *wg;
  state_t *word_graph;
  int num_states;
  int from_wg_state,from_wg_state_cp,to_wg_state,to_wg_state_cp;
  int from_sfst_state,to_sfst_state;
  int sfst_edge,aux_sfst_edge;

  /* New variables for optimized search */
  int *curr_sfst2wg_state=NULL;
  int *next_sfst2wg_state=NULL;
  int curr_max_sfst_state=-1;
  int next_max_sfst_state=-1;
  list_t *curr_L;
  list_t *next_L;
  float *curr_score_state=NULL;
  float *next_score_state=NULL;
  float max_curr_score=LOG(0.0);
  float max_next_score=LOG(0.0);
  float prob;
  float beam_construction_BO, beam_construction;
  int edindex,e;
/*****************************************************************************/

  /* Initialize word graph */
  word_graph = NULL;
  num_states = 0;

  /* Some useful symbols that wil be employed when generating the word graph */
  Symbol_Back_Off = get_id_dict(indict,"<BACKOFF>"); /* Back-off symbol */
  Symbol_Unk = get_id_dict(indict,"<UNK>");          /* Unknown symbol */
  Symbol_Empty = get_id_dict(outdict," ");            /* Empty symbol */

  /* Compute sentence length */
  for(length=0;sentence[length]!=NO_WORD;length++);

  /* Beams to construct word graph are disabled */
  beam_construction_BO = beam_construction = LOG(0.0);

/*
  beam_construction_BO = LOG((length<5)?0:(length<30)?1e-3:1e-2);
  beam_construction = LOG((length<10)?1e-4:(length<25)?1e-3:1e-2);
*/

/*
  beam_construction_BO = LOG((length<5)?0:(length<30)?1e-1:8e-1);
  beam_construction = LOG((length<10)?1e-2:(length<25)?1e-1:8e-1);
*/

  /* Initialize list of active states in current state and next stage */
  curr_L = create_list(1);
  next_L = create_list(1);

  from_sfst_state=sfst->ini[INITIAL_SFST_STATE];
  from_wg_state=num_states++;

  word_graph = create_word_graph_state(word_graph, from_wg_state, sfst->state[from_sfst_state].final);

  /* Score of the initial state is 1.0 . This probability will be used to prune the word graph */
  mt(curr_score_state = (float *) realloc(curr_score_state,num_states*sizeof(float)));
  curr_score_state[from_wg_state] = 0.0;

  /* List of states to be visited in the current stage */
  insert_into_list(curr_L,create_sfst2wg_state(from_sfst_state,from_wg_state));

  /* Vector indexed by transducer state and store the state number in the word graph */
  /* We just increase the size of this vector to the maximum state number visited in the current stage */
  if(curr_max_sfst_state < from_sfst_state){
    mt(curr_sfst2wg_state = (int *) realloc(curr_sfst2wg_state,(from_sfst_state+1)*sizeof(int)));
    memset(curr_sfst2wg_state+curr_max_sfst_state+1,-1,(from_sfst_state-curr_max_sfst_state)*sizeof(int));
  }
  curr_sfst2wg_state[from_sfst_state] = from_wg_state;

  /***********************************************************************/
  /*                     WORD GRAPH CONSTRUCTION                         */
  /***********************************************************************/

  /* We are going to do this loop for each word in the source sentence */
  for (j=0;j<=length;j++) {

    /* While we have states to visit in the current stage */
    while(!EMPTY_LIST(curr_L)){

      /* Let's read an state from the head of the list of current states curr_L */
      extract_sfst2wg_state((sfst2wg_state_t *)extract_from_list(curr_L),&from_sfst_state,&from_wg_state);

      /*******************************************************/
      /*            DEALING FIRST WITH THE BACKOFF EDGE      */
      /*******************************************************/

      /* There is only one backoff edge for each state, let's visit is first */
      edindex = bs(edstate[from_sfst_state].insym, edstate[from_sfst_state].l, Symbol_Back_Off);

      /* Just checking this backoff edge exists and we are not in the unigram */
      if(edindex!=-1){

        /* There is only one edge with the BACKOFF symbol */
        sfst_edge=edstate[from_sfst_state].edsymv[edindex].edsym[0];

        /* Applying beam */
        if(LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state]>beam_construction_BO+max_curr_score){

          /* Let's see to which state the backoff edge is taking us */
          to_sfst_state = sfst->edge[sfst_edge].dest;

          /* Controling which state in the transducer is associated to which state 
             in the word graph in the current state */
          if(curr_max_sfst_state < to_sfst_state){
            /* Increasing size of curr_sfst_state2presence_sl_index */
            mt(curr_sfst2wg_state = (int *) realloc(curr_sfst2wg_state,(to_sfst_state+1)*sizeof(int)));
            memset(curr_sfst2wg_state+curr_max_sfst_state+1,-1,(to_sfst_state-curr_max_sfst_state)*sizeof(int));
            curr_max_sfst_state = to_sfst_state;
          }

          /* Once we have increased the size of curr_sfst2wg_state if needed, then
          checking if state was already created */
          if(curr_sfst2wg_state[to_sfst_state]==-1){

            /* Inserting state list index into next_sfst_state2presence_sl_index */
            to_wg_state=num_states++;
            curr_sfst2wg_state[to_sfst_state] = to_wg_state;

            /* If we have parsed the input sentence completely (j==length) start considering final states */
            word_graph = create_word_graph_state(word_graph, to_wg_state,(j==length)?sfst->state[to_sfst_state].final:0.0);

            /* Storing probability of reaching dest state */
            mt(curr_score_state = (float *) realloc(curr_score_state,num_states*sizeof(float)));
            curr_score_state[to_wg_state]=LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state];

            /* Inserting new state to be visited in the next phase */
            insert_into_list(curr_L,create_sfst2wg_state(to_sfst_state,to_wg_state));
          }
          /* If we have already created this state in the current state, then we just need to know what
          number we assigned to that state in the word graph, that is known through curr_sfst_state2presence_sl_index[to_sfst_state] */
          else{
            to_wg_state=curr_sfst2wg_state[to_sfst_state];
            /* Just in case the new probability is better than the one we had before */
            prob=LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state];
            curr_score_state[to_wg_state]=MAX(prob,curr_score_state[to_wg_state]);
          }/* Close of if(curr_sfst_state2presence_sl_index[to_sfst_state]==-1){ */

          max_curr_score=MAX(curr_score_state[to_wg_state],max_curr_score);

          /* Creating edge to go from from_sfst_state to to_sfst_state in the transducer 
             or from from_wg_state to to_wg_state in the word graph */
          edge = create_word_graph_edge(from_wg_state,to_wg_state,sfst->edge[sfst_edge].prob,Symbol_Empty);

          /* Inserting new edge in origin state */
          add_edge2origin_state(&(word_graph[from_wg_state]),edge);

          /* Inserting new ede in destiny state */
          add_edge2destiny_state(&(word_graph[to_wg_state]), edge);

        }/*End of beam: if(LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state]>beam_construction_BO+max_curr_score){ */

      } /* Close if(edindex!=-1){ */
      /* We have checked if there is a BACKOFF edge, but there wasn't , so we are in the unigram */
      else{

        /* This is a way to save my back, just in case end our input sentence and we haven't found any state
        using backoff that has a final probability different from zero */
        if(j==length) continue;

        /* We are in the unigram, let's see first if there is any edge compatible with the source word
        otherwise we will be forced to use the unknown edge */

        /* Obtain list of edges compatible with the input word */
        edindex = bs(edstate[from_sfst_state].insym, edstate[from_sfst_state].l, sentence[j]);

        /* If there is no compatible symbol with this source word go to the next state */
        if(edindex==-1){

          edindex = bs(edstate[from_sfst_state].insym, edstate[from_sfst_state].l, Symbol_Unk);

          /* There is only one symbol with the UNK symbol */
          sfst_edge=edstate[from_sfst_state].edsymv[edindex].edsym[0];

          /* Applying beam */
          if(LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state]>beam_construction+max_next_score){

            to_sfst_state = sfst->edge[sfst_edge].dest;

            /* Increasing the vector that stores which transducer state is associated 
               to each word graph state in the next stage */
            if(next_max_sfst_state < to_sfst_state){
              /* Increasing size of next_sfst_state2presence_sl_index */
              mt(next_sfst2wg_state = (int *) realloc(next_sfst2wg_state,(to_sfst_state+1)*sizeof(int)));
              memset(next_sfst2wg_state+next_max_sfst_state+1,-1,(to_sfst_state-next_max_sfst_state)*sizeof(int));
              next_max_sfst_state = to_sfst_state;
            }

            /* Once we have increased the size of next_sfst_state2presence_sl_index if needed, then
            checking if state was already created */
            if(next_sfst2wg_state[to_sfst_state]==-1){

              /* Inserting state list index into next_sfst_state2presence_sl_index */
              to_wg_state=num_states++;
              next_sfst2wg_state[to_sfst_state] = to_wg_state;

              /* Creating new state in the word graph */
              word_graph = create_word_graph_state(word_graph, to_wg_state, (j==length-1)?sfst->state[to_sfst_state].final:0.0);

              /* Probability of reaching this state */
              mt(next_score_state = (float *) realloc(next_score_state,num_states*sizeof(float)));
              next_score_state[to_wg_state]=LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state];

              if(!((j==length-1)&&(sfst->state[to_sfst_state].final!=0.0)))
                /* Inserting new state to be visited in the next phase */
                insert_into_list(next_L,create_sfst2wg_state(to_sfst_state,to_wg_state));
            }
            /* In case we have already reach this state in the current stage, just leave the highest probability */
            else{
              to_wg_state=next_sfst2wg_state[to_sfst_state];
              prob=LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state];
              next_score_state[to_wg_state]=MAX(prob,next_score_state[to_wg_state]);
            }

            max_next_score=MAX(next_score_state[to_wg_state],max_next_score);
 
            /* Creating edge to go from from_sfst_state to to_sfst_state in the transducer or 
               from from_wg_state to to_wg_state in the word graph */
            /* edge = create_word_graph_edge(from_wg_state, to_wg_state, sfst->edge[sfst_edge].prob, Symbol_Empty); */
            /* Paco suggested to do so, in PRHLT seminar 06-05-2004 */
            edge = create_word_graph_edge(from_wg_state, to_wg_state, sfst->edge[sfst_edge].prob, set_token_dict(outdict,get_token_dict(indict,sentence[j])));

            /* Inserting new edge in origin state */
            add_edge2origin_state(&(word_graph[from_wg_state]),edge);

            /* Inserting new ede in destiny state */
            add_edge2destiny_state(&(word_graph[to_wg_state]), edge);

          }/* End of if(edindex==-1){ */

        }/*End of beam: if(LOG(sfst->edge[sfst_edge].prob)+curr_score_state[posic]>beam_construction+max_next_score){ */

      } /* Close of else of if(edindex!=-1){ */

      /**************************************/
      /* TIME TO DEAL WITH COMPATIBLE EDGES */
      /**************************************/

      /* We have consumed all the symbols in the source sentence we are just going over lambda edges
         to finish the word graph construction smoothly, by the way this if avoids a dangerous situation when
         using sentence[j==length] :-) */
      if(j==length) continue;

      /* Obtain list of edges compatible with the input word */
      edindex = bs(edstate[from_sfst_state].insym, edstate[from_sfst_state].l, sentence[j]);

      /* If there is no compatible symbol with this source word go to the next state */
      if(edindex==-1) continue;

      /*We are going to analyze those edges compatible with the input word */
      for(e=0;e<edstate[from_sfst_state].edsymv[edindex].l;e++){

        sfst_edge=edstate[from_sfst_state].edsymv[edindex].edsym[e];

        /* Applying beam */
        if(LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state]>beam_construction+max_next_score){

          to_sfst_state = sfst->edge[sfst_edge].dest;

          if(next_max_sfst_state < to_sfst_state){
            /* Increasing size of next_sfst_state2presence_sl_index */
            mt(next_sfst2wg_state = (int *) realloc(next_sfst2wg_state,(to_sfst_state+1)*sizeof(int)));
            memset(next_sfst2wg_state+next_max_sfst_state+1,-1,(to_sfst_state-next_max_sfst_state)*sizeof(int));
            next_max_sfst_state = to_sfst_state;
          }

          /* Once we have increased the size of next_sfst_state2presence_sl_index if needed, then
             checking if state was already created */
          if(next_sfst2wg_state[to_sfst_state]==-1){

            /* Inserting state list index into next_sfst_state2presence_sl_index */
            to_wg_state=num_states++;
            next_sfst2wg_state[to_sfst_state] = to_wg_state;

            /* Creating new state in the word graph */
            word_graph = create_word_graph_state(word_graph, to_wg_state, (j==length-1)?sfst->state[to_sfst_state].final:0.0);

            mt(next_score_state = (float *) realloc(next_score_state,num_states*sizeof(float)));
            next_score_state[to_wg_state]=LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state];

            /* Inserting new state to be visited in the next phase */
            insert_into_list(next_L,create_sfst2wg_state(to_sfst_state,to_wg_state));

          }
          else{
            to_wg_state=next_sfst2wg_state[to_sfst_state];
            prob=LOG(sfst->edge[sfst_edge].prob)+curr_score_state[from_wg_state];
            next_score_state[to_wg_state]=MAX(prob,next_score_state[to_wg_state]);
          }/* End of else if(next_sfst_state2presence_sl_index[to_sfst_state]==-1){ */

          max_next_score = MAX(next_score_state[to_wg_state],max_next_score);

          if(sfst->edge[sfst_edge].ot<=1){

            /* Creating edge to go from from_sfst_state to to_sfst_state in the transducer 
               or from from_wg_state to to_wg_state in the word graph */
            edge = create_word_graph_edge(from_wg_state,to_wg_state,sfst->edge[sfst_edge].prob, (sfst->edge[sfst_edge].ot==0)?Symbol_Empty:sfst->out[sfst->edge[sfst_edge].pt]);

            /* Inserting new edge in origin state */
            add_edge2origin_state(&(word_graph[from_wg_state]), edge);

            /* Inserting new ede in destiny state */
            add_edge2destiny_state(&(word_graph[to_wg_state]), edge);

          }/* End of if(sfst->edge[sfst_edge].ot<=1){ */
          /* There is more than one target symbols, so we need to create fake states to put each target
             word in a different edge */
          else{
            to_wg_state_cp = to_wg_state;
            from_wg_state_cp = from_wg_state;

            /* For each target word create a state and an edge */
            for(aux_sfst_edge=sfst->edge[sfst_edge].pt;aux_sfst_edge<(sfst->edge[sfst_edge].pt+sfst->edge[sfst_edge].ot)-1;aux_sfst_edge++){

              /* Creating new state in the word graph */
              to_wg_state = num_states++;
              word_graph = create_word_graph_state(word_graph,to_wg_state,0.0);

              /* Creating edge to go from from_sfst_state to to_sfst_state in the transducer 
                 or from from_wg_state to to_wg_state in the word graph */
              edge = create_word_graph_edge(from_wg_state, to_wg_state, (aux_sfst_edge==sfst->edge[sfst_edge].pt)?sfst->edge[sfst_edge].prob:1.0, sfst->out[aux_sfst_edge]);

              /* Inserting new edge in origin state */
              add_edge2origin_state(&(word_graph[from_wg_state]), edge);

              /* Inserting new ede in destiny state */
              add_edge2destiny_state(&(word_graph[to_wg_state]), edge);

              from_wg_state = to_wg_state;

            }/* End of for(aux_sfst_edge=sfst->edge[sfst_edge].pt;aux_sfst_edge<(sfst->edge[sfst_edge].pt+sfst->edge[sfst_edge].ot)-1;aux_sfst_edge++){ */

            aux_sfst_edge = (sfst->edge[sfst_edge].pt+sfst->edge[sfst_edge].ot)-1;

            /* Creating edge to go from from_sfst_state to to_sfst_state in the transducer 
               or from from_wg_state to to_wg_state in the word graph */
            to_wg_state = to_wg_state_cp;
            edge = create_word_graph_edge(from_wg_state, to_wg_state, 1.0, sfst->out[aux_sfst_edge]);

            /* Inserting new edge in origin state */
            add_edge2origin_state(&(word_graph[from_wg_state]),edge);

            /* Inserting new ede in destiny state */
            add_edge2destiny_state(&(word_graph[to_wg_state]),edge);

            /* Recover from_wg_state since it's used for all edges leaving from from_sfst_state */
            from_wg_state = from_wg_state_cp;

          }/* End of else of if(sfst->edge[sfst_edge].ot<=1){*/

        }/*End of beam*/

      } /* Close while (!EMPTY_LIST((&auxiliar_list)))*/

    }/*Close while(!EMPTY_LIST(curr_L)) */

    free(curr_sfst2wg_state);
    curr_sfst2wg_state = next_sfst2wg_state;
    next_sfst2wg_state = NULL;
    curr_max_sfst_state = next_max_sfst_state;
    next_max_sfst_state = -1;
    DESTROY_LIST(curr_L);
    curr_L = next_L;
    next_L = create_list(1);

    free(curr_score_state);
    curr_score_state = next_score_state;
    mt(next_score_state = (float*) malloc(num_states*sizeof(float)));

    max_curr_score = max_next_score;
    max_next_score = LOG(0.0);

  } /* Close for j */

  /* Free all data structures employed during the word graph creation */
  while(!EMPTY_LIST(curr_L))
    extract_sfst2wg_state((sfst2wg_state_t *)extract_from_list(curr_L),&from_sfst_state,&from_wg_state);

  DESTROY_LIST(curr_L);
  DESTROY_LIST(next_L);
  free(curr_score_state);
  free(next_score_state);
  free(curr_sfst2wg_state);

  /* Create word graph data structure */
  mt(wg=(wg_t *) malloc(sizeof(wg_t)));
  wg->wg=word_graph;
  wg->size=num_states;
  wg->dict=NULL;
  wg->state_order=NULL;

  return(wg);
}

