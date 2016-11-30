%{
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "lat-flex.h"


#define MAX_COND_LEVEL 128
#undef  BEGIN
#define BEGIN slf_cond_stack[slf_cond_idx++] = yy_start; (yy_start) = 1 + 2 *
#undef  END 
#define END() (yy_start) = slf_cond_stack[--slf_cond_idx]
int slf_cond_idx = 0;
int slf_cond_stack[MAX_COND_LEVEL];


%}


/* Options */
%option noyywrap nounput case-insensitive
%option prefix="lat_"
/* %option debug */

%x INCOMMENT LONGSTR NODELINE LINKLINE NWSTR

WSCHAR              [ \t\f\v\r]
NOT_WSCHAR          [^ \t\f\v\r\n]
OCTDIGIT            [0-7]
HEXDIGIT            [0-9a-fA-F]
DIGIT               [0-9]

                /* Strings */
ESCAPE              (\\[ntvbrfa\n\\?'"])
IN_STR              ([^\\\n"]|{ESCAPE})*
STRING              \"{IN_STR}\"
CHARLIT             \'{IN_STR}\'

                /* Intergers */
OCTAL               (0{OCTDIGIT}+)
HEXADECIMAL         (0[xX]{HEXDIGIT}+)
UINTEGER            ([1-9]{DIGIT}*)|0
INTEGER             [+-]?{UINTEGER}

                /* Floats */
EXPONENT            [Ee][+-]?{DIGIT}+
FRACTIONAL          ({DIGIT}+\.)|({DIGIT}*\.{DIGIT}+)
FLOAT               [+-]?{FRACTIONAL}{EXPONENT}?|{DIGIT}+{EXPONENT}

                /* Others */
WHITESPACE          {WSCHAR}+|({WSCHAR}*\\)+{WSCHAR}*
NEWLINE             [\n]

IDENTCHARS	    [_/a-zA-Z]
IDENT               {IDENTCHARS}({IDENTCHARS}|[0-9])*

%%

"#".*                   { /* yylloc->lines (1); ylloc->step (); return(COMMENT);*/ }

      /* Header */
("VERSION"|"V"){WHITESPACE}*"="   { BEGIN(LONGSTR); return SLF_VERSION; }
("UTTERANCE"|"U"){WHITESPACE}*"=" { BEGIN(LONGSTR); return UTTERANCE; }
("SUBLAT"|"S"){WHITESPACE}*"="    { BEGIN(LONGSTR); return SUBLAT; }
"base"{WHITESPACE}*"="            { return BASE; }
"lmname"{WHITESPACE}*"="          { BEGIN(LONGSTR); return LMNAME; }
"lmscale"{WHITESPACE}*"="         { return LMSCALE; }
"wdpenalty"{WHITESPACE}*"="       { return WDPENALTY; }
"acscale"{WHITESPACE}*"="         { return ACSCALE; }
"amscale"{WHITESPACE}*"="         { return AMSCALE; }
"start"{WHITESPACE}*"="           { return INITIAL_NODE; }
"end"{WHITESPACE}*"="             { return FINAL_NODE; }
"vocab"{WHITESPACE}*"="           { BEGIN(LONGSTR); return VOCAB; }

<LONGSTR>{
  {IN_STR}     { // Copy and don't remove "" and '' 
                 lat_lval.sval = lat_copy_string(yytext, false); 
                 return STRING; 
               }
  {NEWLINE}    { 
                 END(); 
                 //yylloc->lines (yyleng); yylloc->step (); 
                 return ENDL;
               }
}


      /* Size defs */
("NODES"|"N"){WHITESPACE}*"="   { return NODES; }
("LINKS"|"L"){WHITESPACE}*"="   { return LINKS; }

      /* Nodes */
"I"{WHITESPACE}*"="             { BEGIN(NODELINE); return NODE; }
<NODELINE>{
  ("time"|"t"){WHITESPACE}*"="   { return TIME; }
  ("WORD"|"W"){WHITESPACE}*"="   { BEGIN(NWSTR); return WORD; }
  "L"{WHITESPACE}*"="            { return SUBS; }
  ("var"|"v"){WHITESPACE}*"="    { return VAR; }
  {WHITESPACE}                   { /*yylloc->step();*/ }
  {NEWLINE}                      { 
                                   END(); 
                                   //yylloc->lines (yyleng); yylloc->step (); 
                                   return ENDL;
                                 }
}

      /* Links */
"J"{WHITESPACE}*"="                { BEGIN(LINKLINE); return LINK; }
<LINKLINE>{
  ("START"|"S"){WHITESPACE}*"="    { return START_NODE; }
  ("END"|"E"){WHITESPACE}*"="      { return END_NODE; }
  ("WORD"|"W"){WHITESPACE}*"="     { BEGIN(NWSTR); return WORD; }
  ("div"|"d"){WHITESPACE}*"="      { return DIV; }
  ("acoustic"|"a"){WHITESPACE}*"=" { return ACOUSTIC; }
  ("ngram"|"n"){WHITESPACE}*"="    { return NGRAM; }
  ("language"|"l"){WHITESPACE}*"=" { return LANGUAGE; }
  ("posterior"|"p"){WHITESPACE}*"=" { return POSTERIOR; }
  {WHITESPACE}                     { /*yylloc->step();*/ }
  {NEWLINE}                        { END(); 
                                     //yylloc->lines (yyleng); yylloc->step (); 
                                     return ENDL;
                                   }
}


 
{NEWLINE}       { 
                  //yylloc->lines (yyleng); yylloc->step (); 
                  return ENDL;
                }

{WHITESPACE}    { /*yylloc->step();*/ /*return(SPACE);*/ }

<NWSTR>{
  {WHITESPACE}  { 
                  //yylloc->step(); 
                  END();
                }
  {NEWLINE}     { 
                  //yylloc->lines (yyleng); yylloc->step (); 
                  END();
                  return ENDL;
                }

  {NOT_WSCHAR}* {
                  lat_lval.sval = lat_copy_string(yytext, false); 
                  END();
                  return STRING; 
                }
}


<*>{

  {OCTAL}	{ 
  		  errno = 0;
  	          long n = strtol(yytext+1,NULL,8); 
                      if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
                        //driver.error(*yylloc, "integer is out of range");
                      }
  	          lat_lval.ival = n;
                    return INT;
  		}
  
  {HEXADECIMAL}   {
  	          errno = 0;
  	          long n = strtol(yytext+2,NULL,16); 
                      if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
                        //driver.error(*yylloc, "integer is out of range");
                      }
  	          lat_lval.ival = n;
  	          return INT;
  		}
  
  {INTEGER}       { 
  	          errno = 0;
  	          long n = strtol(yytext,NULL,10); 
                    if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
                      //driver.error(*yylloc, "integer is out of range");
                    }
  	          lat_lval.ival = n;
                    return INT;
  		}
  
  
  {FLOAT}         { 
  	          errno = 0;
  	          double n = strtod(yytext,NULL); 
                    if (! (errno != ERANGE)) {
                      //driver.error(*yylloc, "float is out of range");
                    }
  	          lat_lval.dval = n;
                    return FLOAT;
  		}
}

<INITIAL,NODELINE,LINKLINE>{STRING} |
{CHARLIT}       { // Copy and remove "" and '' 
                  lat_lval.sval = lat_copy_string(yytext, true); 
                  return STRING; 
                }


<*>{IDENT}{WHITESPACE}*"=" { 
                  //lat_lval.sval = lat_copy_string(yytext, false); 
                  BEGIN(NWSTR); 
                  return UNK_OPTION; 
                }

.               {
                  lat_begin_token(yytext);
                  fprintf(stderr, "unexpected character\n");
                  return UNEXPECTED;
                }

%%

