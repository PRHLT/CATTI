/* Jorge Civera <jcivera@dsic.upv.es>, Dec 2006 from Vicente Alabau */
/** \file 
\brief \d Library to read and write normal, .gz and .bz2 files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxiliar.h"
#include "zipio.h"

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/*****************************************************************************/
/** \d Check if file ends in .gz */
int is_gzipped(const char * const fname) {
  return strcmp(fname+(strlen(fname)-3),".gz") == 0;}

/*****************************************************************************/
/** \d Check if file ends in .bz2 */
int is_bzipped2(const char * const fname) {
  return strcmp(fname+(strlen(fname)-4),".bz2") == 0;}

/*****************************************************************************/
/** \d Open normal, gzip or bzip2 file for reading or writing */  
FILE *zopen(char *fname, char *mode) {
  char *cmd; FILE *fp=NULL;

  if (is_gzipped(fname)){
    if (strcmp(mode,"r")==0){
      ft(fp=fopen(fname, mode),fname);fclose(fp);/* Check if file exists */
      mt(cmd=(char *)malloc(sizeof(char)*(strlen("gunzip -c ")+strlen(fname)+1)));
      cmd[0]='\0'; strcat(cmd,"gunzip -c "); strcat(cmd,fname);
      fp=popen(cmd,mode);free(cmd);}
    else if (strcmp(mode,"w")==0){
      mt(cmd=(char *)malloc(sizeof(char)*(strlen("gzip -c > ")+strlen(fname)+1)));
      cmd[0]='\0'; strcat(cmd,"gzip -c > "); strcat(cmd,fname);
      fp=popen(cmd,mode);free(cmd);}      
    else fp=NULL;
    return fp;
  }
  if (is_bzipped2(fname)){
    if (strcmp(mode,"r")==0){
      ft(fp=fopen(fname, mode),fname);fclose(fp);/* Check if file exists */
      mt(cmd=(char *)malloc(sizeof(char)*(strlen("bunzip2 -c ")+strlen(fname)+1)));
      cmd[0]='\0'; strcat(cmd,"bunzip2 -c "); strcat(cmd,fname);
      fp=popen(cmd,mode);free(cmd);}
    else if (strcmp(mode,"w")==0){
      mt(cmd=(char *)malloc(sizeof(char)*(strlen("bzip2 -c > ")+strlen(fname)+1)));
      cmd[0]='\0'; strcat(cmd,"bzip2 -c > "); strcat(cmd,fname);
      fp=popen(cmd,mode);free(cmd);}      
    else fp=NULL;
    return fp;
  }
  ft(fp=fopen(fname, mode),fname);
  return fp;
}

/*****************************************************************************/
/** \d Close normal, gzip or bzip2 file for reading or writing */  
int zclose(FILE *fd){
  int rc = -1;
  
  rc = pclose(fd);
  /* stream not associated with a popen() */
  if (rc == -1) rc = fclose(fd);
  return rc;
}

