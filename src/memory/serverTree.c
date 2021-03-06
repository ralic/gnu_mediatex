/*=======================================================================
 * Project: MediaTeX
 * Module : serverTree

* Server producer interface

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


/*=======================================================================
 * Function   : createImage 
 * Description: Create, by memory allocation an image file (ISO file)
 * Synopsis   : Image* createImage(void)
 * Input      : N/A
 * Output     : The address of the create empty image file
 =======================================================================*/
Image* 
createImage(void)
{
  Image* rc = 0;

  rc = (Image*)malloc(sizeof(Image));
  if(rc == 0)
    goto error;
   
  memset(rc, 0, sizeof(Image));

  return(rc);
 error:
  logMemory(LOG_ERR, "malloc: cannot create an Image");
  return(rc);
}

/*=======================================================================
 * Function   : destroyImage 
 * Description: Destroy an image file by freeing all the allocate memory.
 * Synopsis   : void destroyImage(Image* self)
 * Input      : Image* self = the address of the image file to
 *              destroy.
 * Output     : Nil address of an image FILE
 =======================================================================*/
Image* 
destroyImage(Image* self)
{
  if(self == 0) goto error;
  free(self);
 error:
  return (Image*)0;
}

/*=======================================================================
 * Function   : serializeImage 
 * Description: 2012/01/30
 *              Serialize a configuration.
 * Synopsis   :
 *              int serializeImage(Image* self, FILE* fd)
 * Input      :
 *              Image* self = what to serialize
 *              FILE* fd     = where to serialize
 * Output     :
 *               0 = success
 *              -1 = error
 =======================================================================*/
int 
serializeImage(Image* self, FILE* fd)
{
  int rc = FALSE;

  if(self == 0) {
    logMemory(LOG_ERR, "empty image");
    goto error;
  }

  fprintf(fd, "%s:%llu=%.2f", 
	  self->archive->hash, 
	  (long long unsigned int)self->archive->size, 
	  self->score);

  rc = TRUE;
 error:
  if (!rc) {
    logMemory(LOG_ERR, "serializeImage fails");
  }
  return rc;
}

/*=======================================================================
 * Function   : cmpImage
 * Description: Compare 2 image files
 * Synopsis   : int cmpImage(const void *p1, const void *p2)
 * Input      : const void *p1, const void *p2: the image to compare
 * Output     : like strcmp
 =======================================================================*/
int 
cmpImage(const void *p1, const void *p2)
{
  int rc = 0;

  /* p1 and p2 are pointers on &items
   * and items are suposed to be Image* 
   */
  
  Image* a1 = *((Image**)p1);
  Image* a2 = *((Image**)p2);

  rc = cmpArchive(&a1->archive, &a2->archive);
  if (!rc) rc = cmpServer(&a1->server, &a2->server);
  return rc;
}

/*=======================================================================
 * Function   : cmpImageScore
 * Description: Compare 2 image files
 * Synopsis   : int cmpImageScore(const void *p1, const void *p2)
 * Input      : const void *p1, const void *p2: the image to compare
 * Output     : like strcmp
 * Note       : order by decreasing scores
 =======================================================================*/
int 
cmpImageScore(const void *p1, const void *p2)
{
  int rc = 0;

  /* p1 and p2 are pointers on &items
   * and items are suposed to be Image* 
   */
  
  Image* a1 = *((Image**)p1);
  Image* a2 = *((Image**)p2);

  rc = cmpArchive(&a1->archive, &a2->archive);
  if (!rc && a1->score < a2->score) rc = +1;
  if (!rc && a1->score > a2->score) rc = -1;
  //if (!rc) rc = cmpServer(&a1->server, &a2->server);
  return rc;
}


/*=======================================================================
 * Function   : createServer 
 * Description: Create, by memory allocation a Server configuration 
 *              projection.
 * Synopsis   : Server* createServer(void)
 * Input      : N/A
 * Output     : The address of the create empty configuration.
 =======================================================================*/
