#  * Project: MediaTex
#  * Module:  inital configuration files (and testing/exemple purpose)
#  *
#  * Process this file with configure to produce a Makefile.
# 
#  MediaTex is an Electronic Records Management System
#  Copyright (C) 2014 2015 2016 2017 Nicolas Roche
# 
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

miscdir = $(datarootdir)@mdtx_mediatexdir@/misc
dist_misc_DATA = \
	cron mediatex.conf \
	supports.txt catalog000.txt extract000.txt servers.txt \
	index.htaccess cache.htaccess score.htaccess cgi.htaccess \
	home.htaccess apache-mdtx.conf htgroup \
	mediatex.css floppy-icon.png \
	put.shtml put.js put.sh \
	logo logo.png \
	logoP1.iso logoP2.iso logoP1.cat logoP2.cat\
	logo.tgz logo.tar.bz2 logo.afio \
	logo.tar logo.cpio.gz logo.cpio logo.cpio.bz2 \
	logo.zip logo.part1.rar logo.part2.rar

uninstall-hook:
	rmdir $(miscdir) 2>/dev/null || true
