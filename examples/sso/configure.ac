dnl  * Project: mediatex's sso
dnl  * Module: autotools
dnl  *
dnl  * Process this file with autoconf to produce a configure script.
dnl 
dnl  MdtxSso is a single-sign-one for mediatex
dnl  Copyright (C) 2017 Nicolas Roche
dnl 
dnl  This program is free software: you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation, either version 3 of the License, or
dnl  any later version.
dnl 
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl 
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT([mdtxSso], [0.1], [nroche@narval.fr.eu.org])
AM_INIT_AUTOMAKE()

AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

AC_PREREQ

AC_CONFIG_FILES([
	Makefile
	doc/mdtxSso-figures/Makefile
	doc/Makefile
	])

# Documentation figures (doc/mdtxSso-figures/Makefile.am)
EPS_FILES=$(for IMG in $(ls $srcdir/doc/mdtxSso-figures/*.fig); do \
		IMG=${IMG#$srcdir/doc/mdtxSso-figures/}; \
		echo -n "${IMG%.fig}.eps "; \
		done)
PNG_FILES=$(for IMG in $(ls $srcdir/doc/mdtxSso-figures/*.fig); do \
		IMG=${IMG#$srcdir/doc/mdtxSso-figures/}; \
		echo -n "${IMG%.fig}.png "; \
		done)
AC_SUBST([EPS_FILES]) 
AC_SUBST([PNG_FILES]) 

AC_OUTPUT