Server* 
createServer(void)
{
  Server* rc = 0;

  rc = (Server*)malloc(sizeof(Server));
  if(rc == 0) goto error;
   
  memset(rc, 0, sizeof(Server));

  // note: we do not set default ports (to allow to change them)
  rc->score = -1;
  rc->address.sin_addr.s_addr = LOCALHOST; // localhost by default

  if ((rc->networks = createRing()) == 0) goto error;
  if ((rc->gateways = createRing()) == 0) goto error;
  if ((rc->images = createRing()) == 0) goto error;

   /* if (!(rc->records =  */
   /* 	 avl_alloc_tree(cmpRecordAvl, (avl_freeitem_t)destroyRecord))) */
   /*   goto error; */

   return rc;
 error:
  logMemory(LOG_ERR, "malloc: cannot create Server");
  return destroyServer(rc);
}

/*=======================================================================
 * Function   : destroyServer 
 * Description: Destroy a configuration by freeing all the allocate memory
 * Synopsis   : void destroyServer(Server* self)
 * Input      : Server* self = the address of the configuration to
 *              destroy.
 * Output     : Nil address of a configuration.
 =======================================================================*/
Server* 
destroyServer(Server* self)
{
  if(self == 0) goto error;
  self->label = destroyString(self->label);
  self->user = destroyString(self->user);
  self->comment = destroyString(self->comment);
  self->url = destroyString(self->url);
  self->mdtxPort = 0;
  self->sshPort = 0;
  self->wwwPort = 0;
  self->userKey = destroyString(self->userKey);
  self->hostKey = destroyString(self->hostKey);
  
  // do not delete objets (owned by serverTree or recordTree)
  self->networks = destroyOnlyRing(self->networks);
  self->gateways = destroyOnlyRing(self->gateways);
  //self->records->freeitem = 0;
  //avl_free_tree(self->records);

  self->images = 
    destroyRing(self->images, (void*(*)(void*)) destroyImage);

  free(self);
 error:
  return (Server*)0;
}

/*=======================================================================
 * Function   : serializeServer 
 * Description: Serialize a configuration.
 * Synopsis   : int serializeServer(Server* self, FILE* fd)
 * Input      : Server* self = what to serialize
 *              FILE* fd     = where to serialize
 * Output     :  0 = success
 *              -1 = error
 =======================================================================*/
