/*
    Copyright (C) 2012 Verónica Romero, Vicent Alabau

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU  Lesser General Public License as published by
    the Free Software Foundation.

*/



//*****************************************
//
// \file   catti-plugin.cpp
// \author Vicent Alabau
// \author Verónica Romero 
// \brief  CATTI plugin for Transcriptorium 
//
// Copyright (C) 2012
//_________________________________________

#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include <string>
#include <cctype>
#include <algorithm>  
#include <string.h> 
#include <values.h>

#include <casmacat/IImtEngine.h>
#include <casmacat/IPluginFactory.h>
#include <casmacat/utils.h>

extern "C" {
#include "wg/word_graph.h"
#include "wg/CAT.h"
#include "wg/global.h"
#include "wg/librefx.h"
#include "wg/ecparser.h"
#include "wg/nbestsuffix.h"
#include "wg/auxiliar.h"
#include "wg/strop.h"
#include "wg/global.h"
}

#define FALSE 0
#define TRUE 1

using namespace std;
using namespace casmacat;


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


class CattiSession: public IInteractiveMtSession {
public:
  CattiSession() {};
  CattiSession(const string &_wordgraph_fn,wg_t *&_unigram): wordgraph_fn(_wordgraph_fn) {
    
         
    float GSF=MAXFLOAT;
    float WIP=MAXFLOAT; 
    float ECF=-350;
    int ECAlg=0;
    int error;
    char * wg_fn = strdup(wordgraph_fn.c_str());
    //float w_beam=MAXFLOAT;
    completions_t ipe_completions;
    //unsigned char errorCorrecting;
    //ipe_completions.completions=NULL;
    vector<string> correct_suffix;

        
    first_word_last_suffix = create_vector(1);

    this->WG=NULL;
    this->old_prefix=NULL;
   // this->old_first_word_suffix=NULL;
    this->unigram=_unigram;
     
    
//    cerr << "catti-plugin: CattiSession creator\n";  
    
    if(!(error = set_input_sentence(wg_fn,GSF,WIP,ECF,&WG,ECAlg)))  exit(EXIT_FAILURE);
    //this->setPrefix("", CO_NONE, info);
  
    if(!(error = ipeSetTargetText(""))) exit(EXIT_FAILURE);
    
    
    this->setPrefix(vector<string>(), vector<string>(), FALSE, correct_suffix);
    
   
    free(wg_fn);
  
  };
  virtual ~CattiSession() { 
    
     set_ec_state(NULL);
     destroy_nbestsuffix();
     if(WG!=NULL){
	destroy_word_graph(&WG);  
     }
 
    
    unigram=NULL;
  
//    fprintf(stderr, "after unigram=NULL\n");
   
   destroy_vector(first_word_last_suffix,1,0);
 //  if(!old_prefix.empty()){ free(old_prefix)};
     
   cerr << "I, " << typeid(*this).name() <<  ", (CattiSession) am free!!!" << endl; 
    
  };

  /* Set a partial validation for isle-driven, not left-to-rigth interactions */
  virtual void setPartialValidation(const vector<string> &partial_translation,
                                    const vector<bool> &validated,
                                          vector<string> &corrected_translation,
                                          vector<bool> &corrected_validated)
  {
    cerr << "WARNING: 'setPartialValidation' not implemented!!!\n";
  }

