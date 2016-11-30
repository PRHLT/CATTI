/* Jorge Civera <jcivera@dsic.upv.es>, Dec 2006 from Vicente Alabau */
/** \file 
\brief \d Library to read and write normal, .gz and .bz2 files */
#ifndef GZIO_INCLUDED
#define GZIO_INCLUDED

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d Open normal or gzip file for reading or writing */ 
FILE *zopen(char *fname, char *mode);

/** \d Close normal or gzip file for reading or writing */
int zclose(FILE *fd);

#endif