int 
serializeServer(Server* self, FILE* fd)
{
  int rc = FALSE;
  Image* image = 0;
  char* string = 0;
  RGIT* curr = 0;
  struct tm date;

  if(self == 0) {
    logMemory(LOG_ERR, "cannot serialize empty Server");
    goto error;
  }

  if (localtime_r(&self->lastCommit, &date) == (struct tm*)0) {
    logMemory(LOG_ERR, "localtime_r returns on error");
    goto error;
  }
  
  fprintf(fd, "\nServer %s\n", self->fingerPrint);
  if (!isEmptyString(self->comment)) {
    fprintf(fd, "\t%-10s \"%s\"\n", "comment", self->comment);
  }
  if (!isEmptyString(self->label)) {
    fprintf(fd, "\t%-10s %s\n", "label", self->label);
  }
  if (!isEmptyString(self->host)) {
    fprintf(fd, "\t%-10s %s\n", "host", self->host);
  }

  if (self->lastCommit) {
    fprintf(fd, "\t%-10s %04i-%02i-%02i,%02i:%02i:%02i \n", "lastCommit",
	    date.tm_year + 1900, date.tm_mon+1, date.tm_mday,
	    date.tm_hour, date.tm_min, date.tm_sec);
  }
  
  if (self->mdtxPort) {
    fprintf(fd, "\t%-10s %i\n", "mdtxPort", self->mdtxPort);
  }
  if (self->sshPort) {
    fprintf(fd, "\t%-10s %i\n", "sshPort", self->sshPort);
  }
  if (self->wwwPort) {
    fprintf(fd, "\t%-10s %i\n", "wwwPort", self->wwwPort);
  }

  if (!isEmptyRing(self->networks)) {
    fprintf(fd, "\t%-10s ", "networks");
    if (!rgSort(self->networks, cmpString)) goto error;
    curr = 0;
    if (!(string = rgNext_r(self->networks, &curr))) goto error;
    fprintf(fd, "%s", string);
    while ((string = rgNext_r(self->networks, &curr))) {
      fprintf(fd, ", %s", string);
    }
    fprintf(fd, "\n");
  }
  
  if (!isEmptyRing(self->gateways)) {
    fprintf(fd, "\t%-10s ", "gateways");
    if (!rgSort(self->gateways, cmpString)) goto error;
    curr = 0;
    if (!(string = rgNext_r(self->gateways, &curr))) goto error;
    fprintf(fd, "%s", string);
    while ((string = rgNext_r(self->gateways, &curr))) {
      fprintf(fd, ", %s", string);
    }
    fprintf(fd, "\n");
  }

  /* cache parameters (take care, this use macro) */
  if (self->cacheSize) {
    printCacheSize(fd, "\t%-10s", "cacheSize", self->cacheSize);
  }
  if (self->cacheTTL) {
    printLapsTime(fd, "\t%-10s", "cacheTTL", self->cacheTTL);
  }
  if (self->queryTTL) {
    printLapsTime(fd, "\t%-10s", "queryTTL", self->queryTTL);
  }

  // list of images
  if (!isEmptyRing(self->images)) {
    if (!rgSort(self->images, cmpImage)) goto error;
    curr = 0;
    if (!(image = rgNext_r(self->images, &curr))) goto error;
    fprintf(fd, "%s", "\tprovide\t  ");
    do {
      if (!serializeImage(image, fd)) goto error;
      image = rgNext_r(self->images, &curr);
      if (image) fprintf(fd, ",\n\t\t  ");
    }
    while (image);
    fprintf(fd, "\n");
  } 

  fprintf(fd, "%s", "#\tkeys:\n");
  if (!isEmptyString(self->userKey)) {
    fprintf(fd, "\t%-10s \"%s\"\n", "userKey", self->userKey);
  }
  if (!isEmptyString(self->hostKey)) {
    fprintf(fd, "\t%-10s \"%s\"\n", "hostKey", self->hostKey);
  }
  
  fprintf (fd, "end\n");
  rc = TRUE;
 error:
  return(rc);
}

/*=======================================================================
 * Function   : cmpServer
 * Description: compare 2 servers
 * Synopsis   : int cmpServer(const void *p1, const void *p2)
 * Input      : const void *p1, const void *p2: the servers to compare
 * Output     : like strcmp
 =======================================================================*/
int 
cmpServer(const void *p1, const void *p2)
{
  int rc = 0;

  /* p1 and p2 are pointers on &items
   * and items are suposed to be Image* 
   */
  
  Server* a1 = *((Server**)p1);
  Server* a2 = *((Server**)p2);

  rc = strcmp(a1->fingerPrint, a2->fingerPrint);
  return rc;
}


/*=======================================================================
 * Function   : createServerTree 
 * Description: Create, by memory allocation a ServerTree
 *              configuration projection.
 * Synopsis   : ServerTree* createServerTree(void)
 * Input      : N/A
 * Output     : The address of the create empty configuration.
 =======================================================================*/