  /* Set prefix */
  virtual void setPrefix(const vector<string> &prefix,
                         const vector<string> &suffix,
                         const bool last_token_is_partial,
                               vector<string> &corrected_suffix)
  {
     
  //   cerr << "catti-plugin: setPrefix\n";  
    completions_t completions;
    //char *prefix_sp = NULL;
    int error;
    //dict_t *wg_dict;
    float w_beam=MAXFLOAT;
    int ECAlg=0;
    int pot=1;
    unsigned char errorCorrecting;
    //wg_t *unigram=NULL;
    char *n_prefix=NULL;
   
    
    destroy_vector(first_word_last_suffix,1,0);
    first_word_last_suffix = create_vector(1); 
    
    completions.completions=NULL;  
    completions.alignments=NULL;
    
    corrected_suffix.clear();
    if(prefix.size()==0){ 
      mt(n_prefix = (char *) malloc(sizeof(char)*1));
      n_prefix[0]='\0';               
    }
    else{
       n_prefix=strdup(prefix[0].c_str());
       if((prefix.size()>1)||(!last_token_is_partial)){ 
	 corrected_suffix.push_back(prefix[0].c_str());
       }
       for(int i=1;i<prefix.size();i++) {
	   mt(n_prefix=(char *) realloc(n_prefix,sizeof(char)*(strlen(n_prefix)+prefix[i].size()+2)));
	   strcat(n_prefix," ");
	   strcat(n_prefix,prefix[i].c_str());
	   if((i<prefix.size()-1)||(!last_token_is_partial))   corrected_suffix.push_back(prefix[i].c_str());
       }
     }
   
    if(!last_token_is_partial){
      mt(n_prefix=(char *) realloc(n_prefix,sizeof(char)*(strlen(n_prefix)+2)));
      strcat(n_prefix, " ");
    }
   
    //cerr << "catti-plugin: n_prefix: '" << n_prefix << "'\n";
    
     /* Set the maximum number of N-best transcriptions */
    ipeSetMaxCompletions(1);

  
    if(!(error = ipeSetTargetText(n_prefix))) exit(EXIT_FAILURE);
  
    if(!(error = ipeProvideCompletions(&completions,strlen(n_prefix),w_beam,first_word_last_suffix,FALSE,pot,&errorCorrecting,this->WG,unigram,ECAlg)))  exit(EXIT_FAILURE);
   // if(!(error = ipeProvideCompletions(&completions,strlen(n_prefix),w_beam,first_word_last_suffix,FALSE,pot,&errorCorrecting,this->WG,this->WG,ECAlg)))  exit(EXIT_FAILURE);

    if(completions.completions[0]!=NULL){
      // cerr << "catti-plugin: completions.completions[0] '" << completions.completions[0] << "'\n";
       char **completionTok=tokenize(completions.completions[0]);
       int i=0;
       while(completionTok[i]!=NULL){
            corrected_suffix.push_back(completionTok[i]);
	    i++;
	}
//       for(i=0;i<corrected_suffix.size();i++)  cerr << "catti-plugin: suffix: " << corrected_suffix[i] << "\n";
       i=0;
     }
       
    
    int c=0;
    if (completions.completions != NULL){
        while(completions.completions[c]!=NULL){
           free(completions.completions[c]);
           c++;
        }
        free(completions.completions);
     }
     completions.completions =NULL;
    c=0;
    if(completions.alignments !=NULL){
	while(completions.alignments[c]!=NULL){
	  free(completions.alignments[c]); 			
	  c++;
       }
       free(completions.alignments);
    }
    completions.alignments=NULL;   
     
  
     if(n_prefix!=NULL) free(n_prefix); 
 //    if(prefix_sp!=NULL) free(prefix_sp); 
     
    
    int i=0;
    old_corrected_suffix.clear();
    for(i=0;i<corrected_suffix.size();i++)  old_corrected_suffix.push_back(corrected_suffix[i]);
       
           
       
  }

