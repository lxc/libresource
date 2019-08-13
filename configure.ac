# Copyright (C) 2018, Oracle and/or its affiliates. All rights reserved
#
# This file is part of libresource.
#
# libresource is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# libresource is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libresource. If not, see <http://www.gnu.org/licenses/>.
#
# This is main file which is starting point to all interfaces provided
# with libresource.

AC_INIT([libresource], [0.1.1])
LT_INIT([shared])
AM_INIT_AUTOMAKE(foreign)
AM_INIT_AUTOMAKE
AC_DISABLE_STATIC
AC_PROG_CC
AC_PROG_CC_STDC
AM_CFLAGS="-g -Wall -Werror -fPIC"
AC_CHECK_HEADERS_ONCE([linux/resource.h])
VERSION_MAJOR=0
VERSION_MINOR=1
VERSION_MICRO=1
AC_SUBST([VERSION_MAJOR])
AC_SUBST([VERSION_MINOR])
AC_SUBST([VERSION_MICRO])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
