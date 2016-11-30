#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <unistd.h>
#include <ctype.h>
#include <values.h>
#include <limits.h>


#include "global.h"
#include "auxiliar.h"
#include "vector.h"
#include "dynamic_list.h"
#include "dynamic_ordered_list.h"
#include "hash.h"
#include "dictionary.h"
#include "heap.h"
#include "strop.h"
#include "librefx.h"
#include "word_graph.h"
#include "ecparser.h"
#include "nbestsuffix.h"
#include "CAT.h"
#include "entropy.h"

 
#define FALSE 0
#define TRUE 1
#define BEST_COMPLETION 1
#define NBEST_COMPLETIONS 5

/********************************************************************************/
void usage(char *prog){

    fprintf(stdout,"\n\
usage: %s <options below>\n\
\n\
   -w <filename>   plain text file containing a list wordgraphs \n\
   -y <filename>   plain text file containing a list of hypothesis \n\
   -t <filename>   plain text file containing a list of transcriptions \n\
   -d              disable posprocess \n\
   -e              1-best transcription for each text image in -w <filename>\n\
   -s              1-best word stroke ratio calculation.\n\
   -l <filename>   generate a log file\n\
   -g <float>      set the grammar scale factor to <float>\n\
   -p <float>      set the word insertion penalty to <float>\n\
   -c <float>      set the error correcting scale factor to <float>.Default -100\n\
   -C <int>        the error correcting tipe: 0 default (modifying probability with levenstein distance), 1 (modifying probability without levensteind distance), 2 (cost)\n\
   -v <float>      set word end pruning threshold to <float>\n\
   -n <int>        set the max number of clics to <int>\n\
   -m <int>        set the type of source (1=transcription, 2=translation). Default transcription\n\
   -a              enable only transparent clicks (escenario a)\n\
   -o <string>     enable the multimodality scenario and set the path of the online wordgraphs\n\
   -b <float>      set the online scale factor to <float>. Default 1 \n\
   -z <string>     character model and <string> is the name of the unigram file \n\
   -O <string>     define the different opperations ibtusr. Default ibtusr \n\
   -N              prediction disabled when only there is an error \n\
   -E <float>      compute the entropy of the wordgraphs. The <float> is the entropy scale factor. \n\
   -D              disable the wordgraph normalization in the entropy process. \n\
   -A              show the alignments \n\
   -V              verbosity \n\
   -h              this message \n\n\
   \n",prog); 

    exit(0);

}

/*********************************************************************************/
 
 int isDifferent(int *ref, int *hip){
    int iref=0;
    int ihip=0;
    int different=FALSE;
    
    while((hip[ihip]!=NO_WORD)&&(ref[iref]!=NO_WORD)&& !different){
        if(hip[ihip]==ref[iref]){
	 iref++;
 	 ihip++;  
	}else{
	  different=TRUE;
	}
    }
    if((hip[ihip]!=NO_WORD)||(ref[iref]!=NO_WORD)) different=TRUE;
    return different;
     
 }
   
 
