/*=======================================================================
 * Version: $Id: have.c,v 1.7 2015/08/07 17:50:33 nroche Exp $
 * Project: MediaTeX
 * Module : have
 *
 * Manage extraction from removable device

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

#include "mediatex-config.h"
#include "server/mediatex-server.h"

int haveArchive(ExtractData* data, Archive* archive);

/*=======================================================================
 * Function   : haveContainer
 * Description: Single container haveion
 * Synopsis   : int haveContainer
 * Input      : Collection* coll = context
 *              Container* container = what to have
 * Output     : int *found = TRUE if we ewtract something
 *              FALSE on error
 =======================================================================*/
int haveContainer(ExtractData* data, Container* container)
{
  int rc = FALSE;
  FromAsso* asso = 0;
  AVLNode *node = 0;
  int found = FALSE;

  checkCollection(data->coll);
  logMain(LOG_DEBUG, "have a container %s/%s:%lli", 
	  strEType(container->type), container->parent->hash,
	  (long long int)container->parent->size);
 
  // look for wanted record depp into all childs
  for(node = container->childs->head; node; node = node->next) {
    asso = node->item;

    if (!haveArchive(data, asso->archive)) goto error;
    if (data->found) found = TRUE;
  }  

  data->found = found;
  rc = TRUE;
 error:
  if (!rc) {
    logMain(LOG_ERR, "haveContainer fails");
  }
  return rc;
} 

/*=======================================================================
 * Function   : haveArchive
 * Description: Single archive haveion
 * Synopsis   : int haveArchive
 * Input      : Collection* coll = context
 *              Archive* archive = what to have
 * Output     : int *found = TRUE if haveed
 *              FALSE on error
 =======================================================================*/
int haveArchive(ExtractData* data, Archive* archive)
{
  int rc = FALSE;
  Record* record = 0;
  RGIT* curr = 0;
  int deliver = FALSE;

  logMain(LOG_DEBUG, "have an archive: %s:%lli", 
	  archive->hash, archive->size);

  data->found = FALSE;
  checkCollection(data->coll);

  // exit condition
  if (archive->state == AVAILABLE) {
    data->found = TRUE;
    goto end;
  }

  // go deeper first
  if (archive->toContainer) {
    if (!haveContainer(data, archive->toContainer)) goto error;
  }

  // do not extract parent if wanted childs was already extracted
  //if (!data->found && archive->state == WANTED) {
  if (archive->state == WANTED) {

    // (same code as extractArchives)
    data->context = X_STEP;
    curr = 0;
    while ((record = rgNext_r(archive->demands, &curr))) {
      if (getRecordType(record) == FINALE_DEMAND) {
	data->context = X_MAIN;
	break;
      }
    }

    logMain(LOG_NOTICE, "have an archive to extract: %s:%lli", 
	    archive->hash, archive->size);
    data->target = archive;
    if (!extractArchive(data, archive))  {
      logMain(LOG_WARNING, "need more place ?");
      goto end;
    }

    // (same code as extractArchives)
    if (data->found) {
      // adjuste to-keep date
      curr = 0;
      while((record = rgNext_r(archive->demands, &curr)) != 0) {
	if (!keepArchive(data->coll, archive, getRecordType(record)))
	  goto error;
	if (getRecordType(record) == FINALE_DEMAND &&
	    !(record->type & REMOVE)) deliver = TRUE;
      }

      if (deliver && !deliverMail(data->coll, archive)) goto error;
      deliver = FALSE;
    }
  }

 end:
  rc = TRUE;
 error:
  if (!rc) {
    logMain(LOG_ERR, "haveArchive fails");
  }
  return rc;
} 

/*=======================================================================
 * Function   : addFinalSupplies
 * Description: 
 * Synopsis   : 
 * Input      :
 * Output     : TRUE on success
 =======================================================================*/
static int 
addFinalSupplies(RecordTree* tree, Record** iso)
{
  int rc = FALSE;
  Record* record = 0;
  RGIT* curr = 0;
  off_t maxSize = 0;

  while((record = rgNext_r(tree->records, &curr))) {
    if (getRecordType(record) != FINALE_SUPPLY) {
      logMain(LOG_ERR, "please provide final supplies");
      goto error;
    }

    if (!addCacheEntry(tree->collection, record)) goto error;

    // math the bigest record as the iso
    if (record->archive->size > maxSize) {
      maxSize = record->archive->size;
      *iso = record;
    }
  }
  rc = TRUE;
 error:
  return rc;
}

/*=======================================================================
 * Function   : delFinalSupplies
 * Description: 
 * Synopsis   : 
 * Input      :
 * Output     : TRUE on success
 =======================================================================*/
static int delFinalSupplies(RecordTree* tree)
{
  int rc = FALSE;
  Record* record = 0;
  RGIT* curr = 0;

  while((record = rgNext_r(tree->records, &curr))) {
    if (!delCacheEntry(tree->collection, record)) goto error;

    // consume records from the tree
    curr->it = 0;
  }
  rc = TRUE;
 error:
  return rc;
}

/*=======================================================================
 * Function   : extractFinaleArchives
 * Description: Try to extract archives using the image provided
 * Synopsis   : int extractFinaleArchives(ArchiveTree* finalSupplies)
 * Input      : ArchiveTree* finalSupplies = support/colls we provide
 * Output     : TRUE on success
 =======================================================================*/
int 
extractFinaleArchives(RecordTree* recordTree, Connexion* connexion)
{
  int rc = FALSE;
  Record* iso = 0;
  ExtractData data;
  (void) connexion;

  logMain(LOG_DEBUG, "remote extraction");
  if (!(data.toKeeps = createRing())) goto error;

  // get the archiveTree's collection
  if ((data.coll = recordTree->collection) == 0) {
    logMain(LOG_ERR, "unknown collection for archiveTree");
    goto error;
  }
  
  // check we get final supplies
  if (isEmptyRing(recordTree->records)) {
    logMain(LOG_ERR, "please provide records for have query");
    goto error;
  }

  if (!loadCollection(data.coll, SERV | EXTR | CACH)) goto error;
  if (!lockCacheRead(data.coll)) goto error2;
  if (!addFinalSupplies(recordTree, &iso)) goto error3;

  // up to down recursive match for wanted records on iso archive
  if (!haveArchive(&data, iso->archive)) goto error4;

  // if we still have room, dump the iso if it is not perenne
#warning test for place before to dump the iso ?
  if (iso->archive->fromContainers->nbItems == 0
      && iso->archive->state < AVAILABLE) {
    if (!computeExtractScore(data.coll)) goto error4;
    if (iso->archive->extractScore 
	<= data.coll->serverTree->scoreParam.maxScore /2) {
      data.target = iso->archive;
      if (!extractArchive(&data, iso->archive)) {
	logMain(LOG_WARNING, "need more place ?");
      }
    }
  }

  rc = TRUE;
 error4:
  if (!delFinalSupplies(recordTree)) rc = FALSE;
 error3:
  if (!unLockCache(data.coll)) rc = FALSE;
 error2:
  if (!releaseCollection(data.coll, SERV | EXTR | CACH)) rc = FALSE;
 error:
  if (!rc) {
    logMain(LOG_ERR, "remote extraction fails");
  }
  destroyOnlyRing(data.toKeeps);
  return rc;
} 

/* Local Variables: */
/* mode: c */
/* mode: font-lock */
/* mode: auto-fill */
/* End: */

