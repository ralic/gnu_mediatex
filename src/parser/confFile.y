/*=======================================================================
 * Version: $Id: confFile.y,v 1.9 2015/08/31 00:14:52 nroche Exp $
 * Project: Mediatex
 * Module : conf parser
 *
 * configuration file parser
 
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

/* prologue: =======================================================*/

%code provides {
/*=======================================================================
 * Version: this file is generated by BISON using confFile.y
 * Project: MediaTeX
 * Module : conf parser
 *
 * configuation file parser

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

#define YYSTYPE CONF_STYPE
}

%{  
#include "mediatex-config.h"
  /* yyscan_t is pre-defined by mediatex-config.h and will be defined
     by Flex header bellow */
%}

%union {
  char          string[MAX_SIZE_STRING+1];
  unsigned int  number;
  float         score;
  off_t         size;
  time_t        time;
}

%{
  // upper YYSTYPE union is required by Flex headers
#include "parser/confFile.h"

#define LINENO conf_get_lineno(yyscanner)
  
void conf_error(yyscan_t yyscanner, Collection* coll, 
		const char* message);
%}

/* declarations: =======================================================*/
%defines "parser/confFile.tab.h"
%output "parser/confFile.tab.c"
%define api.prefix {conf_}
%define api.pure full
%param {yyscan_t yyscanner}
%parse-param {Collection* coll}
%define parse.error verbose
%verbose
%debug

%start stanzas

%token          confERROR
%token          confHOST
%token          confNETWORKS
%token          confGATEWAYS
%token          confMDTXPORT
%token          confSSHPORT
%token          confWWWPORT
%token          confCACHESIZE
%token          confCACHETTL
%token          confQUERYTTL
%token          confCHECKTTL
%token          confSUPPTTL
%token          confMAXSCORE
%token          confBADSCORE
%token          confPOWSUPP
%token          confPOWIMAGE
%token          confFACTSUPP
%token          confCOLL
%token          confCOMMENT
%token          confSHARE
%token          confLOCALHOST
%token          confCOMMA
%token          confDOT
%token          confCOLON
%token          confAROBASE
%token          confMINUS
%token          confENDBLOCK

%token <string> confSTRING
%token <number> confNUMBER
%token <score>  confSCORE
%token <size>   confSIZE
%token <time>   confTIME

%%
 /* grammar rules: =====================================================*/
   
stanzas: stanzas confLine
       | stanzas collectionStanza
       | confLine
       | collectionStanza
;
 
collectionStanza: collection collectionLines confENDBLOCK
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, 
	    "collectionStanza: newCollection collectionLines confENDBLOCK");
  coll = 0;
}
;

collection: confCOLL confSTRING confMINUS confSTRING confAROBASE confSTRING confCOLON confNUMBER
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, 
	    "newCollection: confCOLL confSTRING confAROBASE confSTRING"
	    " confCOLON confNUMBER");
  if (!(coll = addCollection($4))) YYERROR;

  if (!(coll->masterLabel = createString($2))) YYERROR;
  strncpy(coll->masterHost, $6, MAX_SIZE_HOST);
  coll->masterPort = $8;
}
;

collectionLines: collectionLines collectionLine
               | collectionLine
;

collectionLine: confSHARE supports
              | confLOCALHOST confSTRING
{
  logParser(LOG_DEBUG, "line %-3i localhost = %s", LINENO, $2);
  strncpy(coll->userFingerPrint, $2, MAX_SIZE_HASH+1);
}
              | confNETWORKS cnetworks
              | confGATEWAYS cgateways
              | confCACHESIZE confNUMBER confSIZE
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "cache size");
  coll->cacheSize = $2*$3;
}
              | confCACHETTL confNUMBER confTIME
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "cache TTL");
  coll->cacheTTL = $2*$3;
}
              | confQUERYTTL confNUMBER confTIME
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "query TTL");
  coll->queryTTL = $2*$3;
}
;

supports: supports confCOMMA support
        | support
;

support: confSTRING
{
  Support* supp = 0;
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "support");
  if (!(supp = addSupport($1))) YYERROR;
  if (!addSupportToCollection(supp, coll)) YYERROR;
}
;