ServerTree* 
createServerTree(void)
{
  ServerTree* rc = 0;
  ScoreParam defaultScoreParam = DEFAULT_SCORE_PARAM;

  rc = (ServerTree*)malloc(sizeof(ServerTree));
  if(rc == 0)
    goto error;
    
  memset(rc, 0, sizeof(ServerTree));
  strncpy(rc->aesKey, "01234567890abcdef", MAX_SIZE_AES);
  rc->doHttps = TRUE;
  rc->log = NO_LOG;
  rc->uploadTTL = DEFAULT_TTL_UPLOAD;
  rc->serverTTL = DEFAULT_TTL_SERVER;
  rc->scoreParam = defaultScoreParam;
  rc->minGeoDup = DEFAULT_MIN_GEO;

  if ((rc->servers = createRing()) == 0) goto error;
  if ((rc->archives = createRing()) == 0) goto error;
  
  return rc;
 error:
  logMemory(LOG_ERR, "malloc: cannot create a serverTree");
  return destroyServerTree(rc);
}

/*=======================================================================
 * Function   : destroyServerTree 
 * Description: Destroy a configuration by freeing all the allocate memory.
 * Synopsis   : void destroyServerTree(ServerTree* self)
 * Input      : ServerTree* self = the address of the configuration to
 *              destroy.
 * Output     : Nil address of a configuration.
 =======================================================================*/
ServerTree* 
destroyServerTree(ServerTree* self)
{
  if(self == 0) goto error;
  self->dnsUrl = destroyString(self->dnsUrl);
  
  // do not delete archives objets 
  self->archives = destroyOnlyRing(self->archives);
  self->servers = destroyRing(self->servers, 
				  (void*(*)(void*)) destroyServer);
  free(self);
 error:
  return (ServerTree*)0;
}

/*=======================================================================
 * Function   : serializeServerTree
 * Description: Serialize a server tree
 * Synopsis   : int serializeServerTree(Collection* coll)
 * Input      : Collection coll* = what to serialize
 * Output     : TRUE on success
 =======================================================================*/
int 
serializeServerTree(Collection* coll)
{ 
  int rc = FALSE;
  char* path = 0;
  FILE* fd = stdout; 
  ServerTree *self = 0;
  Server *server = 0;
  RGIT* curr = 0;
  int uid = getuid();

  checkCollection(coll);
  if(!(self = coll->serverTree)) goto error;
  logMemory(LOG_DEBUG, "serialize %s server tree", coll->label);

  // we neeed to use the home git collection directory
  if (!(coll->memoryState & EXPANDED)) {
    logMemory(LOG_ERR, "collection must be expanded first");
    goto error;
  }

  if (!becomeUser(coll->user, TRUE)) goto error;

  // output file
  if (env.dryRun == FALSE) path = coll->serversDB;
  logMemory(LOG_INFO, "Serializing the server tree file: %s", 
	  path?path:"stdout");
  if (path && *path != (char)0) {
    if (!(fd = fopen(path, "w"))) {
      logMemory(LOG_ERR, "fdopen %s fails: %s", path, strerror(errno));
      fd = stdout;
      goto error;
    }
    if (!lock(fileno(fd), F_WRLCK)) goto error;
  }

  fprintf(fd, "# This file is managed by MediaTeX software.\n");
  
  if (!self->master) {
    logMemory(LOG_WARNING, "loose master server for %s collection", 
	    coll->label);
  }

  // tel which is the master collection server
  if (self->master) {
    fprintf(fd, "# MediaTeX %s@%s:%i servers list:\n\n", 
	    self->master->user, self->master->host, 
	    self->master->sshPort);
    fprintf(fd, "%-10s %s\n", "master", self->master->fingerPrint);
  }

  if (*self->dnsHost) {
    fprintf(fd, "%-10s %s\n", "dnsHost", self->dnsHost);
  }
  fprintf(fd, "%-10s %s\n", "collKey", self->aesKey);
  fprintf(fd, "%-10s %s\n", "https", self->doHttps?"yes":"no");
  
  fprintf(fd, "\n# self-ingestion parameters\n");
  fprintf(fd, "%-10s %s\n", "logApache", self->log & APACHE?"yes":"no");
  fprintf(fd, "%-10s %s\n", "logGit", self->log & GIT?"yes":"no");
  fprintf(fd, "%-10s %s\n", "logAudit", self->log & AUDIT?"yes":"no");

  fprintf(fd, "\n# score parameters\n");
  printLapsTime(fd, "%-10s", "uploadTTL", self->uploadTTL);
  printLapsTime(fd, "%-10s", "serverTTL", self->serverTTL);
  printLapsTime(fd, "%-10s", "suppTTL",  self->scoreParam.suppTTL);
  fprintf(fd, "%-10s %.2f\n", "maxScore", self->scoreParam.maxScore);
  fprintf(fd, "%-10s %.2f\n", "badScore", self->scoreParam.badScore);
  fprintf(fd, "%-10s %.2f\n", "powSupp",  self->scoreParam.powSupp);
  fprintf(fd, "%-10s %.2f\n", "factSupp", self->scoreParam.factSupp);
  fprintf(fd, "%-10s %.2f\n", "fileScore", self->scoreParam.fileScore);
  fprintf(fd,  "%-10s %i", "minGeoDup", self->minGeoDup);
  fprintf(fd, "\n");

  if (!isEmptyRing(self->servers)) {
    if (!rgSort(self->servers, cmpServer)) goto error;
    while ((server = rgNext_r(self->servers, &curr))) {
      if (!serializeServer(server, fd)) goto error;
    }
  }

  // emacs mode
  fprintf(fd, "\n# Local Variables:\n"
	  "# mode: conf\n"
	  "# mode: font-lock\n"
	  "# End:\n");

  fflush(fd);
  rc = TRUE;
 error:
  if (fd != stdout) {
    if (!unLock(fileno(fd))) rc = FALSE;
    if (fclose(fd)) {
      logMemory(LOG_ERR, "fclose fails: %s", strerror(errno));
      rc = FALSE;
    }
  }
  if (!logoutUser(uid)) rc = FALSE;
  return rc;
}