  virtual void rejectSuffix(const vector<string> &prefix,
                            const vector<string> &suffix,
                            const bool last_token_is_partial,
                                  vector<string> &corrected_suffix)
  {
    completions_t completions;
    //char *prefix_sp = NULL;
    int error;
    //int pref_word_length,suf_word_length,i;
    //char *word=NULL; 
    dict_t *wg_dict;
    //int *int_completion = NULL;
    char *n_prefix=NULL;
    float w_beam=MAXFLOAT;
    int ECAlg=0;
    int pot=1;
    unsigned char errorCorrecting;
    //int *int_trans = NULL;
    //int *int_suffix = NULL;
    
    
    
    completions.completions=NULL; 
    completions.alignments=NULL;
    
    corrected_suffix.clear();
    if(prefix.size()==0){ 
      mt(n_prefix = (char *) malloc(sizeof(char)*1));
      n_prefix[0]='\0';   
    }
    else{
       n_prefix=strdup(prefix[0].c_str());
       if((prefix.size()>1)||(!last_token_is_partial)){ 
	 corrected_suffix.push_back(prefix[0].c_str());
       }
       for(int i=1;i<prefix.size();i++) {
	   mt(n_prefix=(char *) realloc(n_prefix,sizeof(char)*(strlen(n_prefix)+prefix[i].size()+2)));
	   strcat(n_prefix," ");
	   strcat(n_prefix,prefix[i].c_str());
	   if((i<prefix.size()-1)||(!last_token_is_partial)) {
	      corrected_suffix.push_back(prefix[i].c_str());
	   }
       }
     }
         
     if(!last_token_is_partial){
       mt(n_prefix=(char *) realloc(n_prefix,sizeof(char)*(strlen(n_prefix)+2)));
       strcat(n_prefix, " ");
     }
      
//    cerr << "catti-plugin: n_prefix: '" << n_prefix << "'\n";
    
    /* Set the maximum number of N-best transcriptions */
    ipeSetMaxCompletions(1);
 
    if(!(error = ipeSetTargetText(n_prefix))) exit(EXIT_FAILURE);
    
//     if(prefix.size()>0){
//       if(!old_prefix.empty()){
// 	if(old_prefix.compare(prefix[0])!=0){
// 	  destroy_vector(first_word_last_suffix,1,0);
// 	  first_word_last_suffix = create_vector(1);
// 	}
// 	old_prefix.clear();
//       }
//       old_prefix.assign(prefix[0]);
//     }
    
    
    
    if(old_prefix!=NULL){
       if( strcmp(old_prefix,n_prefix)!=0) {
          destroy_vector(first_word_last_suffix,1,0);
 	  first_word_last_suffix = create_vector(1);
        }
        free(old_prefix);
    }
    old_prefix = (char *) malloc(sizeof(char)*strlen(n_prefix)+1);
    strcpy(old_prefix,n_prefix);
    
  //  cerr << "catti-plugin: old_prefix: '" << old_prefix << "'\n";
    
    int i=0;
    
 
    /*Obtain de dictionary of the word_graph */
    wg_dict=this->WG->dict;

    

    int int_rejectedWord; 
    
    if(last_token_is_partial){
      char *rejectedWord = strdup(prefix[prefix.size()-1].c_str());
    //  cerr << "catti-plugin: rejectedWord '" << rejectedWord << "'\n"; 
     // if(old_first_word_suffix!=NULL){ //Comprobamos si es una palabra completa y por lo tanto no cambia.
      
      if( ((suffix.size()>0) && (old_corrected_suffix.size()>0) && (old_corrected_suffix.size() == prefix.size())) ||((suffix.size()>0) && (old_corrected_suffix.size()>0) && (old_corrected_suffix.size() > prefix.size()) && (old_corrected_suffix[prefix.size()].compare(suffix[0])!=0))){ 
	  mt(rejectedWord=(char *) realloc(rejectedWord,sizeof(char)*(strlen(rejectedWord)+strlen(suffix[0].c_str())+1)));
	  strcat(rejectedWord,suffix[0].c_str());
     }
      //}
      set_token_dict(unigram->dict,rejectedWord);
      int_rejectedWord=set_token_dict(wg_dict,rejectedWord); 
      //cerr << "catti-plugin: rejectedWord '" << rejectedWord << "'\n"; 
	
    //  old_first_word_suffix = (char *) malloc(sizeof(char)*strlen(suffix[0].c_str())+1);
    //  strcpy(old_first_word_suffix,suffix[0].c_str());
    }
    else{
      if(suffix.size()>0){
	int_rejectedWord=set_token_dict(wg_dict,(char *)suffix[0].c_str());
	//cerr << "catti-plugin: suffix[0]'" << suffix[0] << "'\n"; 
      }else{
	int_rejectedWord=get_id_dict(wg_dict,"</s>");
      }
    }
    
//    cerr << "catti-plugin: int_rejectWord '" << int_rejectedWord << "'\n";

    /*The rejected word is added in first_word_last_suffix */
    mt(first_word_last_suffix->element = (void **) realloc(first_word_last_suffix->element, (first_word_last_suffix->size+1)*sizeof(void *)));
    first_word_last_suffix->size++;
    first_word_last_suffix->element[first_word_last_suffix->size-1] = (void *) int_rejectedWord; 
 
    
    if(completions.completions!=NULL){cerr << "catti-plugin: completions.completions[0] before'" << completions.completions[0] << "'\n"; }
/*    cerr << "catti-plugin: strlen(n_prefix)'" << strlen(n_prefix) << "'\n"; 
    cerr << "catti-plugin: w_beam'" << w_beam << "'\n";
    cerr << "first_word_last_suffix'" << first_word_last_suffix->element[0] << "'\n";
    cerr << "get_token_dict(wg_dict,first_word_last_suffix->element[0])'" << get_token_dict(wg_dict, int_rejectedWord) << "'\n";
    cerr << "catti-plugin: pot'" << pot << "'\n";
    cerr << "catti-plugin: EAClg'" << ECAlg << "'\n";*/
    
   /* Compute the posible sufix*/
    if(!(error = ipeProvideCompletions(&completions,strlen(n_prefix),w_beam,first_word_last_suffix,FALSE,pot,&errorCorrecting,this->WG,unigram,ECAlg)))  exit(EXIT_FAILURE);
//    if(!(error = ipeProvideCompletions(&completions,strlen(n_prefix),w_beam,first_word_last_suffix,FALSE,pot,&errorCorrecting,this->WG,this->WG,ECAlg)))  exit(EXIT_FAILURE);
       
         
    if(completions.completions[0]!=NULL){
       cerr << "catti-plugin: completions.completions[0] '" << completions.completions[0] << "'\n"; 
       char **completionTok=tokenize(completions.completions[0]);
       int i=0;
       while(completionTok[i]!=NULL){
            corrected_suffix.push_back(completionTok[i]);
	    i++;
	}
       for(i=0;i<corrected_suffix.size();i++)  cerr << "catti-plugin: suffix: " << corrected_suffix[i] << "\n";
       i=0;
     }
       
    
    int c=0;
    if (completions.completions != NULL){
        while(completions.completions[c]!=NULL){
           free(completions.completions[c]);
           c++;
        }
        free(completions.completions);
     }
     completions.completions=NULL; 
      c=0;
     if(completions.alignments !=NULL){
	while(completions.alignments[c]!=NULL){
	  free(completions.alignments[c]); 			
	  c++;
       }
       free(completions.alignments);
     }
     completions.alignments=NULL;   
     
  
     if(n_prefix!=NULL) free(n_prefix); 
 //    if(prefix_sp!=NULL) free(prefix_sp); 
    // if(unigram!=NULL) destroy_word_graph(&unigram);
    
    //int i=0;
    old_corrected_suffix.clear();
    for(i=0;i<corrected_suffix.size();i++)  old_corrected_suffix.push_back(corrected_suffix[i]);
   
    
  }

private:
  const string wordgraph_fn;
  wg_t *WG;
  //string old_prefix;
  vector<string> old_corrected_suffix;
  char *old_prefix;
  vector_t *first_word_last_suffix;
  wg_t *unigram;
    
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

class CattiEngine: public IInteractiveMtEngine {
public:
  std::map<std::string,std::string> WGsPath;
  wg_t *unigram;
  