confLine: confHOST confSTRING
{
  logParser(LOG_DEBUG, "line %-3i host = %s", LINENO, $2);
  strncpy(getConfiguration()->host, $2, MAX_SIZE_HOST);
}
       | confNETWORKS Cnetworks
{
  logParser(LOG_DEBUG, "line %-3i networks", LINENO);
}
       | confGATEWAYS Cgateways
{
  logParser(LOG_DEBUG, "line %-3i gateways", LINENO);
}
       | confCOMMENT confSTRING
{
  logParser(LOG_DEBUG, "line %-3i comment = %s", LINENO, $2);
  destroyString(getConfiguration()->comment);
  if (!(getConfiguration()->comment = createString($2))) YYERROR;
}
       | confMDTXPORT confNUMBER
{
  logParser(LOG_DEBUG, "line %-3i mdtxPort = %i", LINENO, $2);
  getConfiguration()->mdtxPort = $2;
}
       | confSSHPORT confNUMBER
{
  logParser(LOG_DEBUG, "line %-3i sshPort = %i", LINENO, $2);
  getConfiguration()->sshPort = $2;
}
       | confWWWPORT confNUMBER
{
  logParser(LOG_DEBUG, "line %-3i wwwPort = %i", LINENO, $2);
  getConfiguration()->wwwPort = $2;
}
       | confCACHESIZE confNUMBER confSIZE
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "cache size");
  getConfiguration()->cacheSize = $2*$3;
}
       | confCACHETTL confNUMBER confTIME
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "cache TTL");
  getConfiguration()->cacheTTL = $2*$3;
}
       | confQUERYTTL confNUMBER confTIME
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "query TTL");
  getConfiguration()->queryTTL = $2*$3;
}
       | confCHECKTTL confNUMBER confTIME
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "check TTL");
  getConfiguration()->checkTTL = $2*$3;
}
       | confSUPPTTL confNUMBER confTIME
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "supp TTL");
  getConfiguration()->scoreParam.suppTTL = $2*$3;
}
       | confMAXSCORE confSCORE
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "score: max");
  getConfiguration()->scoreParam.maxScore = $2;
}
       | confBADSCORE confSCORE
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "score: bad");
  getConfiguration()->scoreParam.badScore = $2;
}
       | confPOWSUPP confSCORE
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "score: pow supp");
  getConfiguration()->scoreParam.powSupp = $2;
}
       | confFACTSUPP confSCORE
{
  logParser(LOG_DEBUG, "line %-3i %s", LINENO, "score: fact supp");
  getConfiguration()->scoreParam.factSupp = $2;
}
;


cnetworks: cnetworks confCOMMA cnetwork
         | cnetwork
;

cgateways: cgateways confCOMMA cgateway
         | cgateway
;

Cnetworks: Cnetworks confCOMMA Cnetwork
         | Cnetwork
;

Cgateways: Cgateways confCOMMA Cgateway
         | Cgateway
;

cnetwork: confSTRING
{
  logParser(LOG_DEBUG, "line %-3i network: %s", LINENO, $1);
  if (!addNetworkToRing(coll->networks, $1)) YYABORT;
}
;

cgateway: confSTRING
{
  logParser(LOG_DEBUG, "line %-3i gateway: %s", LINENO, $1);
  if (!addNetworkToRing(coll->gateways, $1)) YYABORT;
}
;

Cnetwork: confSTRING
{
  logParser(LOG_DEBUG, "line %-3i network: %s", LINENO, $1);
  if (!addNetworkToRing(getConfiguration()->networks, $1)) YYABORT;
}
;

Cgateway: confSTRING
{
  logParser(LOG_DEBUG, "line %-3i gateway: %s", LINENO, $1);
  if (!addNetworkToRing(getConfiguration()->gateways, $1)) YYABORT;
}
;

%%
	  /* epilogue: =================================================*/


/*=======================================================================
 * Function   : parsererror
 * Description: Emit an error message. Called by the parser on error
 *              state.
 * Synopsis   : void parsererror(char* message)
 * Input      : char* message = the error message.
 * Output     : N/A
 * Note       : Sorry, find no way to get the lineNo using reentrant
 *              parser.
 =======================================================================*/
void 
conf_error(yyscan_t yyscanner, Collection* coll, const char* message)
{
  logParser(LOG_ERR, "%s on token '%s' line %i", message, 
	  conf_get_text(yyscanner), LINENO);
}

/*=======================================================================
 * Function   : parseConfiguration
 * Description: Parse a file
 * Synopsis   : int parseConfiguration(const char* path)
 * Input      : const char* path: the file to parse
 * Output     : TRUE on success
=======================================================================*/
int 
parseConfiguration(const char* path)
{ 
  int rc = FALSE;
  FILE* inputStream = stdin;
  yyscan_t scanner;
  Collection* coll = 0;
 
  logParser(LOG_INFO, "parse configuration from %s",
	    path?path:"stdin");

  // initialise scanner
  if (conf_lex_init(&scanner)) {
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

  conf_set_in(inputStream, scanner);

  // debug mode for scanner
  conf_set_debug(env.debugLexer, scanner);
  logParser(LOG_DEBUG, "conf_set_debug = %i", conf_get_debug(scanner));

  // call the parser
  if (conf_parse(scanner, coll)) {
    logParser(LOG_ERR, "configuration parser fails on line %i", 
	    conf_get_lineno(scanner));
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
    logParser(LOG_ERR, "%s", "configuration parser fails");
  }
  conf_lex_destroy(scanner);
  return rc;
}

/* Local Variables: */
/* mode: c */
/* mode: font-lock */
/* mode: auto-fill */
/* End: */

