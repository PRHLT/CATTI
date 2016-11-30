/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Include the set of functions used by the GUI */
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <float.h>
#include <ctype.h>
#include <limits.h>

/********************/
#include <sys/time.h>
/********************/

#include "auxiliar.h"
#include "dynamic_list.h"
#include "hash.h"
#include "dictionary.h"
#include "strop.h"
#include "librefx.h"
#include "word_graph.h"
#include "sfst.h"
#include "global.h"
#include "CAT.h"
#include "vector.h"
#include "ecparser.h"
//#include "TT2.h"

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define DICT_HASH_TABLE_SIZE 65536
#define MAX_COMPLETIONS 5
#define MAX_COMPLETION_LENGTH 256

/*****************************************************************************/
/*****************************VARIABLES***************************************/
/*****************************************************************************/

static int max_completions=MAX_COMPLETIONS;/**< \d Maximum number of n-best translations */
char *prefix=NULL;                         /**< \d Prefix validated by the user  */

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/*****************************************************************************/
/** \d Set maximum number of completions to be returned  */
 int ipeSetMaxCompletions(int max_comps){ 
   max_completions=max_comps; 
   return 1; 
 } 


/*****************************************************************************/
/** \d This function reads the word graphs of the sentences to be recognized */
int ipeLoadwordgraphs(char *names){

    int error=1,nl=0;
    FILE *fp=NULL;

    if (names != NULL) {
	if ( (fp=fopen(names,"r")) == NULL ) {
	    fprintf(stderr,"\nError opening file %s\n",names);fflush(stderr);
	    error = 0;
        }
    }
    else error = 0;

    wordgraphs=NULL;
    mt(wordgraphs= (char **) realloc(wordgraphs,sizeof(char *)*(nl+1)));
    wordgraphs[nl]=NULL;
    while(readline(fp,&(wordgraphs[nl]))>0){
      nl++;
      mt(wordgraphs= (char **) realloc(wordgraphs,sizeof(char *)*(nl+1)));
      wordgraphs[nl]=NULL;
    }

    fclose(fp);
    return error;
}

/*****************************************************************************/
/*\d This function reads the sentences to be translated*/
int ipeLoadText(char *source){

    int error=1,nl=0;
    FILE *fp=NULL;

    if (source != NULL) {
	if ( (fp=fopen(source,"r")) == NULL ) {
	    fprintf(stderr,"\nError opening source file %s\n",source);fflush(stderr);
	    error = 0;
        }
    }
    else error = 0;

    segments=NULL;
    mt(segments= (char **) realloc(segments,sizeof(char *)*(nl+1)));
    segments[nl]=NULL;
    while(readline(fp,&(segments[nl]))>0){nl++;
      mt(segments= (char **) realloc(segments,sizeof(char *)*(nl+1)));
      segments[nl]=NULL;
    }

    fclose(fp);
    return error;
}

/*****************************************************************************/
/** \d This function reads the reference transcriptions */
int ipeLoadReference(char *reference){

    int error=1,nl=0;
    FILE *names=NULL;
    FILE *fp=NULL;
    char *name=NULL;        

    if (reference != NULL) {
      if ( (names=fopen(reference,"r")) == NULL ) {
        fprintf(stderr,"\nError opening reference file %s\n",reference);
        error = 0;
      }
    }
    else
      error = 0;

    references=NULL;
    mt(references= (char **) realloc(references,sizeof(char *)*(nl+1)));
    references[nl]=NULL;
   
    if(name!=NULL) free(name);
    while(readline(names,&(name))>0){
	if ( (fp=fopen(name,"r")) == NULL ) {
	    fprintf(stderr,"\nError opening file %s\n",name);
	    error = 0;
	}        
	if(readline(fp,&(references[nl]))>0){
            nl++;
            mt(references= (char **) realloc(references,sizeof(char *)*(nl+1)));
            references[nl]=NULL;
	}
	else{
	    fprintf(stderr,"\n The file %s is empty\n",name);
	    error = 0; 
	}
	fclose(fp);
        free(name); 
    }
    fclose(names);
    free(name);
    return error;
}


/*****************************************************************************/
/** \d This function sets the user prefix */
  int ipeSetTargetText(const char* target){ 
// 
    int error=1;  
// 
    if(prefix) free(prefix); 
// 
    if(target!=NULL){
    	if(!(prefix = strdup2(target))){ 
       	    fprintf(stderr,"Error not enough memory for prefix in ipeSetTargetText\n"); 
            error = 0; 
        }
    } 
    return error; 
  } 