  CattiEngine(const std::map<std::string,std::string> &_WGsPath_fn, wg_t *&_unigram): WGsPath(_WGsPath_fn) {
    unigram=_unigram;
  }
 
  CattiEngine() {};
  
  virtual ~CattiEngine() { 
    
    unigram=NULL;
    cerr << "I, " << typeid(*this).name() <<  ", (CattiEngine) am free!!!" << endl;
    
  };

  /* Update catti models will not be implemented */
  virtual void validate(const vector<string> &source,
                        const vector<string> &target,
                        const vector<bool> &validated)
  {
    cerr << "WARNING: 'validate' not implemented!!!\n";
  }

  /* Return the most likely hypothesis for WG stored in source[0] */
  virtual void translate(const std::vector<std::string> &source,
                               std::vector<std::string> &target)
  {
    //XXX: Implement this!
    // for (size_t i = 0; i < len; i++) target.push_back(result[i]);
      
  //   cerr << "catti-plugin: translate\n";  
     
     float GSF=MAXFLOAT;
     float WIP=MAXFLOAT; 
     float ECF=-350;
     float w_beam=MAXFLOAT;
     wg_t *WG=NULL;
     completions_t ipe_completions;
     int error;
     vector_t *first_word_last_suffix = create_vector(1);
     int ECAlg=0;
     int pot=1;
     unsigned char errorCorrecting;
    // wg_t *unigram=NULL;
     char *wg_fn =NULL;
     ipe_completions.completions=NULL;
     ipe_completions.alignments=NULL;
    
     if(source.empty()){
        cerr << "The id is empty\n";
	exit(EXIT_FAILURE);
     }
       
     
     if (WGsPath.count(source[0])>0){
	wg_fn = strdup(WGsPath[source[0]].c_str()); 
        cerr << "catti-plugin: WGsPath: " << WGsPath[source[0]] << "\n";
     }
     else{ 
        cerr << "The id: " << source[0] << "is not found\n";
	exit(EXIT_FAILURE);
     }
     //printf("setting source: '%s'\n", source[0]); 

     if(!(error = set_input_sentence(wg_fn,GSF,WIP,ECF,&WG,ECAlg)))  exit(EXIT_FAILURE);
  
     if(!(error = ipeSetTargetText(""))) exit(EXIT_FAILURE);
          
     if(!(error = ipeProvideCompletions(&ipe_completions,0,w_beam,first_word_last_suffix,FALSE,pot,&errorCorrecting,WG,unigram,ECAlg)))  exit(EXIT_FAILURE);
   
     if(ipe_completions.completions[0]!=NULL){
       target.push_back(ipe_completions.completions[0]);
    //   cerr << "catti-plugin: target: " << target[0] << "\n";
     }
           
     int c=0;
     if (ipe_completions.completions != NULL){
        while(ipe_completions.completions[c]!=NULL){
           free(ipe_completions.completions[c]);
           c++;
        }
        free(ipe_completions.completions);
     }
     ipe_completions.completions=NULL; 
     c=0;
     if(ipe_completions.alignments !=NULL){
	while(ipe_completions.alignments[c]!=NULL){
	  free(ipe_completions.alignments[c]); 			
	  c++;
       }
       free(ipe_completions.alignments);
     }
     ipe_completions.alignments=NULL;   
  
     free(wg_fn);
     set_ec_state(NULL);
    
     destroy_nbestsuffix();
     if(WG!=NULL){
	destroy_word_graph(&WG);  
     }
     destroy_vector(first_word_last_suffix,1,0);
    
  }


