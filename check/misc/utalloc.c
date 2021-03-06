/*=======================================================================
 * Project: MediaTeX
 * Module : alloc
 *
 * modified malloc

 MediaTex is an Electronic Records Management System
 Copyright (C) 2014 2015 2016 2017 Nicolas Roche

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

#include "mediatex-config.h"

#define BLOCK_SIZE 100
#define BLOCK_MAX  10

char* ptr[BLOCK_MAX];
long s = 0;

/*=======================================================================
 * Function   : disease
 * Description: Callback function call when malloc fails
 * Synopsis   : int disease(size_t size)
 * Input      : size_t size: amount of memory to try to free
 * Output     : TRUE if some memory was free
 =======================================================================*/
int disease(long size)
{
  int rc = FALSE;
  Alloc* alloc = env.alloc;
  long goal = 0;
  int i=0;

  logMain(LOG_INFO, "disease callback: %i", size);

  goal = alloc->maxAllocated - size;
  if (goal < 0) goal = 0;

  while (i<BLOCK_MAX && alloc->sumAllocated > goal) {
    while (i<BLOCK_MAX && !ptr[i]) ++i;
    if (i<BLOCK_MAX) {
      logMain(LOG_INFO, "free slot %i", i);
      free(ptr[i]);
      ptr[i] = 0;
      rc = TRUE;
    }
    ++i;
  }

  // check if something was free
  if (!rc) {
    logMain(LOG_ERR, "fail to disease memory");
  }
  return rc;
}

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
  mdtxUsage(programName);
  mdtxOptions();

  return;
}


/*=======================================================================
 * Function   : main 
 * Description: Unit test for alloc module
 * Synopsis   : ./utalloc -A
 * Input      : N/A
 * Output     : N/A
 =======================================================================*/
int 
main(int argc, char** argv)
{
  int i = 0;
  // ---
  int rc = 0;
  int cOption = EOF;
  char* programName = *argv;
  char* options = MISC_SHORT_OPTIONS;
  struct option longOptions[] = {
    MISC_LONG_OPTIONS,
    {0, 0, 0, 0}
  };

  // import mdtx environment
  env = envUnitTest;
  env = envUnitTest;
  env.allocLimit = .002; // 2 Ko
  env.allocDiseaseCallBack = disease;
  getEnv(&env);

  // parse the command line
  while ((cOption = getopt_long(argc, argv, options, longOptions, 0)) 
	!= EOF) {
    switch(cOption) {
      
      GET_MISC_OPTIONS; // generic options
    }
    if (rc) goto optError;
  }

  // export mdtx environment
  if (!setEnv(programName, &env)) goto optError;

  /************************************************************************/
  for (i=1; i<BLOCK_MAX; ++i) {
    logMain(LOG_NOTICE, "allocating slot %i...", i);
    if (!(ptr[i] = malloc(BLOCK_SIZE*i))) goto error;
    memset(ptr[i], 42, BLOCK_SIZE*i);
    logMain(LOG_NOTICE, "...slot %i allocated", i);
    memoryStatus(LOG_DEBUG, __FILE__, __LINE__);
  }

  // Finished
  for (i=1; i<BLOCK_MAX; ++i) {
    if (ptr[i]) {
      logMain(LOG_NOTICE, "freeing slot %i...", i);
      free(ptr[i]);
      logMain(LOG_NOTICE, "...slot %i is free", i);
    }
  }
  /************************************************************************/

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
