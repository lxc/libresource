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

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = CONTRIBUTORS.md README.md

include_HEADERS = resource.h

noinst_HEADERS = resource_impl.h resmem.h resnet.h resproc.h

lib_LTLIBRARIES = libresource.la

SOURCES_ALL = \
	resource.c resmem.c resnet.c resproc.c \
	resource.h resource_impl.h resmem.h resnet.h resproc.h

libresource_la_SOURCES = ${SOURCES_ALL}
libresource_la_CFLAGS = ${AM_CFLAGS}
libresource_la_LDFLAGS = -version-number \
	$(VERSION_MAJOR):$(VERSION_MINOR):$(VERSION_MICRO)