/*=======================================================================
 * Function   : getImage
 * Description: Find an image
 * Synopsis   : Image* getImage(Collection* coll, 
 *                              Server* server, Archive* archive)
 * Input      : Collection* coll : where to find
 *              Server* server : id
 *              Archive* archive : id
 * Output     : Image* : the Image we have found
 =======================================================================*/
Image* 
getImage(Collection* coll, Server* server, Archive* archive)
{
  Image* rc = 0;
  RGIT* curr = 0;

  checkCollection(coll);
  checkServer(server);
  logMemory(LOG_DEBUG, "getImage %s, %s:%i, %s:%lli",
	  coll, server->host, server->mdtxPort, archive->hash,
	  (long long int)archive->size);
  
  // look for image
  while ((rc = rgNext_r(server->images, &curr)))
    if (!cmpArchive(&rc->archive, &archive)) break;

 error:
  return rc;
}

/*=======================================================================
 * Function   : addImage
 * Description: Add an image if not already there
 * Synopsis   : Image* getImage(Collection* coll, 
 *                              Server* server, Archive* archive)
 * Input      : Collection* coll : where to add
 *              Server* server : id
 *              Archive* archive : id
 * Output     : Image* : the Image we have found/added
 =======================================================================*/
Image* 
addImage(Collection* coll, Server* server, Archive* archive)
{
  Image* rc = 0;
  Image* image = 0;

  checkCollection(coll);
  checkServer(server);
  logMemory(LOG_DEBUG, "addImage %s, %s:%i, %s:%lli",
	  coll, server->host, server->mdtxPort, archive->hash,
	  (long long int)archive->size);

  // already there
  if ((image = getImage(coll, server, archive))) goto end;

  // add new one if not already there
  if ((image = createImage()) == 0) goto error;
  image->archive = archive;
  image->server = server;

  // add image to server
  if (!rgInsert(server->images, image)) goto error;
    
  // add image to archive
  if (!rgInsert(archive->images, image)) goto error;

  // add archive to serverTree, if not already there
  if (!rgHaveItem(coll->serverTree->archives, archive)) {
    if (!rgInsert(coll->serverTree->archives, archive)) goto error;
  }

 end:
  rc = image;
 error:
  if (!rc) {
    logMemory(LOG_ERR, "fails to add an image");
    if (image) delImage(coll, image);
  }
  return rc;
}

