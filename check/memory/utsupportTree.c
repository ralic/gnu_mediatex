/*=======================================================================
 * Version: $Id: utsupportTree.c,v 1.3 2015/08/30 17:07:58 nroche Exp $
 * Project: MediaTeX
 * Module : md5sumTree
 *
 * SupportTree producer interface

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

#include "mediatex.h"
#include "memory/utFunc.h"
GLOBAL_STRUCT_DEF;

/*=======================================================================
 * Function   : usage
 * Description: Print the usage.
 * Synopsis   : static void usage(char* programName)
 * Input      : programName = the name of the program; usually argv[0].
 * Output     : N/A
 =======================================================================*/
static void 
usage(char* programName)
{
  memoryUsage(programName);
  fprintf(stderr, " [ -d repository ]");

  memoryOptions();
  fprintf(stderr, "  ---\n"
	  "  -d, --input-rep\tsrcdir directory for make distcheck\n");
  return;
}

/*=======================================================================
 * Function   : main 
 * Author     : Nicolas ROCHE
 * modif      : 2012/05/01
 * Description: Unit test for confTree module.
 * Synopsis   : utconfTree
 * Input      : N/A
 * Output     : N/A
 =======================================================================*/
int 
main(int argc, char** argv)
{
  char inputRep[256] = ".";
  Support* supp = 0;
  // ---
  int rc = 0;
  int cOption = EOF;
  char* programName = *argv;
  char* options = MEMORY_SHORT_OPTIONS"d:";
  struct option longOptions[] = {
    MEMORY_LONG_OPTIONS,
    {"input-rep", required_argument, 0, 'd'},
    {0, 0, 0, 0}
  };
  
  // import mdtx environment
  env.dryRun = FALSE;
  getEnv(&env);

  // parse the command line
  while((cOption = getopt_long(argc, argv, options, longOptions, 0)) 
	!= EOF) {
    switch(cOption) {
      
    case 'd':
      if(optarg == 0 || *optarg == (char)0) {
	fprintf(stderr, 
		"%s: nil or empty argument for the input repository\n",
		programName);
	rc = EINVAL;
	break;
      }
      strncpy(inputRep, optarg, strlen(optarg)+1);
      break; 

      GET_MEMORY_OPTIONS; // generic options
    }
    if (rc) goto optError;
  }

  // export mdtx environment
  if (!setEnv(programName, &env)) goto optError;

  /************************************************************************/
  // test types on this architecture:
  off_t offset = 0xFFFFFFFFFFFFFFFFULL; // 2^64
  time_t timer = 0x7FFFFFFF; // 2^32
  logMemory(LOG_DEBUG, "type off_t is store into %u bytes", 
	  (unsigned int)sizeof(off_t));
  logMemory(LOG_DEBUG, "type time_t is store into %u bytes", 
	  (unsigned int) sizeof(time_t));
  logMemory(LOG_NOTICE,  "off_t max value is: %llu", 
	  (unsigned long long int)offset);
  logMemory(LOG_NOTICE, "time_t max value is: %lu", 
	  (unsigned long int)timer);

  // test memory tree
  if (!createExempleSupportTree(inputRep)) goto error;
  if (!(supp = getSupport("SUPP11_logo.png"))) goto error;
  if (!serializeSupports()) goto error;
  /************************************************************************/

  freeConfiguration();
  rc = TRUE;
 error:
  ENDINGS;
  rc=!rc;
 optError:
  exit(rc);
}

/* Local Variables: */
/* mode: c */
/* mode: font-lock */
/* mode: auto-fill */
/* End: */

