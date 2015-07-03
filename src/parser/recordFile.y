/*=======================================================================
 * Version: $Id: recordFile.y,v 1.2 2015/07/02 12:14:08 nroche Exp $
 * Project: MediaTeX
 * Module : record parser
 *
 * record file parser

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
 * Version: this file is generated by BISON using recordFile.y
 * Project: MediaTeX
 * Module : recordList parser
 *
 * recordList parser

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

#define YYSTYPE RECORD_STYPE
}

%{  
#include "mediatex-config.h"
  /* yyscan_t is pre-defined by mediatex-config.h and will be defined
     by Flex header bellow */
%}

%union {
  Type    type;
  time_t  time;
  off_t   size;
  int     msgval;
  char    string[MAX_SIZE_STRING+1];
  char    hash[MAX_SIZE_HASH+1];
  Record* record;
}

%{
  // upper YYSTYPE union is required by Flex headers
#include "parser/recordFile.h"

#define LINENO record_get_lineno(yyscanner)
  
void record_error(yyscan_t yyscanner, RecordTree* recordTree,
		  const char* message);
%}

/* declarations: ===================================================*/
%defines "parser/recordFile.tab.h"
%output "parser/recordFile.tab.c"
%define api.prefix {record_}
%define api.pure full
%param {yyscan_t yyscanner}
%parse-param {RecordTree* recordTree}
%define parse.error verbose
%verbose
%debug

%start file

%token          recordHEADER
%token          recordBODY
%token          recordCOLL
%token          recordDOCYPHER
%token          recordSERVER
%token          recordMSGTYPE
%token <string> recordSTRING
%token          recordERROR

%token <type>   recordTYPE
%token <time>   recordDATE
%token <hash>   recordHASH
%token <size>   recordSIZE
%token <string> recordPATH
%token <msgval> recordMSGVAL

%type <record>  line

%%

/* grammar rules: ==================================================*/

file: //empty file 
{
  logParser(LOG_WARNING, "%s", "the records file was empty");
} 
    | header
    | header lines
; 

header: recordHEADER hLines recordBODY
{
  // enabling encryption for body if needed is done by scanner
}
;

hLines: hLines hLine
     | hLine

hLine: recordCOLL recordSTRING
{
  Collection* coll = 0;
  if (!(coll = getCollection($2))) {
    logParser(LOG_WARNING, "unknown collection: %s", $2);
    YYABORT;
  }
  recordTree->collection = coll;

  // load the collection key
  if (!loadCollection(coll, SERV)) YYERROR;
  if (!aesInit(&recordTree->aes, coll->serverTree->aesKey, DECRYPT)) 
    YYERROR;
  if (!releaseCollection(coll, SERV)) YYERROR;
}
     | recordSERVER recordSTRING
{
  strncpy(recordTree->fingerPrint, $2, MAX_SIZE_HASH);
}
     | recordMSGTYPE recordMSGVAL
{
    recordTree->messageType = $2;
}
    | recordDOCYPHER // manage by scanner
;

lines: lines newLine
     | newLine

newLine: line
{
  logParser(LOG_NOTICE, "%s", "add new record");

  struct tm date;
  if (localtime_r(&$1->date, &date) == (struct tm*)0) {
    logParser(LOG_ERR, "%s", "localtime_r returns on error");
    YYABORT;
  }

  logParser(LOG_NOTICE, "line %i: %c "
	    "%04i-%02i-%02i,%02i:%02i:%02i "
	    "%*s %*s %*lli %s", 
	    LINENO, 
	    ($1->type & 0x3) == DEMAND?'D':
	    ($1->type & 0x3) == SUPPLY?'S':'?',
	    date.tm_year + 1900, date.tm_mon+1, date.tm_mday,
	    date.tm_hour, date.tm_min, date.tm_sec,
	    MAX_SIZE_HASH, $1->server->fingerPrint, 
	    MAX_SIZE_HASH, $1->archive->hash, 
	    MAX_SIZE_SIZE, (long long int)$1->archive->size, 
	    $1->extra?$1->extra:"");
 
  if (!rgInsert(recordTree->records, $1)) YYERROR;
  // RQ: YYABORT will close the socket, do YYERROR close it ?
  $1 = 0;

}
;

line: recordTYPE recordDATE recordHASH recordHASH recordSIZE recordPATH
{
  Server* server = 0;
  Archive* archive = 0;
  char* extra = 0;
  logParser(LOG_NOTICE, "%s", "new record");

  /* maybe we should refuse unknown servers ?
  if (!(server = getServer(recordTree->collection, $3))) {
    logEmit(LOG_ERR, "unknown server: %s", $3);
    YYERROR;
  }
  */
  if (!(server = addServer(recordTree->collection, $3))) {
    logEmit(LOG_ERR, "unknown server: %s", $3);
    YYERROR;
  }

  if (!(archive = addArchive(recordTree->collection, $4, $5))) 
    YYERROR;

  if (!(extra = createString($6))) YYERROR;
  if (!($$ = 
	addRecord(recordTree->collection, server, archive, $1, extra)))
    YYABORT;
  $$->date = $2;
}

%%
      /* epilogue: =====================================================*/


/*=======================================================================
 * Function   : yyerror
 * Description: Emit an error message. Called by the parser on error
 *              state.
 * Synopsis   : void parsererror(char* message)
 * Input      : char* message = the error message.
 * Output     : N/A
 =======================================================================*/
void 
record_error(yyscan_t yyscanner, RecordTree* recordTree,
		  const char* message)
{
  logEmit(LOG_ERR, "%s on token '%s' line %i\n",
	  message, record_get_text(yyscanner), LINENO);
}


/*=======================================================================
 * Function   : parseRecords
 * Description: Parse records from a file or a socket
 * Synopsis   : int parseRecords(int fd)
 * Input      : int fd: file or socket handler
 * Output     : return a RecordTree on success
=======================================================================*/
RecordTree* 
parseRecords(int fd)
{ 
  RecordTree* rc = 0;
  yyscan_t scanner;
  RecordExtra extra;
  RecordTree* tree = 0;

  logParser(LOG_NOTICE, "parse records");

  // scanner input file
  if (fd == -1) {
    logEmit(LOG_ERR, "%s", 
	    "recordList parser need a file descriptor parameter");
    goto error;
  }
  
  // initialise parser
  if (record_lex_init(&scanner)) {
    logEmit(LOG_ERR, "%s", "error initializing scanner");
    goto error;
  }

  // debug mode for scanner
  record_set_debug(env.debugLexer, scanner);
  logEmit(LOG_DEBUG, "record_set_debug = %i", record_get_debug(scanner));

  // initialise parser parameter
  if (!(tree = createRecordTree())) goto error2;

  // use extra data with reentrant scanner
  extra.aesData = &tree->aes;
  extra.aesData->fd = fd;
  extra.aesData->way = DECRYPT;
  record_set_extra (&extra, scanner);

  // call the parser
  if (record_parse(scanner, tree)) {
    logEmit(LOG_ERR, "record file parser error on line %i",
	    record_get_lineno(scanner));
    goto error2;
  }

  // set aes data for default serialization
  tree->aes.fd = STDOUT_FILENO;
  tree->aes.way = ENCRYPT;
  tree->doCypher = extra.aesData->doCypher;

  rc = tree;
  tree = 0;
 error2:
  destroyRecordTree(tree);
  record_lex_destroy(scanner);
 error:  
  return rc;
}

/* Local Variables: */
/* mode: c */
/* mode: font-lock */
/* mode: auto-fill */
/* End: */

 