  /* Update catti models will not be implemented */
  virtual void update(const std::vector<std::string> &source,
                      const std::vector<std::string> &target)
  {
    cerr << "WARNING: 'update' not implemented!!!\n";
  }

  /**
   * initialize Catti session
   */
  virtual IInteractiveMtSession *newSession(const vector<string> &source) {
     if (WGsPath.count(source[0])>0)
        return new CattiSession(WGsPath[source[0]],unigram);
     else 
        cerr << "The id: '" << source[0] << "is not found\n";
  }

  /**
   * delete Catti session
   */
  virtual void deleteSession(IInteractiveMtSession *session) {
    delete session;
  }
 
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


class CattiFactory: public IInteractiveMtFactory {
public:
  std::map<std::string,std::string> WGs;
  //std::string unigramFile;
  wg_t *unigram;
  
  CattiFactory() { unigram=NULL; }
  // do not forget to free all allocated resources
  // otherwise define the destructor with an empty body
  virtual ~CattiFactory() {
    if(unigram!=NULL) destroy_word_graph(&unigram);
    cerr << "I, " << typeid(*this).name() <<  ", am free!!!" << endl; 
    
  }

  /**
   * initialize the Catti engine with main-like parameters
   */
  virtual int init(int argc, char *argv[], Context *context = 0) {
    //XXX: Implement this!
   if (argc != 2) { // invalid number of arguments
      return EXIT_FAILURE;
    }

    // initialize structures with parameters from args
    
    string pathWG;
    string unigramFile;
    string conf(argv[1]);
       
    std::string line;
    ifstream FileConf(conf.c_str(), fstream::in); 
    
    if(FileConf.fail()) {
      cerr << "Error al abrir el archivo" << endl;
      return EXIT_FAILURE;
    }
    else
    {
      while(FileConf.good() and not FileConf.eof())
      {
	  getline(FileConf,line);
	  if (line.size() > 0) {
	      char * linestr = strdup(line.c_str());
	      char **lineTok=tokenize(linestr);
	      if (lineTok != NULL and lineTok[0] != NULL and lineTok[1] != NULL) {
		//std::string tok0(lineTok[0]);
	        if(strcmp(lineTok[0],"-W")==0) pathWG = lineTok[1];
		if(strcmp(lineTok[0],"-G")==0) unigramFile = lineTok[1];
	      }
	      else {
		  cerr << "Wrong Configure line: '" << line << "'\n";
	      }
	      free(linestr);
	     /* int i=0;
	      if(lineTok !=NULL){
		while(lineTok[i]!=NULL){
		  free(lineTok[i]);
		  i++;
		}
		free(lineTok);
	      }*/
	      
	  }  
      }
    }
    FileConf.close();
    
    cerr << "WG file: '" << pathWG.c_str() << "'" << endl;
    cerr << "Unigram: '" << unigramFile.c_str() << "'" << endl;
    
    ifstream fileWG(pathWG.c_str(), fstream::in); 
    
    if(fileWG.fail()) {
      cerr << "Error al abrir el archivo de WGs" << endl;
      return EXIT_FAILURE;
    }
    else
    {
      while(fileWG.good() and not fileWG.eof())
      {
	  getline(fileWG,line);
	  if (line.size() > 0) {
	      char * linestr = strdup(line.c_str());
	      char **lineTok=tokenize(linestr);
	      if (lineTok != NULL and lineTok[0] != NULL and lineTok[1] != NULL) {
		  WGs[lineTok[0]]=lineTok[1];
	      }
	      else {
		  cerr << "Wrong WG line: '" << line << "'\n";
	      }
	      free(linestr);
	      int i=0;
   
	  }  
      }
    }
    fileWG.close();

    cerr << WGs.size() << " WGs loaded\n";
    
    disable_pospro=TRUE;
            
    unigram=NULL;
    int error=0;
    if(!(error = set_input_unigram((char *)unigramFile.c_str(),&unigram))) exit(EXIT_FAILURE); 
   
    
    return EXIT_SUCCESS;
  }

  virtual string getVersion() { return "Catti Plugin v0.1"; }


  virtual IInteractiveMtEngine *createInstance(const std::string &specialization_id = "") {
    return new CattiEngine(WGs,unigram);
  }

  virtual void deleteInstance(IInteractiveMtEngine *instance) {
    delete instance;
  }

};

EXPORT_CASMACAT_PLUGIN(IInteractiveMtEngine, CattiFactory);