/*=======================================================================
 * Function   : delImage
 * Description: Del an image if not already there
 * Synopsis   : void delImage(Collection* coll, Image* image)
 * Input      : Collection* coll : where to del
 *              Image* image : the image to del
 * Output     : TRUE on success
 * Note       :
 *     Non sens: only delArchive and delServer can call it.
 *     from  delServer => O(n2)
 =======================================================================*/
int
delImage(Collection* coll, Image* image)
{
  int rc = FALSE;
  RGIT* curr = 0;
 
  checkCollection(coll);
  checkImage(image);
  logMemory(LOG_DEBUG, "delImage %s, %s:%i, %s:%lli",
	  coll, image->server->host, image->server->mdtxPort, 
	  image->archive->hash, (long long int)image->archive->size);
  logMemory(LOG_DEBUG, "delImage");

  // delete image from server
  if ((curr = rgHaveItem(image->server->images, image))) {
    rgRemove_r(image->server->images, &curr);
  }

  // delete image from archive
  if ((curr = rgHaveItem(image->archive->images, image))) {
    rgRemove_r(image->archive->images, &curr);
  }

  // delete archive from serverTree (if last related image)
  if (isEmptyRing(image->archive->images)) {
    if ((curr = rgHaveItem(coll->serverTree->archives, image->archive))) {
      rgRemove_r(coll->serverTree->archives, &curr);
    }
  }

  // free the image
  image = destroyImage(image);
  rc = TRUE;
 error:
 if (!rc) {
    logMemory(LOG_ERR, "delImage fails");
  }
  return rc;
}


/*=======================================================================
 * Function   : getServer
 * Description: Find an server
 * Synopsis   : Server* getServer(Collection* coll, 
                                    char* hash, off_t size)
 * Input      : Collection* coll : where to find
 *              char* fingerPrint : the user key fingerprint 
 * Output     : Server* : the Server we have found
 =======================================================================*/
Server* 
getServer(Collection* coll, char* fingerPrint)
{
  Server* rc = 0;
  RGIT* curr = 0;

  checkCollection(coll);
  checkLabel(fingerPrint);
  logMemory(LOG_DEBUG, "getServer %s", fingerPrint);
  
  // look for server
  while ((rc = rgNext_r(coll->serverTree->servers, &curr)))
    if (!strncmp(rc->fingerPrint, fingerPrint, MAX_SIZE_MD5)) break;

 error:
  return rc;
}

/*=======================================================================
 * Function   : addServer
 * Description: Add an server if not already there
 * Synopsis   : Server* getServer(Collection* coll, 
 *                                  char* hash, off_t size)
 * Input      : Collection* coll : where to add
 *              char* fingerPrint : the user key fingerprint 
 * Output     : Server* : the Server we have found/added
 =======================================================================*/
Server* 
addServer(Collection* coll, char* fingerPrint)
{
  Server* rc = 0;
  Server* server = 0;

  checkCollection(coll);
  checkLabel(fingerPrint);
  logMemory(LOG_DEBUG, "addServer %s", fingerPrint);
  
  // already there
  if ((server = getServer(coll, fingerPrint))) goto end;

  // add new one if not already there
  if (!(server = createServer())) goto error;
  strncpy(server->fingerPrint, fingerPrint, MAX_SIZE_MD5);
  server->fingerPrint[MAX_SIZE_MD5] = (char)0; // developpement code
  if (!rgInsert(coll->serverTree->servers, server)) goto error;

  // shorter than a function even if not used a lot
  server->isLocalhost = 
    !strncmp(fingerPrint, coll->userFingerPrint, MAX_SIZE_MD5);

 end:
  rc = server;
 error:
  if (!rc) {
    logMemory(LOG_ERR, "addServer fails");
    server = destroyServer(server);
  }
  return rc;
}

