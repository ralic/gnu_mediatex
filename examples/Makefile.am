#  * Project: MediaTex
#  * Module:  use case exemples
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

examplesdir = $(datarootdir)@mdtx_mediatexdir@/examples
dist_examples_DATA = cake.tgz seda-0.2.tgz
dist_examples_SCRIPTS = tests.sh example.sh

uninstall-hook:
	rmdir $(examplesdir) 2>/dev/null || true
