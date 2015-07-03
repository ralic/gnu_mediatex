/*=======================================================================
 * Version: $Id: utcacheTree.c,v 1.1 2015/07/01 10:49:45 nroche Exp $
 * Project: MediaTeX
 * Module : cache
 *
 * Manage local cache directory and DB

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
 * Function   : testThread
 * Description: Testing the concurent access
 * Synopsis   : void* testThread(void* arg)
 * Input      : void* arg: CacheTree* mergeMd5
 * Output     : N/A
 =======================================================================*/
void* 
testThread(void* arg)
{
  //env.recordTree = copyCurrentRecordTree(mergeMd5);
  return 0;
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
  fprintf(stderr, " [ -d repository ]");

  memoryOptions();
  //fprintf(stderr, "\t\t---\n");
  return;
}


/*=======================================================================
 * Function   : main 
 * Author     : Nicolas ROCHE
 * modif      : 2012/05/01
 * Description: Unit test for cache module.
 * Synopsis   : ./utcache
 * Input      : N/A
 * Output     : N/A
 =======================================================================*/
int 
main(int argc, char** argv)
{
  Collection* coll = 0;
  Record* record = 0;
  //pthread_t thread;
  // ---
  int rc = 0;
  int cOption = EOF;
  char* programName = *argv;
  char* options = MEMORY_SHORT_OPTIONS;
  struct option longOptions[] = {
    MEMORY_LONG_OPTIONS,
    {0, 0, 0, 0}
  };

  // import mdtx environment
  env.dryRun = FALSE;
  env.debugMemory = TRUE;
  getEnv(&env);

  // parse the command line
  while((cOption = getopt_long(argc, argv, options, longOptions, 0)) 
	!= EOF) {
    switch(cOption) {
 
      GET_MEMORY_OPTIONS; // generic options
    }
    if (rc) goto optError;
  }

  // export mdtx environment
  if (!setEnv(programName, &env)) goto optError;

  /************************************************************************/
  if (!createExempleConfiguration()) goto error;
  if (!(coll = getCollection("coll1"))) goto error;

  if (!lockCacheRead(coll)) goto error;
  if (!unLockCache(coll)) goto error;
  if (!lockCacheWrite(coll)) goto error;
  if (!unLockCache(coll)) goto error;
  if (!lockCacheRead(coll)) goto error;
  if (!lockCacheRead(coll)) goto error;
  if (!unLockCache(coll)) goto error;
  if (!unLockCache(coll)) goto error;

  if (!createExempleRecordTree(coll)) goto error;

  // index the record tree
  logMemory(LOG_DEBUG, "%s", "___ test indexation ___");
  coll->cacheTree->recordTree->aes.fd = STDERR_FILENO;

  while ((record = rgHead(coll->cacheTree->recordTree->records))) {
    if (!serializeRecord(coll->cacheTree->recordTree, record)) goto error;
    aesFlush(&coll->cacheTree->recordTree->aes);
    fprintf(stderr, "\n");

    if (!addCacheEntry(coll, record)) goto error;
    if (record->archive->state >= AVAILABLE) {
      if (!keepArchive(coll, record->archive, 0)) goto error;
      if (!unKeepArchive(coll, record->archive)) goto error;
    }
    if (!delCacheEntry(coll, record)) goto error;
    if (!cleanCacheTree(coll)) goto error;
  }
  if (!diseaseCacheTree(coll)) goto error;
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