/*=======================================================================
 * Function   : delServer
 * Description: Del an server if not already there
 * Synopsis   : void delServer(Collection* coll, Server* server)
 * Input      : Collection* coll : where to del
 *              Server* server : the server to del
 * Output     : TRUE on success
 =======================================================================*/
int
delServer(Collection* coll, Server* server)
{
  int rc = FALSE;
  Image* img = 0;
  //Record* rec = 0;
  RGIT* curr = 0;

  checkCollection(coll);
  checkServer(server);
  logMemory(LOG_DEBUG, "delServer %s", server->fingerPrint);

  // delete from coll if localhost 
  if (coll->localhost == server) coll->localhost = 0;

  // delete master (needed by mdtxDelCollection to serialize)
  if (coll->serverTree->master == server) coll->serverTree->master = 0;
  
  // delete images own by the this server
  while ((img = rgHead(server->images)))
    if (!delImage(coll, img)) goto error;

  // delete records
  //while ((rec = rgHead(server->records)))
  //  if (!delRecord(coll, rec)) goto error;

  // delete server from collection ring
  if ((curr = rgHaveItem(coll->serverTree->servers, server))) {
    rgRemove_r(coll->serverTree->servers, &curr);
  }

  // free the server
  server = destroyServer(server);
  rc = TRUE;
 error:
  return rc;
}

/*=======================================================================
 * Function   : diseaseServer
 * Description: Disease an server if not already there
 * Synopsis   : void diseaseServer(Collection* coll, Server* server)
 * Input      : Collection* coll : where to disease
 *              Server* server : the server to disease
 * Output     : TRUE on success
 =======================================================================*/
int
diseaseServer(Collection* coll, Server* server)
{
  int rc = FALSE;
  Image* image = 0;
  RGIT* curr = 0;

  checkCollection(coll);
  checkServer(server);
  logMemory(LOG_DEBUG, "disease %s server", server->fingerPrint);

  // delete images own by the this server
  while ((image = rgHead(server->images))) {

    // delete archive from serverTree
    if ((curr = rgHaveItem(coll->serverTree->archives, image->archive))) {
      rgRemove_r(coll->serverTree->archives, &curr);
    }

    // delete image from archive
    if ((curr = rgHaveItem(image->archive->images, image))) {
      rgRemove_r(image->archive->images, &curr);
    }

    // try to disease archive
    if (!diseaseArchive(coll, image->archive)) goto error;    

    // delete image from server
    curr = server->images->head;
    rgRemove_r(server->images, &curr);
    
    // free the image
    destroyImage(image);
  }
  
  // delete networks and gateways
  rgDelete(server->networks);
  rgDelete(server->gateways);

  // disease fields
  server->label = destroyString(server->label);
  server->user = destroyString(server->user);
  server->comment = destroyString(server->comment);
  server->lastCommit = 0;
  server->mdtxPort = 0;
  server->sshPort = 0;
  server->wwwPort = 0;
  server->userKey = destroyString(server->userKey);
  server->hostKey = destroyString(server->hostKey);
  server->host[0] = (char)0;
  server->cacheSize = 0;
  server->cacheTTL = 0;
  server->queryTTL = 0;
  
  rc = TRUE;
 error:
  if (!rc) logMemory(LOG_ERR, "diseaseServer fails");
  return rc;
}

/*=======================================================================
 * Function   : diseaseServerTree
 * Description: Disease an serverTree if not already there
 * Synopsis   : void diseaseServerTree(Collection* coll)
 * Input      : Collection* coll : where to disease
 * Output     : TRUE on success
 =======================================================================*/
