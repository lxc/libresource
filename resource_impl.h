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
#include <stdlib.h>

#define RESOURCE_64	64
#define RESOURCE_256	256
#define RESOURCE_2048	2048

#define FNAMELEN	RESOURCE_256

#define INITSCOPE	"/init.scope"

/* We are assuming that cgroup is mounted at its usual location,
 * If we have reeust from applications who do not mount cgroup at
 * usual location, then we need to change this.
 */
#define DEFAULTCGFS	"/sys/fs/cgroup"

#define eprintf(msg, ...)	fprintf(stderr,\
					"Err at line %d in file %s: "msg"\n",\
					__LINE__, __FILE__, ##__VA_ARGS__)\

#define libres_iterate_parts(__iterator, __str, __separators)		   \
	for (char *__p = NULL, *__it = strtok_r(__str, __separators, &__p);\
		(__iterator = __it);                                       \
		__iterator = __it = strtok_r(NULL, __separators, &__p))

#define CHECK_SIZE(sz, req_sz)                                          \
        if (sz < req_sz) {                                              \
                eprintf("memory (%ld) is not enough to hold data (%ld)",\
                sz, req_sz);                                            \
                errno = ENOMEM;                                         \
                return -1;                                              \
        }

/* Helper function to skip first n spaces from string s and return
 * pointer to string after that. If len is reached before we get n spaces,
 * null is returned.
 */
static inline char *skip_spaces(char *s, size_t len, int n) {
	size_t i = 0;
	int spacenum = 0;
	while(i++ < len) {
		if (*s++ == ' ') {
			spacenum++;
			if (spacenum == n) {
				return s;
			}
		}
	}
	return NULL;
}

static inline void clean_init(char *cg)
{
	char *p;
	size_t cg_len = strlen(cg), init_len = strlen(INITSCOPE);

	if (cg_len < init_len)
		return;

	p = cg + cg_len - init_len;
	if (strcmp(p, INITSCOPE) == 0) {
		if (p == cg)
			*(p+1) = '\0';
		else
			*p = '\0';
	}
}

/* Find a cgroup controller in list of comma(,) seperated cgroup controller
 * list.
 */
static inline int controller_in_clist(char *cgline, const char *c)
{
	char *tok;
	libres_iterate_parts(tok, cgline, ",")
		if (strcmp(tok, c) == 0)
			return 1;
	return 0;
}

/* Get cgroup path for a particular controller */
static inline char *get_cgroup(pid_t pid, const char *contrl)
{
	char fn[FNAMELEN];
	FILE *f;
	char *line = NULL;
	size_t len = 0;
	size_t l = 0;
	int ret;
	char *cgrp = NULL;
	char *c1, *c2;

	/* If no pid is provided then return cgroup info for current process.
	 */
	if (pid) {
		ret = snprintf(fn, FNAMELEN, "/proc/%d/cgroup", pid);
	} else {
		ret = snprintf(fn, FNAMELEN, "/proc/self/cgroup");
	}

	if (ret < 0 || ret >= FNAMELEN)
		return NULL;

	if (!(f = fopen(fn, "r")))
		return NULL;

	while (getline(&line, &len, f) != -1) {
		if (!line[0])
			continue;
		c1 = strchr(line, ':');
		if (!c1)
			goto out;
		c1++;

		c2 = strchr(c1, ':');
		if (!c2)
			goto out;
		*c2 = '\0';
		if (!controller_in_clist(c1, contrl))
			continue;
		c2++;
		l = strlen(c2);
		if (l && c2[l-1] == '\n')
			c2[l-1] = '\0';
		if (strcmp(c2, "/") == 0)
			goto out;
		cgrp = strdup(c2);
		break;
	}

out:
	fclose(f);
	free(line);
	return cgrp;
}

static inline int file_to_buf(char *fname, char *buf, unsigned int bufsz)
{
	int fd;
	size_t rdsz;
	int err;

	fd = open(fname, O_RDONLY);
	if (fd == -1) {
		err = errno;
		eprintf("Error opening File %s with errno: %d",
			fname, errno);
		return -err;
	}

	rdsz = read(fd, buf, bufsz - 1);
	if (rdsz < 0) {
		err = errno;
		eprintf("in read from File %s with errno: %d", fname, errno);
		close(fd);
		return -err;
	}
	buf[rdsz] = '\0';
	close(fd);
	return rdsz;
}

#endif /* _RESOURCE_IMPL_H */
