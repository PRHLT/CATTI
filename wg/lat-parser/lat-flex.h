#ifndef _LAT_FLEX_H_
#define _LAT_FLEX_H_

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>
#include <float.h>
#include "lat-driver.h"
#include "lat-parser.h"

#define true 1
#define false 0

#define YY_INPUT(buf,result,max_size)  {\
    result = lat_get_next_char(buf, max_size); \
    if (  result <= 0  ) \
      result = YY_NULL; \
    }

/*
 * global variable
 */
extern int debug;

/*
 * lex & parse
 */
extern int lat_lex(void);
extern int lat_parse(wg_t * wg);
extern void lat_error(wg_t * wg,char*);

/*
 * ccalc.c
 */
extern void lat_begin_token(char*);






extern void lat_dump_row(void);
extern int  lat_get_next_char(char *b, int max_buffer);
//extern void begin_token(char*);
extern void lat_print_error(char *s, ...);

/*
 * utilities
 */
char *lat_copy_string(const char *txt, int remove_quotes);









#endif /*_LAT_FLEX_H_*/
