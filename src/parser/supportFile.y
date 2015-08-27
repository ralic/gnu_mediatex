/*=======================================================================
 * Version: $Id: supportFile.y,v 1.6 2015/08/27 10:51:53 nroche Exp $
 * Project: MediaTeX
 * Module : support parser
 *
 * support file parser
 
 MediaTex is an Electronic Records Management System
 Copyright (C) 2014 2015 Nicolas Roche
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 =======================================================================*/

/* prologue: ===========================================================*/

%code provides {
/*=======================================================================
 * Version: this file is generated by BISON using supportFile.y
 * Project: MediaTeX
 * Module : support parser
 *
 * support file parser

 MediaTex is an Electronic Records Management System
 Copyright (C) 2014 2015 Nicolas Roche
   
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 =======================================================================*/

#define YYSTYPE SUPP_STYPE
}

%{  
#include "mediatex-config.h"
  /* yyscan_t is pre-defined by mediatex-config.h and will be defined
     by Flex header bellow */
%}
 
%union {
  off_t  size;
  time_t time;
  char   hash[MAX_SIZE_HASH+1];
  char   status[MAX_SIZE_STAT+1];
  char   name[MAX_SIZE_STRING+1];
}

%{
  // upper YYSTYPE union is required by Flex headers
#include "parser/supportFile.h"
 
#define LINENO supp_get_lineno(yyscanner)

void supp_error(yyscan_t yyscanner, const char* message);
%}

/* declarations: =======================================================*/
%defines "parser/supportFile.tab.h"
%output "parser/supportFile.tab.c"
%define api.prefix {supp_}
%define api.pure full
%param {yyscan_t yyscanner}
%define parse.error verbose
%verbose
%debug

%start file

%token <time>   suppDATE
%token <hash>   suppHASH
%token <size>   suppSIZE
%token <status> suppSTATUS
%token <name>   suppNAME
%token          suppERROR

%%
 /* grammar rules: ======================================================*/

file: lines
    | //empty file
{
  logParser(LOG_WARNING, "the supports file was empty");
}
;

lines: lines line
     | line
;

line: suppDATE suppDATE suppDATE suppHASH suppHASH suppSIZE suppSTATUS suppNAME
{
  Support *support = 0;
  struct tm date1;
  struct tm date2;
  struct tm date3;

  if (localtime_r(&$1, &date1) == (struct tm*)0 ||
      localtime_r(&$2, &date2) == (struct tm*)0 ||
      localtime_r(&$3, &date3) == (struct tm*)0) {
    logParser(LOG_ERR, "%s", "localtime_r returns on error");
    YYABORT;
  }

  logParser(LOG_DEBUG, "line %-3i: %04i-%02i-%02i,%02i:%02i:%02i "
	    "%04i-%02i-%02i,%02i:%02i:%02i %04i-%02i-%02i,%02i:%02i:%02i "
	    "%*s %*s %*lli %*s %s",
	    LINENO,
	    date1.tm_year + 1900, date1.tm_mon+1, date1.tm_mday,
	    date1.tm_hour, date1.tm_min, date1.tm_sec,
	    date2.tm_year + 1900, date2.tm_mon+1, date2.tm_mday,
	    date2.tm_hour, date2.tm_min, date2.tm_sec,
	    date3.tm_year + 1900, date3.tm_mon+1, date3.tm_mday,
	    date3.tm_hour, date3.tm_min, date3.tm_sec,
	    MAX_SIZE_HASH, $4, MAX_SIZE_HASH, $5,
	    MAX_SIZE_SIZE, (long long int)$6, 
	    MAX_SIZE_STAT, $7, $8);

  if ((support = addSupport($8)) == 0) YYERROR;
  support->firstSeen = $1;
  support->lastCheck = $2;
  support->lastSeen = $3;
  strncpy(support->quickHash, $4, MAX_SIZE_HASH);
  strncpy(support->fullHash, $5, MAX_SIZE_HASH);
  support->size = $6;
  strncpy(support->status, $7, MAX_SIZE_STAT);
  strncpy(support->name, $8, MAX_SIZE_STRING);
  support = 0;
}
;

%%
      /* epilogue: =====================================================*/


/*=======================================================================
 * Function   : supportFile_error
 * Description: Emit an error message. Called by the parser on error
 *              state.
 * Synopsis   : void supportFile_error(char* message)
 * Input      : char* message = the error message.
 * Output     : N/A
 =======================================================================*/
void 
supp_error(yyscan_t yyscanner, const char* message)
{
  logParser(LOG_ERR, "%s on token '%s' line %i\n", message, 
	    supp_get_text(yyscanner), LINENO);
}

/*=======================================================================
 * Function   : parseLocalSuppTree
 * Description: Parse the support File
 * Synopsis   : int parseSupports(const char* path)
 * Input      : const char* path: the support file path
 * Output     : TRUE on success
=======================================================================*/
int 
parseSupports(const char* path)
{
  int rc = FALSE;
  FILE* inputStream = stdin;
  yyscan_t scanner;

  logParser(LOG_INFO, "parse supports from %s",
	    path?path:"stdin");

  // initialise scanner
  if (supp_lex_init(&scanner)) {
    logParser(LOG_ERR, "%s", "error initializing scanner");
    goto error;
  }

  if (path != 0) {
    if ((inputStream = fopen(path, "r")) == 0) {
      logParser(LOG_ERR, "cannot open input stream: %s", path); 
      goto error;
    }
    if (!lock(fileno(inputStream), F_RDLCK)) goto error2;
  }

  supp_set_in(inputStream, scanner);

  // debug mode for scanner
  supp_set_debug(env.debugLexer, scanner);
  logParser(LOG_DEBUG, "supp_set_debug = %i", supp_get_debug(scanner));

  // call the parser
  if (supp_parse(scanner)) {
    logParser(LOG_ERR, "support parser fails on line %i", 
	    supp_get_lineno(scanner));
    logParser(LOG_ERR, "please edit %s", path?path:"stdin");
    goto error3;
  }

  rc = TRUE;
 error3:
  if (inputStream != stdin) {
    if (!unLock(fileno(inputStream))) rc = FALSE;
  }
 error2:
  if (inputStream != stdin) {
    fclose(inputStream);
  }
 error:
  if (!rc) {
    logParser(LOG_ERR, "%s", "support parser fails");
  }
  supp_lex_destroy(scanner);
  return rc;
}

/* Local Variables: */
/* mode: c */
/* mode: font-lock */
/* mode: auto-fill */
/* End: */