int
diseaseServerTree(Collection* coll)
{
  int rc = FALSE;
  ServerTree *self = 0;
  Server *server = 0;
  RGIT* curr = 0;
 
  checkCollection(coll);
  if((self = coll->serverTree) == 0) goto error;
  logMemory(LOG_DEBUG, "disease %s servers", coll->label);

  // disease servers
  curr = 0;
  while ((server = rgNext_r(self->servers, &curr))) {
    if (!diseaseServer(coll, server)) goto error;
  }

  rc = TRUE;
 error:
  if (!rc) logMemory(LOG_ERR, "diseaseServerTree fails");
  return rc;
}

/*=======================================================================
 * Function   : isReachable
 * Description: test if "from" server may use gateways to reach "to" 
 *              server
 * Synopsis   : int isReachable(Collection* coll, 
 *                              Server* from, Server* to, int* itIs)
 * Input      : Collection* coll : where to add
 *              Server* from: the source server
 *              Server* to:   the destination server
 *              int* itIs:    TRUE if reachable
 * Output     : TRUE on success
 * Note       : this function is not symmetric
 =======================================================================*/
int 
isReachable(Collection* coll, Server* from, Server* to, int* itIs)
{
  int rc = FALSE;
  int loop1 = TRUE;
  RG* reach = 0;
  void* it = 0;
  RG* tmp = 0;
  RGIT* curr = 0;
  Server* server = 0;
  int nbReachable = 0;

  logMemory(LOG_DEBUG, "isReachable from %s to %s",
	  from->fingerPrint, to->fingerPrint);

  *itIs = FALSE;
  if (from == to) {
    *itIs = TRUE;
    goto end;
  }

  // networks we can reach
  if (!(reach = createRing())) goto error;
  while ((it = rgNext_r(from->networks, &curr))) {
    if (!rgInsert(reach, it)) goto error;
  }

  if (LOG_DEBUG < env.logHandler->severity[LOG_MEMORY]->code) {
    logMemory(LOG_DEBUG, "we want to reach on of them:");
    while ((it = rgNext(to->networks))) {
      logMemory(LOG_DEBUG, "- %s", (char*)it);
    }
  }

  // loop until "to" is located under a reachable network
  while (loop1) {
    nbReachable = reach->nbItems;

    if (LOG_DEBUG < env.logHandler->severity[LOG_MEMORY]->code) {
      logMemory(LOG_DEBUG, "now we can reach:");
      while ((it = rgNext(reach))) {
	logMemory(LOG_DEBUG, "- %s", (char*)it);
      }
    }

    // exit when reachable
    if (tmp) tmp = destroyOnlyRing(tmp);
    if (!(tmp = rgInter(reach, to->networks))) goto error;
    if (!isEmptyRing(tmp)) {
      *itIs = TRUE;
      goto end;
    }

    // look for a (only one) new gateway
    loop1 = FALSE;
    curr = 0; 
    while (!loop1 && 
	   (server = rgNext_r(coll->serverTree->servers, &curr))) {
      tmp = destroyOnlyRing(tmp);
      if (!(tmp = rgInter(reach, server->gateways))) goto error;
      if (!isEmptyRing(tmp)) {

	logMemory(LOG_DEBUG, "find a gateway: %s", server->fingerPrint);

	// we find a new gateway: we can reach its networks
	tmp = destroyOnlyRing(tmp);
	if (!(tmp = rgUnion(reach, server->networks))) goto error;
	reach = destroyOnlyRing(reach);
	reach = tmp;
	tmp = 0;

	// but is there new networks ?
	loop1 = (reach->nbItems > nbReachable);
      }
    }
  }

 end:
  logMemory(LOG_INFO, "%s can%s reach %s",
	  from->fingerPrint, *itIs?"":"not", to->fingerPrint);
  rc = TRUE;
 error:
  if (!rc) {
    logMemory(LOG_ERR, "isReachable fails");
  }
  reach = destroyOnlyRing(reach);
  if (tmp) tmp = destroyOnlyRing(tmp);
  return rc;
}


/* Local Variables: */
/* mode: c */
/* mode: font-lock */
/* mode: auto-fill */
/* End: */
