#!/bin/bash
#set -x
set -e
#=======================================================================
# * Project: MediaTex
# * Module : script libs
# *
# * This script purge a MediaTex collection
#
# MediaTex is an Electronic Records Management System
# Copyright (C) 2014 2015 2016 2017 Nicolas Roche
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#=======================================================================

[ -z $srcdir ] && srcdir=.
[ -z $libdir ] && libdir=$srcdir/lib
[ ! -z $MDTX_SH_INCLUDE ]  || source $libdir/include.sh
[ ! -z $MDTX_SH_USERS ]    || source $libdir/users.sh

Debug "free"
[ $(id -u) -eq 0 ] || Error "need to be root"
[ ! -z $1 ] || Error "expect a label as first parameter"
[ ! "$1" = "mdtx" ] || Error "collection cannot be labeled mdtx"
USER="$MDTX-$1"

# changes into /var/lib and /var/cache
USERS_coll_remove_user $USER

# do not purge the collection repository
if [ -d $GITBARE/$USER ]; then
    Notice "Let you purge '$GITBARE/$USER' repository"
    chown -R root.root $GITBARE/$USER
fi

Info "done"