/*****************************************************************************/
void preproceso(char* entrada, char **salida,int position){

   // if(tipo_CAT==TRANSCRIPCION){
        *salida = strdup2(entrada);
	//if(position >= (int) strlen(prefix)){//Is the last word of the prefix
	 //   mt(*salida=(char *)realloc((*salida),sizeof(char)*(position+1))); 
	 //   (*salida)[position-1]=' ';
	 //   (*salida)[position]='\0';
	//}
    //}

}
/*****************************************************************************/
void posproceso(char *best_completions,char *pospro){

    
    int i;

//  if(tipo_CAT==TRANSCRIPCION){
     i=0;
     if(((pospro[0]=='|') && (pospro[1]==' ')) || ((pospro[0]=='-') && (pospro[1]==' '))) pospro[0]=' ';
//(pospro[i]=='*') || (pospro[i]=='(') || (pospro[i]==')') ||
     while(pospro[i]!='\0'){
         if((pospro[i]=='.') || (pospro[i]=='/') || (pospro[i]=='#') || (pospro[i]==';') || (pospro[i]==':') || (pospro[i]=='+') ||  (pospro[i]==',') || (pospro[i]=='~') || (pospro[i]=='!') || (pospro[i]=='?') || (pospro[i]=='<') || (pospro[i]=='>'))
		 pospro[i]=' '; 
	 if(i>0)
           if (((pospro[i-1]==' ')&&(pospro[i]=='-')&&(pospro[i+1]==' ') ) || ((pospro[i-1]==' ')&&(pospro[i]=='_')))
// || ((pospro[i-1]==' ') && (pospro[i]=='|'))) 
             pospro[i]=' '; 	
        
        i++;
     }
	
  //}


} 
/*****************************************************************************/
/** \d This function returns the best completions given the position of the cursor in the user prefix */
int ipeProvideCompletions(completions_t *completions, int cursorPosition,float beam, vector_t *first_word_last_suffix,int multimodality,int pot,unsigned char *errorCorrecting,wg_t *wg,wg_t *unigram, int ECAlg){
 
     int error=0;
     int i,j=0,h,c=0;
     char *prepro=NULL;
     char *pospro=NULL;
     int *int_prefix=NULL;
     char *actual_prefix=NULL;
     char *final_prefix=NULL;
     dict_t *wg_dict;
     completions_t best_completions;
     char *prefix_last_word=NULL;
     char character;
     alignment_t align;
     double n=-1.0;
     
     
     align.start=0.0;
     align.end=0.0;

     best_completions.completions=NULL;
     best_completions.alignments=NULL; 
    
//    /*****************************************/
//    /* EXTRACT USER PREFIX AND PREPROCESS IT */
//    /*****************************************/
     
     wg_dict=wg->dict; 
     
     actual_prefix = strndup2(prefix, cursorPosition);
     
    // fprintf(stderr,"\ncatti-plugin: actual_prefix '%s'\n",actual_prefix);fflush(stderr);
     
     if(strlen(actual_prefix)>0){   
	 
	preproceso(actual_prefix,&prepro,cursorPosition);
    //    fprintf(stderr,"\ncatti-plugin: prepro '%s'\n",prepro);fflush(stderr);
     	if(get_word_graph_initial_symbol() != INT_MAX){
	    mt(final_prefix=(char *)malloc(sizeof(char)*strlen(prepro)+5));
            strcpy(final_prefix, "<s>");
            strcat(final_prefix, " ");
	    strcat(final_prefix, prepro);
	}
	else{
	  mt(final_prefix=(char *)malloc(sizeof(char)*strlen(prepro)+1));
	  strcpy(final_prefix, prepro);   
	}

		
        if (isspace(final_prefix[strlen(final_prefix)-1])){
	   int_prefix=str2seq(wg_dict,final_prefix);
           prefix_last_word = strdup2("");
        }
        else{
	   /* Find the first space to define the last word in prefix */
          for(i=strlen(final_prefix)-1;(i>=0)&&(!isspace(final_prefix[i]));i--);
	  prefix_last_word=&(final_prefix[i+1]);
	  j=i+1;
          /* Isolate last word in prefix */
          if(i>=0){
	      character=final_prefix[i];
	      final_prefix[i]='\0';
	  }
          else{
	      character=final_prefix[0];
	      final_prefix[0]='\0';
	  }
          /* Convert to sequence of ids */
          int_prefix=str2seq(wg_dict,final_prefix);
          /* Recover last word from isolation */
          if(i>=0) final_prefix[i]=character;
          else     final_prefix[0]=character;
	}
     }
     /* If the actual prefix is empty then set an empty prefix */
     else{
        if(get_word_graph_initial_symbol() != INT_MAX){
         final_prefix = strdup2("<s> ");
	 int_prefix=str2seq(wg_dict,final_prefix);
	}
	else{
	 final_prefix = strdup2("");
	 int_prefix=str2seq(wg_dict,final_prefix);
	}
        prefix_last_word = strdup2("");
      }
     // fprintf(stderr,"\ncatti-plugin: final_prefix '%s'\n",final_prefix);fflush(stderr);
      
     // fprintf(stderr,"\ncatti-plugin: prefix_last_word %s\n",prefix_last_word);fflush(stderr);
      
/*************************************************/
     /* OBTAIN BEST COMPLETIONS AND POSTPROCESS THEM  */
/*************************************************/
 
/* Search n-best suffix in the word graph  */

    
    
     best_completions = complete_prefix(final_prefix, int_prefix, prefix_last_word, max_completions, wg_dict,beam,first_word_last_suffix,multimodality,pot,errorCorrecting,wg,unigram,ECAlg);

     i=0;
     if (completions->completions != NULL){
        while(completions->completions[i]!=NULL){
           free(completions->completions[i]);
           i++;
        }
        free(completions->completions);
     }
     
     if (completions->alignments != NULL){
        while(completions->alignments[i]!=NULL){
           free(completions->alignments[i]);
           i++;
        }
        free(completions->alignments);
     }
     
     
     
     i=0;
     completions->completions=NULL;
     completions->alignments=NULL;

     while(best_completions.completions[i]!=NULL){

        pospro=strdup2(best_completions.completions[i]);
	if (!disable_pospro) posproceso(best_completions.completions[i],pospro); 
	
        mt((completions->completions) = (char **) realloc((completions->completions),(i+1)*sizeof(char*)));
	
	if(get_word_graph_initial_symbol() != INT_MAX){
           completions->completions[i]=strdup2(pospro + strlen(actual_prefix)+4-strlen(prefix_last_word));
	}
	else{
	   completions->completions[i]=strdup2(pospro + strlen(actual_prefix)-strlen(prefix_last_word));
	}

     

        if(get_word_graph_final_symbol()!=INT_MAX)     /*Se elimina el simbolo </s> del final del sufijo*/ 
	   if(strlen(completions->completions[i])>=strlen(get_token_dict(wg_dict,get_word_graph_final_symbol())))
		  completions->completions[i][strlen(completions->completions[i])-strlen(get_token_dict(wg_dict,get_word_graph_final_symbol()))]='\0';
        i++;
	  
    }
	
	
    i=0;
    completions->alignments=NULL;    
    while(best_completions.alignments[i]!=NULL){
         mt((completions->alignments) = (alignment_t **) realloc((completions->alignments),(i+1)*sizeof(alignment_t *)));
	 h=0;
	// align.start=best_completions.alignments[i][h].start;
	// align.end=best_completions.alignments[i][h].end;
	 mt(completions->alignments[i] = (alignment_t *) malloc((h+1)*sizeof(alignment_t))); 
         while(best_completions.alignments[i][h].start!=-1.0){
	 //while((best_completions.alignments[i][h].start)!=1.0){
	 //  completions->alignments[i][h].start = align.start;
	 //  completions->alignments[i][h].end=align.end;
	 //  h++;
	   completions->alignments[i][h].start=best_completions.alignments[i][h].start;
	   completions->alignments[i][h].end=best_completions.alignments[i][h].end;
	   h++;
	   mt(completions->alignments[i] = (alignment_t *) realloc(completions->alignments[i], (h+1)*sizeof(alignment_t)));
	   //align.start=best_completions.alignments[i][h].start;
	   //align.end=best_completions.alignments[i][h].end;
	 }
	 completions->alignments[i][h].start= -1.0;
	 completions->alignments[i][h].end= -1.0;
	 i++;
    }
    

       
   
    c=0;
    if (best_completions.completions != NULL){
      while(best_completions.completions[c]!=NULL){
	free(best_completions.completions[c]); 			
	c++;
      }
      free(best_completions.completions);
    }
    best_completions.completions=NULL;
       
    c=0;
    if (best_completions.alignments != NULL){
      while(best_completions.alignments[c]!=NULL){
      	 free(best_completions.alignments[c]); 			
	 c++;
      }
      free(best_completions.alignments);
    }
    best_completions.alignments=NULL;
    
    free(actual_prefix);
    free(int_prefix);  
    if(prefix_last_word!=final_prefix+j) free(prefix_last_word);
    free(prepro);
    free(final_prefix);
    free(pospro);
    
    mt(completions->completions = (char **) realloc(completions->completions,(i+1)*sizeof(char*)));
    completions->completions[i]=NULL;

    mt(completions->alignments = (alignment_t **) realloc(completions->alignments,(i+1)*sizeof(alignment_t*)));
    completions->alignments[i]=NULL;
    
    
    error=1;
    return error;
}

