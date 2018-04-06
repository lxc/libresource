/* Copyright (C) 2018, Oracle and/or its affiliates. All rights reserved
 *
 * This file is part of libresource.
 *
 * libresource is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * libresource is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libresource. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _RESOURCE_IMPL_H
#define _RESOURCE_IMPL_H

#include <fcntl.h>
#include <errno.h>

#define RESOURCE_64	64
#define RESOURCE_2048	2048

#define eprintf(msg, ...)	fprintf(stderr,\
					"Err at line %d in file %s: "msg"\n",\
					__LINE__, __FILE__, ##__VA_ARGS__)\


static inline int file_to_buf(char *fname, char *buf)
{
	int fd = 0;
	size_t rdsz = 0;
	int err = 0;

	fd = open(fname, O_RDONLY);
	if (fd == -1) {
		err = errno;
		eprintf("in opening File %s with errno: %d", fname, errno);
		errno = err;
		return -1;
	}

	if (lseek(fd, 0L, SEEK_SET) == -1) {
		err = errno;
		eprintf("in lseek for File %s with errno: %d", fname, errno);
		close(fd);
		errno = err;
		return -1;
	}

	rdsz = read(fd, buf, RESOURCE_2048 - 1);
	if (rdsz < 0) {
		err = errno;
		eprintf("in read from File %s with errno: %d", fname, errno);
		close(fd);
		errno = err;
		return -1;
	}
	buf[rdsz] = '\0';
	close(fd);
	return rdsz;
}

#endif /* _RESOURCE_IMPL_H */