/*********************************************************************************/
int main (int argc, char *argv[]) {

 
  int option,error,i,j,k,c,r;
  int test=FALSE,logf=FALSE,reference=FALSE,noprediction=FALSE,hyp=FALSE;
  int z=FALSE;
  int calcwsr=FALSE,calcwer=FALSE,calcnclics=FALSE,multimodality=FALSE,transparent=FALSE;
  int ECAlg=0;
  int entropy=FALSE;
  int verbosity=FALSE;
  int opciones=FALSE;
//  int *int_final=NULL;
  char *fichlog=NULL, *fichwg=NULL, *fichref=NULL,*path_online=NULL, *unigram_file=NULL, *fichhyp=NULL;
 // char *path=NULL; 
  FILE *ofd=stdout;
  int ref_word_length;
  int prefix_position;
  //int position=-1; 
  int *int_ref=NULL, *int_completion=NULL;
  int prefix_long=0;
  char *actual_prefijo=NULL;
  dict_t *wg_dict;
  completions_t completions;
  int ws_count=0, w_count=0, ks_count=0, k_count=0;
  int clics_count=0;
  //ec_state_t *set_ec_state;
  float GSF=MAXFLOAT;
  float WIP=MAXFLOAT; 
  float ECF=-100;
  float w_beam=MAXFLOAT;  
  int maxnClicks=1;
  int nClicks=0;
  int old_prefix_position=-1;
  int wroteLastWord=FALSE;
  int t_cat=1;
  vector_t *first_word_last_suffix; 
//  ssize_t l;
//  char *correction=NULL;
//  char **tok_correction;
//  char *final=NULL;
  //int first_time=TRUE;
  ec_state_t *curr_ec_state;
  int pot=1;
  char *operaciones=NULL;
  unsigned char errorCorrecting;
  wg_t *WG=NULL;
  wg_t *WG_online=NULL;  
  wg_t *unigram=NULL;
  float facEntropy=1;
  
  alignment_t *alignment0;
  int showalignments=0;
  


  alignment0=NULL;
    
  completions.completions=NULL;
  completions.alignments=NULL;
  tipo_CAT=TRANSCRIPCION;
  disable_pospro=FALSE;

  while ((option=getopt(argc,argv,"aC:c:deg:hl:m:n:p:st:v:w:y:o:b:z:O:NE:DV"))!=-1){
      opciones=TRUE;
      switch(option)
      {
          case 'a':
              transparent=TRUE;
              break;
	  case 'w':
	      test=TRUE;
              fichwg=optarg;
	      break;
	  case 'y':
	      hyp=TRUE;
              fichhyp=optarg;
	      break;    
	  case 't':
	      reference=TRUE;
              fichref=optarg;
	      break;
	  case 'l':
	      logf=TRUE;
              fichlog=optarg;
	      break;
          case 'd':
              disable_pospro=TRUE;
              break;
          case 'e':
              calcwer=TRUE;
              break;
          case 's':
              calcwsr=TRUE;
              break;
          case 'o':
              multimodality=TRUE;
              path_online=optarg;
              break;
          case 'g':
              GSF=atof(optarg);
              break;
	  case 'p':
              WIP=atof(optarg);
              break;
	  case 'c':
              ECF=atof(optarg);
              break;
	  case 'C':
              ECAlg=atoi(optarg);
              break;
	  case 'b':
              pot=atoi(optarg);
              break;
          case 'v':
	      w_beam=atof(optarg);
	      break;
          case 'n':
              calcnclics=TRUE;
	      maxnClicks=atoi(optarg);
	      break;
          case 'm':
              t_cat=atoi(optarg);
	      if(t_cat==1) tipo_CAT=TRANSCRIPCION;
	      if(t_cat==2) tipo_CAT=TRADUCCION;
              break;
          case 'z':
              z=TRUE;
              unigram_file=optarg;
              break;
	  case 'O':
	      operaciones=optarg;
	      break;
	  case 'N':
	      noprediction=TRUE;
	      break;
          case 'A':
              showalignments=TRUE;
	      break;
	  case 'E':
	      entropy=TRUE;
	      facEntropy=atof(optarg);
	      break;
	  case 'D':
	      facEntropy=LOG(0.0);
	      break;
	  case 'V':
	      verbosity=TRUE;
          case 'h':
	      usage(argv[0]);
	      break;
	  default:
	      usage(argv[0]);
      }
   }  
   
/********************************************************************/   
      if(!opciones) usage(argv[0]);
      if (transparent) {
        if (calcnclics) {
          fprintf(stderr,"\nIncompatible options: -a && -n\n");fflush(stderr);
          usage(argv[0]);
          exit(EXIT_FAILURE);
        }
        else calcnclics=TRUE;
      }
/********************************************************************/

   /*The file to keep the log is open to write*/
   if(logf){
      if ( (ofd=fopen(fichlog,"a")) == NULL ) {
	    fprintf(stderr,"\nError opening file  %s\n",fichlog);fflush(stderr);
	    exit(EXIT_FAILURE);
        }
   }
/**********************************************************************/
  
  /*The file of reference names is open to read*/
  if(reference){
     if(!(error = ipeLoadReference(fichref))) exit(EXIT_FAILURE);
  }else{
    if(!entropy){
     fprintf(stderr,"You have to introduce a list of references\n");fflush(stderr);
     usage(argv[0]);
    }else reference=NULL;
   
  }

/*******************************************************************/
  
  /*The file of word_graph names is open to read*/
  if(test){
     if(!(error = ipeLoadwordgraphs(fichwg))) exit(EXIT_FAILURE);
  }
  else{
    if((hyp) && (calcwer)){
     if(!(error = ipeLoadwordgraphs(fichhyp))) exit(EXIT_FAILURE);
    }
    else{
     fprintf(stderr,"You have to introduce a list of wordgraphs \n");fflush(stderr);
     usage(argv[0]);
    }
  } 

/*********************************************************************/
/*Character level, the file with the unigram is open to read*/
if(z) { 
   if(!(error = set_input_unigram(unigram_file,&unigram)))  exit(EXIT_FAILURE);
}
 
 /*********************************************************************/

if(verbosity){
  if(GSF!=MAXFLOAT)   fprintf(ofd,"\n GSF: %f",GSF);
  fflush(ofd);

  if(WIP!=MAXFLOAT)   fprintf(ofd,"\n WIP: %f",WIP);
  fflush(ofd);

  if(calcnclics==TRUE)  fprintf(ofd,"\n NCLICS: %d",maxnClicks);
  fflush(ofd);

  fprintf(ofd,"\n ECF: %f",ECF);fflush(ofd);
  fprintf(ofd,"\n pot: %d",pot);fflush(ofd);

  if(w_beam!=MAXFLOAT)   fprintf(ofd,"\n W_beam: %f",w_beam);
  fflush(ofd);

  fprintf(ofd,"\n operaciones %s",operaciones);
  fflush(ofd);

  if(noprediction==TRUE) fprintf(ofd,"\n No predictions when only one error", w_beam);
  fflush(ofd);
  
  if(ECAlg>0) fprintf(ofd,"\n The Error Correcting algorith is %d", ECAlg);
  fflush(ofd);

  fprintf(ofd,"\n ####################################### \n");
  fflush(ofd);
}

/********************************************************************/

/*Calculo del WER */

 if (calcwer){
 
    int ref_character_length=0;
    int completion_word_length=0;
    int **matrix;
    int **maword;
    int count,wcount;
    int nks_count=0;
    int minval=0;
    int mink=0;

    ipeSetMaxCompletions(BEST_COMPLETION);

    i=0;
    while(wordgraphs[i]!=NULL){

       if(!hyp) if(!(error = set_input_sentence(wordgraphs[i],GSF,WIP,ECF,&WG,ECAlg)))  exit(EXIT_FAILURE);
       if(!(error = ipeSetTargetText(references[i]))) exit(EXIT_FAILURE);

       first_word_last_suffix = create_vector(1);
       if(!hyp){ if(!(error = ipeProvideCompletions(&completions,0,w_beam,first_word_last_suffix,FALSE,pot,&errorCorrecting,WG,unigram,ECAlg))) exit(EXIT_FAILURE);}
       else {
	  j=0;
          if (completions.completions != NULL){
            while(completions.completions[j]!=NULL){
               free(completions.completions[j]);
               j++;
            }
            free(completions.completions);
         }
         completions.completions=NULL;
	 mt((completions.completions) = (char **) realloc((completions.completions),(i+1)*sizeof(char*))); 
	 completions.completions[0]=strdup2(wordgraphs[i]); 
	 completions.completions[1]=NULL;
	 
	 /*Como lo que se da es una hipotesis y no un wg no hay alineamiento*/
	 if (completions.alignments != NULL){
            while(completions.alignments[c]!=NULL){
            free(completions.alignments[c]);
            c++;
            }
            free(completions.alignments);
         }
         completions.alignments=NULL;
     
       }
       
       
       if (!disable_pospro) posproceso(references[i],references[i]);

       fprintf(ofd,"%s & %s\n",references[i],completions.completions[0]);  
       fflush(ofd);

       if(z){
          ref_character_length = strlen(references[i]);

          ref_word_length = nwords(references[i]);
          completion_word_length = nwords(completions.completions[0]);
      
          matrix=(int **)malloc(sizeof(int *)*(completion_word_length+1));
          for(k=0;k<completion_word_length+1;k++) matrix[k]=(int *)malloc(sizeof(int)*(ref_word_length+1));

          maword=(int **)malloc(sizeof(int *)*(completion_word_length+1));
          for(k=0;k<completion_word_length+1;k++) maword[k]=(int *)malloc(sizeof(int)*(ref_word_length+1));

          if(test){ wg_dict=WG->dict;}
	  else if(hyp){ wg_dict=unigram->dict;}
	  
          if(int_ref!=NULL) free(int_ref);
          int_ref=str2seq(wg_dict,references[i]);

          if(int_completion!=NULL) free(int_completion);
          int_completion=str2seq(wg_dict,completions.completions[0]);

          matrix[0][0]=0;
          maword[0][0]=0;

          for(k=1;k<=completion_word_length;k++) {
               matrix[k][0]=matrix[k-1][0]+strlen(get_token_dict(wg_dict,int_completion[k-1]));
               maword[k][0]=k;
          }

          for(j=1;j<=ref_word_length;j++) {
               matrix[0][j]=matrix[0][j-1]+cost(int_ref[j-1],NO_WORD,wg_dict,unigram->dict);              
               maword[0][j]=j;  
          }

          for(j=1;j<=ref_word_length;j++){
             for(k=1;k<=completion_word_length;k++) {
                 if(int_ref[j-1]==int_completion[k-1]) { count=0; wcount=0;}
                 else {count=cost(int_ref[j-1],int_completion[k-1],wg_dict,unigram->dict); wcount=1;  }
                 
                 matrix[k][j]=matrix[k-1][j]+strlen(get_token_dict(wg_dict,int_completion[k-1]));  // deletion
                 maword[k][j]=maword[k-1][j]+1;

                 if(matrix[k][j] > (matrix[k][j-1]+cost(int_ref[j-1],NO_WORD,wg_dict,unigram->dict))){  // insertion
                        matrix[k][j]= matrix[k][j-1]+cost(int_ref[j-1],NO_WORD,wg_dict,unigram->dict);
                        maword[k][j]=maword[k][j-1]+1;
                 } 

                 if(matrix[k][j] > (matrix[k-1][j-1]+ count)){ // substitution
                        matrix[k][j]= matrix[k-1][j-1]+ count;
                        maword[k][j]=maword[k-1][j-1]+wcount;
		}
  
             }
           }

           /*Busco el minimo de la última columna para saber cual es el coste sin contar las palabras extras*/ 
	   minval=matrix[completion_word_length][ref_word_length];
	   mink=completion_word_length;
	   for(k=1;k<=completion_word_length;k++){
	    if(minval > matrix[k][ref_word_length]){
	      minval=matrix[k][ref_word_length]; 
	      mink=k;
	    };
	   };
	   	   
	   nks_count+=matrix[mink][ref_word_length];
           ks_count+=matrix[completion_word_length][ref_word_length];
           ws_count+=maword[completion_word_length][ref_word_length];
           k_count+=ref_character_length;
           w_count+=ref_word_length;

           for(k=0;k<completion_word_length+1;k++) free(matrix[k]);
           free(matrix);
      }

       i++;
       destroy_vector(first_word_last_suffix,1,0); 
    }
    
   c=0;
   if (completions.completions != NULL){
        while(completions.completions[c]!=NULL){
           free(completions.completions[c]);
           c++;
        }
        free(completions.completions);
   }

   if (completions.alignments != NULL){
        while(completions.alignments[c]!=NULL){
           free(completions.alignments[c]);
           c++;
        }
        free(completions.alignments);
   }
   
   curr_ec_state = get_ec_state();
   destroy_ec_state(&curr_ec_state);
   destroy_nbestsuffix();
   destroy_word_graph(&WG);
   destroy_word_graph(&WG_online);
   destroy_word_graph(&unigram);
    
   if(z){
     fprintf(ofd,"TOTAL WORDS: %d \n",w_count);
     fprintf(ofd,"TOTAL WORDSTROKES: %d \n",ws_count);   
     fprintf(ofd,"TOTAL WER: %.4f \n",(ws_count/(float) w_count)*100); 
     fprintf(ofd,"TOTAL CHARACTERS: %d \n",k_count);
     fprintf(ofd,"TOTAL KEYSTROKES: %d \n",ks_count);   
     fprintf(ofd,"TOTAL KEY STROKE RATIO: %.4f \n",(ks_count/(float) k_count)*100); 
     fprintf(ofd,"TOTAL KEY STROKE RATIO SIN CONTAR FINAL: %.4f \n", (nks_count/(float) k_count)*100); 
   }
   

 }/* End of if(calcwer) */
 

/**********************************************************************/
/*CALCULO WSR*/ 

 if(calcwsr){
  
    char* ant;
    char* pal_error;
    int online=FALSE;
    int ws_online=0;
    int ref_character_length=0;
    char* ref_word=NULL;
    char* hip_word=NULL;
    int prefix_character_position=0;
    int k=0,c=0,p=0,r=0;
    int ninserciones=0, nborrados=0, nintercambios=0, nuniones=0, nseparaciones=0, nclicks=0;
    char *concatenada=NULL;
    int *int_concatenada=NULL;
    int completion_position=0;
    int d=TRUE;
    int totalErrorCorrecting=0; 
    

    
    
    old_prefix_position=-1;
    ipeSetMaxCompletions(BEST_COMPLETION);

    i=0;   
    while(wordgraphs[i]!=NULL){
 
       fprintf(ofd,"\n SENTENCE:  %s\n",wordgraphs[i]);fflush(ofd);
       fprintf(ofd," REFERENCE: %s\n\n",references[i]);fflush(ofd);

       first_word_last_suffix = create_vector(1); 
       if(!(error = set_input_sentence(wordgraphs[i],GSF,WIP,ECF,&WG,ECAlg)))  exit(EXIT_FAILURE);
       if(!(error = ipeSetTargetText(references[i]))) exit(EXIT_FAILURE);
       
       /* Count number of words  */
       ref_word_length = nwords(references[i]); 

        /* Count number of characters */
       ref_character_length = strlen(references[i]);

       /*Convert the reference in an array of integers */
       wg_dict=WG->dict;
       int_ref=str2seq(wg_dict,references[i]);
    
       completion_position=0;
       prefix_position=0;
       d=TRUE;
       while(prefix_position <= ref_word_length ){
            /*The number of characters in the prefix*/
            prefix_long=0;  
            for(j=0;j<prefix_position;j++){
              prefix_long=prefix_long+strlen(get_token_dict(wg_dict,int_ref[j]))+1; 
            }  
            prefix_long=prefix_long+prefix_character_position;

            if(actual_prefijo!=NULL) free(actual_prefijo);
            actual_prefijo=strndup2(references[i],prefix_long);
	    
//	    if((prefix_position == ref_word_length) && (actual_prefijo[prefix_long-1]=='\0')) actual_prefijo[prefix_long-1]=" ";
	            
            /*para asegurarme que no accedo a una posicion que no existe*/
            if(prefix_long==0) p=0;
            else p=prefix_long-1;  
            /*Si ha nivel de caracteres se introduce un espacio en blanco, es como si se introduce una nueva palabra por lo tanto prefix_position++*/
            if((z) && ((actual_prefijo[p]==' ') || (prefix_long>=ref_character_length)) ){
                   prefix_position++; 
                   prefix_character_position=0;
            }
            fprintf(ofd,"\n PREFIX:    %s\n",actual_prefijo);fflush(ofd); 
   
	    if(int_completion!=NULL && int_ref!=NULL)  
	      d=isDifferent(int_ref+prefix_position,int_completion+(completion_position+1));
	    
	    if(noprediction==TRUE && !d){
	      
		if(prefix_position){  
		  fprintf(ofd," SUFFIX:   ");
		  for(k=0;k<prefix_long;k++) 
		      fprintf(ofd," ");fflush(ofd);
		  fprintf(ofd," %s\n",references[i]+prefix_long);
		}	
		else  
		  fprintf(ofd," SUFFIX:    %s\n",references[i]+prefix_long);
		fflush(ofd); 
		prefix_position=ref_word_length+1;
		completion_position=nwords(completions.completions[0]);
        
	    }else{
		
		/*Me guardo el alignment antiguo, para recuperar la parte del prefijo*/
		k=0;
		if(completions.alignments!=NULL){
		    for(r=old_prefix_position;r<prefix_position-1;r++){
		      mt(alignment0=(alignment_t *) realloc(alignment0,(r+1)*sizeof(alignment_t)));
		      alignment0[r].start=completions.alignments[0][k].start;
		      alignment0[r].end=completions.alignments[0][k].end;
		      k++;
		    }
		}
			
		/* Compute the posible sufix*/
		if(!(error = ipeProvideCompletions(&completions,prefix_long,w_beam,first_word_last_suffix,online,pot,&errorCorrecting,WG,unigram,ECAlg))) exit(EXIT_FAILURE);
	    
	    
		if(completions.completions[0]!=NULL){
               
		  if(prefix_position){  
		    fprintf(ofd," SUFFIX:   ");
		    for(k=0;k<prefix_long;k++) 
                       fprintf(ofd," ");fflush(ofd);
		    fprintf(ofd," %s\n",completions.completions[0]+prefix_character_position);
		  }	
		  else
		    fprintf(ofd," SUFFIX:    %s\n",completions.completions[0]+prefix_character_position);
		  fflush(ofd); 
		  
		  if(showalignments==TRUE){
		    if(verbosity){fprintf(ofd,"ALIGNMENTS:");}
		  
		    if((completions.alignments[0]!=NULL)&&(prefix_position>0)){
		    /*Primero añado el alineamiento de la palabra añadida por el usuario*/
		      mt(alignment0=(alignment_t *) realloc(alignment0,(prefix_position)*sizeof(alignment_t)));
		      if(prefix_position==1) alignment0[prefix_position-1].start=0.0;
		      else alignment0[prefix_position-1].start=alignment0[prefix_position-2].end;
		      alignment0[prefix_position-1].end=completions.alignments[0][0].start;
		    }		  
		   if(verbosity){ 
		     if(alignment0!=NULL){
		      /*lo escribo*/
			for(r=0;r<prefix_position;r++){
			  fprintf(ofd,"%f %f,",alignment0[r].start,alignment0[r].end); 
			  fflush(ofd);
			}
		      }
		  
		      if(completions.alignments[0]!=NULL){
			k=0;
			while(completions.alignments[0][k].start!=-1){
			  fprintf(ofd,"%f %f,",completions.alignments[0][k].start,completions.alignments[0][k].end); 
			  fflush(ofd);
			  k++;
			}
			fprintf(ofd,"\n");
		      }
		      fflush(ofd);
		    }
		  }

		  if(int_completion!=NULL) free(int_completion);
		  int_completion=str2seq(wg_dict,completions.completions[0]);
 
		   /*Compare the sufix with the reference*/	
		  old_prefix_position=prefix_position;
		  completion_position=0;
		  while((int_completion[completion_position]!=NO_WORD)&&(int_ref[prefix_position]!=NO_WORD)&&(int_completion[completion_position]==int_ref[prefix_position])){
		    completion_position++;
		    prefix_position++;  
		  }
	      
		  if((operaciones!=NULL) && (int_completion[completion_position]!=NO_WORD)&&(int_ref[prefix_position]!=NO_WORD)){ //operaciones
		      if((strchr(operaciones, 't') !=NULL) && (int_ref[prefix_position+1]!=NO_WORD) && (int_ref[prefix_position+1]==int_completion[completion_position])  && (int_completion[completion_position+1]!=NO_WORD) && (int_ref[prefix_position]==int_completion[completion_position+1])){ //Intercambio
			nintercambios++;
			prefix_position++;
			completion_position++;
		      }else{
		      if((strchr(operaciones, 'i') !=NULL) && (int_ref[prefix_position+1]!=NO_WORD) && (int_ref[prefix_position+1]==int_completion[completion_position]) ){ //Insercion
			  ninserciones++;
			  prefix_position++;
		      } else{
			  if((strchr(operaciones, 'b') !=NULL) && (int_completion[completion_position+1]!=NO_WORD) && (int_ref[prefix_position]==int_completion[completion_position+1])){ //Borrado
			    completion_position++;
			    nborrados++;
			  }else{
			    //Union
			    if((strchr(operaciones, 'u') !=NULL) &&  (int_completion[completion_position+1]!=NO_WORD)){ 
			      concatenada=(char*)malloc(sizeof(char)*strlen(get_token_dict(wg_dict,int_completion[completion_position]))+strlen(get_token_dict(wg_dict,int_completion[completion_position+1]))+1);			  
			      strcpy(concatenada,get_token_dict(wg_dict,int_completion[completion_position]));
			      //concatenada=(char*)realloc(concatenada,sizeof(char)*strlen(get_token_dict(wg_dict,int_completion[j+1]))+1);
			      strcat(concatenada,get_token_dict(wg_dict,int_completion[completion_position+1]));
			      int_concatenada=str2seq(wg_dict,concatenada);
			      free(concatenada);  
			    }
			    if((strchr(operaciones, 'u')!=NULL) && (int_ref[prefix_position]==int_concatenada[0])){
			       nuniones++;
			       completion_position++;  
			    }else{ //separacion
			       if((strchr(operaciones, 's') !=NULL) &&  (int_ref[prefix_position+1]!=NO_WORD)){
				concatenada=(char*)malloc(sizeof(char)*strlen(get_token_dict(wg_dict,int_ref[prefix_position]))+strlen(get_token_dict(wg_dict,int_ref[prefix_position+1]))+1);
				strcpy(concatenada,get_token_dict(wg_dict,int_ref[prefix_position]));
				//concatenada=(char*)realloc(concatenada,sizeof(char)*strlen(get_token_dict(wg_dict,int_ref[prefix_position+1]))+2);
				strcat(concatenada,get_token_dict(wg_dict,int_ref[prefix_position+1]));
				int_concatenada=str2seq(wg_dict,concatenada);
				free(concatenada);
			       }
			       if((strchr(operaciones, 's')!=NULL) && (int_completion[completion_position]==int_concatenada[0])){
				  nseparaciones++;
				  prefix_position++;
			       }else{//rejects
				  if((strchr(operaciones, 'r') !=NULL) && (old_prefix_position < prefix_position)){
				    destroy_vector(first_word_last_suffix,1,0);
				    first_word_last_suffix =create_vector(1);
				    nclicks++;
				    mt(first_word_last_suffix->element = (void **) realloc(first_word_last_suffix->element, (first_word_last_suffix->size+1)*sizeof(void *)));
				    first_word_last_suffix->size++;
				    first_word_last_suffix->element[first_word_last_suffix->size-1] = (void *) int_completion[completion_position] ;
				    prefix_position--; 
				    ws_count--;
				  }
								
			      }
			    }
			}
		      }
		    }
		  }
	       
		  /*The user write a word to correct the erroneous word that follow the validated prefix*/
		  if((multimodality) && (prefix_position < ref_word_length) && (old_prefix_position!=prefix_position || online==FALSE)){
		      if(prefix_position==0) ant="<s>";    
		      else ant=get_token_dict(wg_dict,int_ref[prefix_position-1]);
		      if(get_token_dict(wg_dict,int_completion[completion_position])==NULL) pal_error="";  
		      else pal_error=get_token_dict(wg_dict,int_completion[completion_position]);
		      set_online_word(path_online,ant,get_token_dict(wg_dict,int_ref[prefix_position]),pal_error,&WG_online);
		      online=TRUE;
		      fprintf(ofd,"online ant= %s,pal= %s,err=%s\n",ant,get_token_dict(wg_dict,int_ref[prefix_position]),pal_error);
		      ws_online++;
		  }else {
		      /* Si estamos trabajando a nivel de caracter, el usuario no introduce la palabra entera, solo un caracter*/
		      if((z) && (prefix_position < ref_word_length) && (prefix_long < ref_character_length) ){
			ref_word=get_token_dict(wg_dict,int_ref[prefix_position]);
			hip_word=get_token_dict(wg_dict,int_completion[completion_position]);
			k=0;
			prefix_character_position=0;
			if(hip_word!=NULL){
			    while((ref_word[prefix_character_position]!='\0') && (hip_word[k]!='\0') && (ref_word[prefix_character_position]==hip_word[k])){
				k++;
				prefix_character_position++;
			    }
			}
			prefix_character_position++;
			ks_count++; 
		      }
		      else{
			prefix_position++;
			online=FALSE;
		      }
		  }
		}
		else{
		    fprintf(stdout,"\n");fflush(stdout);
		}
	        if((prefix_position<=ref_word_length) &&(!z)) ws_count++; 
		c=0;
	    }
       }     

       /*If the number of words in the hipotesi is bigger than in the reference*/ 
       if(int_completion[completion_position]!=NO_WORD){ 
           fprintf(ofd,"\n PREFIX:    %s </s>\n",references[i]);fflush(ofd);
           fprintf(ofd," SUFFIX:    \n");
	   fflush(ofd);
	   ws_count++; 
           ks_count++;
       }

       fprintf(ofd,"\n TRANSCRIPTION: %s\n",references[i]);
       prefix_character_position=0;
       destroy_vector(first_word_last_suffix,1,0);
       if(int_completion!=NULL) free(int_completion);
       int_completion=NULL;
       if(int_ref!=NULL) free(int_ref);
       int_ref=NULL;
       if(actual_prefijo!=NULL) free(actual_prefijo);
       actual_prefijo=NULL;
       c=0;
       while(completions.completions[c]!=NULL){
	  free(completions.completions[c]); 			
	  c++;
       }
       free(completions.completions);
       completions.completions=NULL;
       
       c=0;
       while(completions.alignments[c]!=NULL){
	  free(completions.alignments[c]); 			
	  c++;
       }
       free(completions.alignments);
       completions.alignments=NULL;

       if(alignment0!=NULL) free(alignment0);
       alignment0=NULL;
       
       
       
       w_count += ref_word_length;
       k_count += ref_character_length;
       i++; 
       totalErrorCorrecting=totalErrorCorrecting+errorCorrecting;
       fprintf(ofd,"\n ####################################### \n");
       fflush(ofd);
    } 
     
    fprintf(ofd,"TOTAL WORDS: %d \n",w_count);
    fprintf(ofd,"TOTAL CHARACTERS: %d \n\n",k_count); 
    
    fprintf(ofd,"TOTAL WORD STROKE: %d \n",ws_count);
    fprintf(ofd,"TOTAL WORD STROKE RATIO: %.4f \n\n",(ws_count/(float) w_count)*100); 
    
    fprintf(ofd,"TOTAL INSERTIONS: %d \n", ninserciones);
    fprintf(ofd,"TOTAL WORD INSERTION RATIO: %.4f \n",(ninserciones/(float) w_count)*100); 
    fprintf(ofd,"TOTAL DELETIONS: %d \n", nborrados);
    fprintf(ofd,"TOTAL WORD DELETION RATIO: %.4f \n",(nborrados/(float) w_count)*100); 
    fprintf(ofd,"TOTAL EXCHANGES: %d \n", nintercambios);
    fprintf(ofd,"TOTAL WORD EXCHANGES RATIO: %.4f \n",(nintercambios/(float) w_count)*100); 
    fprintf(ofd,"TOTAL UNIONS: %d \n", nuniones);
    fprintf(ofd,"TOTAL WORD UNIONS RATIO: %.4f \n",(nuniones/(float) w_count)*100); 
    fprintf(ofd,"TOTAL SEPARATIONS: %d \n", nseparaciones);
    fprintf(ofd,"TOTAL WORD SEPARATIONS RATIO: %.4f \n", (nseparaciones/(float) w_count)*100);
    fprintf(ofd,"TOTAL REJECTS: %d \n", nclicks);
    fprintf(ofd,"TOTAL WORD REJECTS RATIO: %.4f \n", (nclicks/(float) w_count)*100);
    
    fprintf(ofd,"TOTAL KEY STROKE: %d \n",ks_count);   
    fprintf(ofd,"TOTAL KEY STROKE RATIO: %.4f \n\n",(ks_count/(float) k_count)*100); 
    
    fprintf(ofd,"TOTAL WORD ONLINE: %d \n",ws_online);
    fprintf(ofd,"TOTAL ONLINE ERROR: %.4f \n\n",((w_count-ws_online)/(float) ws_online)*100); 
    fflush(ofd);

    fprintf(ofd,"TOTAL ERROR CORRECTING: %d \n",totalErrorCorrecting);
    fprintf(ofd,"TOTAL ERROR CORRECTING RATIO: %.4f \n\n",(totalErrorCorrecting/(float) w_count)*100); 

    destroy_nbestsuffix();
    destroy_word_graph(&WG);
    destroy_word_graph(&WG_online);
    destroy_word_graph(&unigram);
}/* End of if(calcwsr) */
/****************************************************************************/


  if(calcnclics){
    int prefix_character_position=0;
    int old_prefix_character_position=0;
    int p=0;
    int ref_character_length=0;
    char* ref_word=NULL;
    char* hip_word=NULL;
    int completion_position=0;
  //  int old_prefix_long=0;
    
   
    ipeSetMaxCompletions(BEST_COMPLETION);

    i=0; 
    while(wordgraphs[i]!=NULL){ 
       int actTotalClicks=0, actWordStrokes=0;
       
       fprintf(ofd,"\n SENTENCE:  %s\n",wordgraphs[i]);fflush(ofd);
       fprintf(ofd," REFERENCE: %s\n\n",references[i]);fflush(ofd);

       first_word_last_suffix = create_vector(1);
       if(!(error = set_input_sentence(wordgraphs[i],GSF,WIP,ECF,&WG,ECAlg)))  exit(EXIT_FAILURE);
       if(!(error = ipeSetTargetText(references[i]))) exit(EXIT_FAILURE);
       
       /* Count number of words  */
       ref_word_length = nwords(references[i]); 

       
       /* Count number of characters */
       ref_character_length = strlen(references[i]);
       
       /*Convert the reference in an array of integers */
       wg_dict=WG->dict;
       int_ref=str2seq(wg_dict,references[i]);
 
       completion_position=0;
       old_prefix_position=-1;  
       prefix_position=0;
       old_prefix_character_position=0;
       prefix_character_position=0;
       nClicks=0;
       wroteLastWord=TRUE;
       while(prefix_position <= ref_word_length ){
         /* If the user made click and the prefix is the same than in the previous step */
         if((old_prefix_position!=prefix_position) || (old_prefix_character_position!=prefix_character_position)){ /*The prefix has some new word*/
            /*The number of characters in the prefix */
            prefix_long=0;
            for(j=0;j<prefix_position;j++){
              prefix_long += strlen(get_token_dict(wg_dict,int_ref[j]))+1; 
            }
            prefix_long=prefix_long+prefix_character_position;
            if(actual_prefijo!=NULL) free(actual_prefijo);
	    
	    //if (prefix_long>0) actual_prefijo=strndup2(references[i],prefix_long-1);
            //else actual_prefijo=strndup2(references[i],prefix_long);
	    
	    actual_prefijo=strndup2(references[i],prefix_long);
	    
	    /*para asegurarme que no accedo a una posicion que no existe*/
	    if(prefix_long==0) p=0;
            else p=prefix_long-1;  
            /*Si ha nivel de caracteres se introduce un espacio en blanco, es como si se introduce una nueva palabra por lo tanto prefix_position++*/
            if((z) && ((actual_prefijo[p]==' ') || (prefix_long>=ref_character_length)) ){
                   prefix_position++; 
                   prefix_character_position=0;
            }
         }
         fprintf(ofd,"\n PREFIX:    %s\n",actual_prefijo);
         fflush(ofd);
	 	 
	 
	 k=0;
	if(completions.alignments!=NULL){
	    for(r=old_prefix_position;r<prefix_position-1;r++){
	      mt(alignment0=(alignment_t *) realloc(alignment0,(r+1)*sizeof(alignment_t)));
	      alignment0[r].start=completions.alignments[0][k].start;
	      alignment0[r].end=completions.alignments[0][k].end;
	      k++;
	    }
	}
	 
	 
	 
	 /* Compute the posible suffix*/
         if(!(error = ipeProvideCompletions(&completions,prefix_long,w_beam,first_word_last_suffix,FALSE,pot,&errorCorrecting,WG,unigram,ECAlg))) exit(EXIT_FAILURE);
         
         if(completions.completions[0]!=NULL){
       
            if(prefix_position){  
              fprintf(ofd," SUFFIX:   ");
              for(k=0;k<prefix_long;k++) 
                   fprintf(ofd," ");fflush(ofd);
	      fprintf(ofd," %s\n",completions.completions[0]+prefix_character_position);
	    }	
            else               fprintf(ofd," SUFFIX:    %s\n",completions.completions[0]+prefix_character_position);
	    fflush(ofd);

	    
	    if(verbosity){fprintf(ofd,"ALIGNMENTS:");}
		  
	    if((completions.alignments[0]!=NULL)&&(prefix_position>0)){
		 /*Primero añado el alineamiento de la palabra añadida por el usuario*/
		 mt(alignment0=(alignment_t *) realloc(alignment0,(prefix_position)*sizeof(alignment_t)));
		 if(prefix_position==1) alignment0[prefix_position-1].start=0.0;
		 else alignment0[prefix_position-1].start=alignment0[prefix_position-2].end;
		 alignment0[prefix_position-1].end=completions.alignments[0][0].start;
	    }		  
	    if(verbosity){
	      if(alignment0!=NULL){
	    /*lo escribo*/
		for(r=0;r<prefix_position;r++){
		     fprintf(ofd,"%f %f,",alignment0[r].start,alignment0[r].end); 
		     fflush(ofd);
		}
	      }
		  
	      if(completions.alignments[0]!=NULL){
		k=0;
		while(completions.alignments[0][k].start!=-1){
		  fprintf(ofd,"%f %f,",completions.alignments[0][k].start,completions.alignments[0][k].end); 
		  fflush(ofd);
		  k++;
		}
		fprintf(ofd,"\n");
	      }
	      fflush(ofd);
	    }
	    
	     
	    
	    
	    
            if(int_completion!=NULL) free(int_completion);
            int_completion=str2seq(wg_dict,completions.completions[0]);
 
	    /*Compare the suffix with the reference*/	
            old_prefix_position=prefix_position;
	    completion_position=0;
            while((int_completion[completion_position]!=NO_WORD)&&(int_ref[prefix_position]!=NO_WORD)&&(int_completion[completion_position]==int_ref[prefix_position])){
	       completion_position++;
 	       prefix_position++;
               nClicks=0; 
            }
            
            old_prefix_character_position=prefix_character_position;
            /*If the number of clics is equal to the max number of clics the user write a word to correct the erroneous word that follow the validated prefix*/
            if(nClicks>=maxnClicks || (transparent && wroteLastWord && old_prefix_position == prefix_position && old_prefix_character_position!=prefix_character_position)){
	              
	        /* Si estamos trabajando a nivel de caracter, el usuario no introduce la palabra entera, solo un caracter*/
		      if((z) && (prefix_position < ref_word_length) && (prefix_long < ref_character_length) ){
			ref_word=get_token_dict(wg_dict,int_ref[prefix_position]);
			hip_word=get_token_dict(wg_dict,int_completion[completion_position]);
			k=0;
			prefix_character_position=0;
			if(hip_word!=NULL){
			    while((ref_word[prefix_character_position]!='\0') && (hip_word[k]!='\0') && (ref_word[prefix_character_position]==hip_word[k])){
				k++;
				prefix_character_position++;
			    }
			}
			prefix_character_position++;
		      }
		      else{
			prefix_position++;
		      }
		      nClicks=0;
		      destroy_vector(first_word_last_suffix,1,0);
                      first_word_last_suffix = create_vector(1);
                      if(prefix_position<=ref_word_length) actWordStrokes++;
                      wroteLastWord=TRUE;
            }
            else {
               wroteLastWord=FALSE;
               if(prefix_position<ref_word_length){
		  if((z) && (prefix_position < ref_word_length) && (prefix_long < ref_character_length) ){
			ref_word=get_token_dict(wg_dict,int_ref[prefix_position]);
			hip_word=get_token_dict(wg_dict,int_completion[completion_position]);
			k=0;
			prefix_character_position=0;
			if(hip_word!=NULL){
			    while((ref_word[prefix_character_position]!='\0') && (hip_word[k]!='\0') && (ref_word[prefix_character_position]==hip_word[k])){
				k++;
				prefix_character_position++;
			    }
			}
		      }
		  if(old_prefix_position<prefix_position){
                     destroy_vector(first_word_last_suffix,1,0);
                     first_word_last_suffix = create_vector(1);
                  }
                  else actTotalClicks++;
                  nClicks++;

                  mt(first_word_last_suffix->element = (void **) realloc(first_word_last_suffix->element, (first_word_last_suffix->size+1)*sizeof(void *)));
                  first_word_last_suffix->size++;
                  first_word_last_suffix->element[first_word_last_suffix->size-1] = (void *) int_completion[completion_position] ;
               }
               else prefix_position++; /*If the number of words in the prefix is equal to the number of words in the reference*/
            }
         }
         else{
            fprintf(stdout,"\n");fflush(stdout);
         }
	 

       }     			      
		  

       /*If the number of words in the hipotesis is bigger than in the reference*/ 
       if(int_completion[completion_position]!=NO_WORD){ 
           fprintf(ofd,"\n PREFIX:    %s </s>\n",references[i]);fflush(ofd);
           fprintf(ofd," SUFFIX:    \n");
	   fflush(ofd);
	   actWordStrokes++; 
       }

       fprintf(ofd,"\n TRANSCRIPTION: %s\n",references[i]);
       fprintf(ofd,"\n CLICKS: %d\n WORD STROKES: %d\n", actTotalClicks, actWordStrokes);
       destroy_vector(first_word_last_suffix,1,0);
       if(int_completion!=NULL) free(int_completion);
       int_completion=NULL;
       if(int_ref!=NULL) free(int_ref);
       int_ref=NULL;
       if(actual_prefijo!=NULL) free(actual_prefijo);
       actual_prefijo=NULL;
       c=0;
       while(completions.completions[c]!=NULL){
	  free(completions.completions[c]); 			
	  c++;
       }
       free(completions.completions);
       completions.completions=NULL;

       c=0;
       if(completions.alignments!=NULL){
	while(completions.alignments[c]!=NULL){
	    free(completions.alignments[c]);
	    c++;
	}
	free(completions.alignments);
       }
         
       completions.alignments=NULL;

       if(alignment0!=NULL) free(alignment0);
       alignment0=NULL;
       
       w_count += ref_word_length;
       i++; 
       fprintf(ofd,"\n ####################################### \n");
       fflush(ofd);
       ws_count+=actWordStrokes;
       clics_count+=actTotalClicks;
    }
     
    fprintf(ofd,"TOTAL WORDS: %d \n",w_count);
    fprintf(ofd,"TOTAL WORD STROKES: %d \n",ws_count);
    fprintf(ofd,"TOTAL NUMBER OF CLICKS: %d \n",clics_count);
    fprintf(ofd,"TOTAL WORD STROKE RATIO: %.4f \n",(ws_count/(float) w_count)*100); 
    fflush(ofd);

    curr_ec_state = get_ec_state();
    destroy_ec_state(&curr_ec_state);
    destroy_nbestsuffix();
    destroy_word_graph(&WG);
    destroy_word_graph(&WG_online);
    destroy_word_graph(&unigram);
}/* End of if(calcnclics) */
/****************************************************************************/





if(entropy){
  
    double valorEntropy=0;
    i=0; 
    while(wordgraphs[i]!=NULL){
 
       //fprintf(ofd,"\n SENTENCE:  %s\n",wordgraphs[i]);fflush(ofd);
       if(!(error = set_input_WG(wordgraphs[i],GSF,WIP,&WG)))  exit(EXIT_FAILURE);

       valorEntropy=calcEntropy(WG,facEntropy);
   
       fprintf(ofd,"\n %s  %f\n",wordgraphs[i], valorEntropy); fflush(ofd);
       i++; 
   
   }
   destroy_word_graph(&WG);
  
}/*End of Entropy*/
  
 /****************************************************************************/ 
  

  r=0;
  if(reference!=NULL){
    while(references[r]!=NULL){
	  free(references[r]);
	  r++;
    }
    free(references);
  }
  r=0;
  while(wordgraphs[r]!=NULL){
        free(wordgraphs[r]);
        r++;
  }
  free(wordgraphs);
  
  return EXIT_SUCCESS;
}




