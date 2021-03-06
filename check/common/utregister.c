/*=======================================================================
 * Project: MediaTeX
 * Module : register
 
 * test for register

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

#include "mediatex.h"

#define UNDEF -1
#define INIT  10
#define GET   11
#define FREE  12
#define ERROR 13

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
  fprintf(stderr, 
	  "\n\t\t{ -I | -G | -F | "
	  "\n\t\t  -W | -E | -N | -Q | -S | -T | -C | -P | -L }"
	  "\n\t\t[ -e ]\n");

  mdtxOptions();
  fprintf(stderr, "  ---\n"
	  "  -I, --initialize\tinitialize share memory\n"
	  "  -G, --read-shm\tread share memory\n"
	  "  -F, --free-shm\tfree share memory\n"
	  "  -W, --do-save\t\tsave md5sums.txt file\n"
	  "  -E, --do-extract\tperform extracton\n"
	  "  -N, --do-notify\tperform notification\n"
	  "  -Q, --do-quick-scan\tperform quick scan\n"
	  "  -S, --do-scan\tperform scan\n"
	  "  -T, --do-trim\tperform trim\n"
	  "  -C, --do-clean\tperform clean\n"
	  "  -P, --do-purge\tperform purge\n"
	  "  -L, --do-status\tperform log status\n"
	  "  -e, --set-error\terror test\n");
  return;
}


/*=======================================================================
 * Function   : main 
 * Author     : Nicolas ROCHE
 * modif      : 2012/05/01
 * Description: Unit test for register module.
 * Synopsis   : ./utregister
 * Input      : (see usage above)
 * Output     : stdout
 =======================================================================*/
int 
main(int argc, char** argv)
{
  int signal = UNDEF;
  ShmParam param;
  int doError = FALSE;
  // ---
  int rc = 0;
  int cOption = EOF;
  char* programName = *argv;
  char* options = MDTX_SHORT_OPTIONS"IGFWENQSTCPde";
  struct option longOptions[] = {
    MDTX_LONG_OPTIONS,
    {"initialize", required_argument, 0, 'I'},
    {"read-shm", required_argument, 0, 'G'},
    {"free-shm", required_argument, 0, 'F'},
    {"do-save", required_argument, 0, 'W'},
    {"do-extract", required_argument, 0, 'E'},
    {"do-notify", required_argument, 0, 'N'},
    {"do-quick-scan", required_argument, 0, 'Q'},
    {"do-scan", required_argument, 0, 'S'},
    {"do-trim", required_argument, 0, 'T'},
    {"do-clean", required_argument, 0, 'C'},
    {"do-purge", required_argument, 0, 'P'},
    {"do-status", required_argument, 0, 'd'},
    {"set-error", required_argument, 0, 'e'},
    {0, 0, 0, 0}
  };

  // import mdtx environment
  env = envUnitTest;
  getEnv(&env);

  // parse the command line
  while ((cOption = getopt_long(argc, argv, options, longOptions, 0)) 
	!= EOF) {
    switch(cOption) {

    case 'I':
      if (signal != UNDEF) rc=1;
      signal = INIT;
      break;

    case 'G':
      if (signal != UNDEF) rc=2;
      signal = GET;
      break;

    case 'F':
      if (signal != UNDEF) rc=3;
      signal = FREE;
      break;

    case 'W':
      if (signal != UNDEF) rc=4;
      signal = REG_SAVEMD5;
      break;

    case 'E':
      if (signal != UNDEF) rc=5;
      signal = REG_EXTRACT;
      break;

    case 'N':
      if (signal != UNDEF) rc=6;
      signal = REG_NOTIFY;
      break;

    case 'Q':
      if (signal != UNDEF) rc=7;
      signal = REG_QUICKSCAN;
      break;

    case 'S':
      if (signal != UNDEF) rc=7;
      signal = REG_SCAN;
      break;

    case 'T':
      if (signal != UNDEF) rc=7;
      signal = REG_TRIM;
      break;

    case 'C':
      if (signal != UNDEF) rc=7;
      signal = REG_CLEAN;
      break;

    case 'P':
      if (signal != UNDEF) rc=7;
      signal = REG_PURGE;
      break;

    case 'd':
      if (signal != UNDEF) rc=7;
      signal = REG_STATUS;
      break;

    case 'e':
      doError = TRUE;
      break;

      GET_MDTX_OPTIONS; // generic options
    }
    if (rc) goto optError;
  }

  // export mdtx environment
  usleep(50000);
  if (!setEnv(programName, &env)) goto optError;

  /************************************************************************/
  switch (signal) {
  case INIT:
    rc = mdtxShmInitialize();
    break;
  case GET:
    if (!(rc = shmRead(getConfiguration()->confFile, REG_SHM_BUFF_SIZE,
		       mdtxShmRead, (void*)&param)))
      goto error;
    printf("=> %s\n", param.buf);
    break;
  case FREE:
    rc = mdtxShmFree();
    break;
  case UNDEF:
    usage(programName);
    break;
  default:
    if (doError) {
      param.flag = signal;
      if (!(rc = shmWrite(getConfiguration()->confFile, REG_SHM_BUFF_SIZE,
			  mdtxShmError, (void*)&param)))
	goto error;
    }
    else {
      rc = mdtxSyncSignal(signal);
    }
  }
  /************************************************************************/

 error:
  env.logHandler->severity[LOG_MEMORY]->code = LOG_NOTICE;
  freeConfiguration();
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
