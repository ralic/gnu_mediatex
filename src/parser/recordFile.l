/*=======================================================================
 * Version: $Id: recordFile.l,v 1.2 2015/07/03 11:39:04 nroche Exp $
 * Project: Mediatex
 * Module : record scanner

 * record file scanner

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

%top{
/*=======================================================================
* Version: this file is generated by LEX using recordList.l
 * Project: Mediatex
 * Module : record scanner

 * record file scanner

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

#include "mediatex.h" // cypher.h

typedef struct RecordExtra {
  AESData*       aesData;
  int            isBodyCyphered;
} RecordExtra;
}

%{
#include "mediatex-config.h"
#include "parser/recordFile.tab.h"

#define YY_INPUT(buf, result, max_size)					\
  aesInput(((RecordExtra*)yyextra)->aesData, buf, (int*)&result, max_size)
  
%}

/* definitions: ========================================================*/

/* Options for Specifing Filenames: */
%option outfile="parser/recordFile.c"
%option header-file="parser/recordFile.h"

/* Options Affecting Scanner Behavior: */
%option case-insensitive
%option nolex-compat
/* option batch OU interactive */
%option batch
%option 8bit
%option noposix-compat
%option nostdinit
%option noyywrap
%option yylineno

/* Code-Level And API Options: */
%option ansi-definitions
%option ansi-prototypes
%option reentrant 
%option bison-bridge
%option noc++
%option pointer
%option prefix="record_"
%option nomain
%option unistd

/* Options for Scanner Speed and Size: */
%option noalign
%option ecs
%option nometa-ecs
%option noread

/* Debugging Options: */
%option nobackup
%option debug
%option noperf-report
%option warn

/* Miscellaneous Options: */
%option nounput
%option noinput

 /* scaner states */
%x COMMENT
%s STRING
%s CYPHER
%x STATE1
%x STATE2
%x STATE3
%x STATE4

 /* No way to use constants from a c header file */

YEAR    [[:digit:]]{4}
MONTH   [[:digit:]]{2}
DAY     [[:digit:]]{2}
HOUR    [[:digit:]]{2}
MIN     [[:digit:]]{2}
SEC     [[:digit:]]{2}
DATE    {YEAR}-{MONTH}-{DAY},{HOUR}:{MIN}:{SEC}

 /* MAX_SIZE_HASH = 32 */
HASH    [[:xdigit:]]{32}

 /* MAX_SIZE_STRING = 511 */
STRING  [^[:blank:]\r\n]{1,511}

%%
 /* rules: ==========================================================*/
   /* first rule: */

<INITIAL>{

  \/\* {
    BEGIN(COMMENT);
  }

  (\n|\r\n) 

  headers {
    return(recordHEADER);
  }

  collection {
    BEGIN(STRING);
    return(recordCOLL);
  }

  docypher {
    BEGIN(CYPHER);
  }

  server {
    BEGIN(STRING);
    return(recordSERVER);
  }

  type {
    return(recordMSGTYPE);
  }

  disk {
    yylval->msgval = DISK;
    return(recordMSGVAL);
  }

  cgi {
    yylval->msgval = CGI;
    return(recordMSGVAL);
  }

  have {
    yylval->msgval = HAVE;
    return(recordMSGVAL);
  }

  notify {
    yylval->msgval = NOTIFY;
    return(recordMSGVAL);
  }

  upload {
    yylval->msgval = UPLOAD;
    return(recordMSGVAL);
  }

  unknown {
    yylval->msgval = UNKNOWN;
    return(recordMSGVAL);
  }

  body {
    BEGIN(STATE1);
    ((RecordExtra*)yyextra)->aesData->doCypher =
      ((RecordExtra*)yyextra)->isBodyCyphered;
    return(recordBODY);
  }
}

<STRING>{

  {STRING} { 
    BEGIN(INITIAL);
    strcpy(yylval->string, yytext);
    return(recordSTRING);
  }
 }

<CYPHER>{
  
  [[:blank:]]+    /* : eat up any blank character */

  false { 
    BEGIN(INITIAL);
    ((RecordExtra*)yyextra)->isBodyCyphered = FALSE;
    return(recordDOCYPHER);
  }

  true { 
    BEGIN(INITIAL);
    ((RecordExtra*)yyextra)->isBodyCyphered = TRUE;
    return(recordDOCYPHER);
  }
 }

<STATE1>{

  (\n|\r\n) {
    ++yylineno;
  }

  s {
    yylval->type = SUPPLY;
    return(recordTYPE);
  }
  
  d {
    yylval->type = DEMAND;
    return(recordTYPE);
  }  
  
  {DATE} {
    // date conversion into time_t
    struct tm date;
    memset(&date, 0, sizeof(struct tm));
    if (sscanf(yytext, "%d-%d-%d,%d:%d:%d",
	       &date.tm_year, &date.tm_mon, &date.tm_mday,
	       &date.tm_hour, &date.tm_min, &date.tm_sec)
	!= 6) {
      logEmit(LOG_ERR, "sscanf: error parsing date %s", yytext);
      return(recordERROR);
    }
    date.tm_year -= 1900; // from GNU/Linux burning date
    date.tm_mon -= 1;     // month are managed from 0 to 11 
    date.tm_isdst = -1; // no information available about spring horodatage
    if ((yylval->time = mktime(&date)) == -1) {
      logEmit(LOG_ERR, "%s", "mktime: error parsing date");
      return(recordERROR);
    }
    return(recordDATE);
  }

  {HASH} {
    BEGIN(STATE2);
    strcpy(yylval->hash, yytext);
    return(recordHASH);
  }
 }

<STATE2>{
  
  {HASH} {
    BEGIN(STATE3);
    strcpy(yylval->hash, yytext);
    return(recordHASH);
  }
 }

<STATE3>{
  
  [[:digit:]]+ {
    BEGIN(STATE4);
    sscanf(yytext, "%llu", (unsigned long long int*)&yylval->size);
    return(recordSIZE);
  }
 }  

<STATE4>{

  {STRING} { 
    strcpy(yylval->string, yytext);
    return(recordPATH);
  }

  (\n|\r\n) {
    BEGIN(STATE1);
    ++yylineno;
  }
 }
   
 /* <INITIAL,STATE2,STATE3,STATE4>{ */
<INITIAL,STRING,STATE1,STATE2,STATE3,STATE4>{
  [[:blank:]]+    /* : eat up any blank character */
    ^#.*$         /* : eat up comments */
    \/\/.*$       /* : eat up comments */
 }

<COMMENT>{
  [^\n\r]  /* : eat up any unmatched character */
  
  \*\/ {
    BEGIN(INITIAL);
  }
 }
   
<*>{
  . { /* : eat up any unmatched character and 
	 let bison manage the errors */
    logEmit(LOG_WARNING, "line %i: scanner read unexpected caractere '%s'", 
	    yylineno, yytext);
  }
 }
   
%%
   /* user code (literally copied to the generated source) :
      ==========*/

/* Local Variables: */
/* mode: c */
/* mode: font-lock */
/* mode: auto-fill */
/* End: */
