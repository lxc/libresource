/* Copyright (C) 2024, Oracle and/or its affiliates. All rights reserved
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
#ifndef _RESOURCE_H
#include "resource.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "resmem.h"
#include "resource_impl.h"
#include <errno.h>
#include <libgen.h>

static int cg_error;

/* read a specific information from file on the basis of a string.
 * String should tell what information is being read.
 */
int get_info_infile(char *fname, char *res, void *out)
{
	const char *loc;
	char buf[MEMBUF_2048];
	int err;

	if ((err = file_to_buf(fname, buf, MEMBUF_2048)) < 0)
		return err;

	loc = strstr(buf, res);
	if (loc == NULL) {
		eprintf("%s is not found in buffer of file %s", res, fname);
		return -ENODATA;
	}

	sscanf(loc, "%*s%zu", (size_t *)out);
	return 0;
}

/* read information from a cgroup file.
 */
static inline size_t cgmemread(char *cg, char *name, char *elem)
{
	char fn[FNAMELEN];
	unsigned long ret;

	snprintf(fn, FNAMELEN, "%s/%s%s/%s", DEFAULTCGFS,
		MEMCGNAME, cg, name);

	cg_error = get_info_infile(fn, elem, &ret);

	return ret / 1024;
}

/* Get all memory info for a cgroup */
static int getmeminfoall(char *cg, void *out)
{
	res_mem_infoall_t *memall = out;

	memall->memfree = (size_t)
				(cgmemread(cg, "memory", "limit_in_bytes")
				 	-
				 cgmemread(cg, "memory", "usage_in_bytes"));
	memall->memavailable = (size_t) 
				(cgmemread(cg, "memory", "limit_in_bytes")
				 	-
				 cgmemread(cg, "memory", "usage_in_bytes")
					+
				 cgmemread(cg, "memory.stat", "cache"));
	memall->memtotal = (size_t) cgmemread(cg, "memory", "limit_in_bytes");
	memall->active = (size_t) (cgmemread(cg, "memory", "\nactive_anon")
					+
				   cgmemread(cg, "memory", "\nactive_file"));
	memall->inactive = (size_t) (cgmemread(cg, "memory", "\ninactive_anon") 
					+
				     cgmemread(cg, "memory", "\ninactive_file"));
	memall->swaptotal = (size_t) 
				cgmemread(cg, "memory", "memsw.limit_in_bytes");
	memall->swapfree = (size_t)
				(cgmemread(cg, "memory", "memsw.limit_in_bytes")
				 	-
				 cgmemread(cg, "memory", "memsw.usage_in_bytes"));
	return cg_error;
}

/*
 * Returns: -1 on error, 0 on success
 */
int getmeminfo_cg(int res_id, void *out, size_t sz, void **hint, int pid, int flags)
{
	char *cg;

	cg = get_cgroup(pid, MEMCGNAME);
	if (!cg) {
		printf("get_cgroups error\n");
		return -1;
	}

	clean_init(cg);

	switch (res_id) {
	case RES_MEM_FREE:
		*(size_t *)out = (size_t) 
				(cgmemread(cg, "memory", "limit_in_bytes") 
				 	-
				 cgmemread(cg, "memory", "usage_in_bytes"));
		break;

	case RES_MEM_AVAILABLE:
		*(size_t *)out = (size_t) 
				(cgmemread(cg, "memory", "limit_in_bytes") 
				 	-
				 cgmemread(cg, "memory", "usage_in_bytes")
				 	+
				 cgmemread(cg, "memory.stat", "cache"));
		break;

	case RES_MEM_TOTAL:
		*(size_t *)out = (size_t) 
				  cgmemread(cg, "memory", "limit_in_bytes");
		break;

	case RES_MEM_ACTIVE:
		*(size_t *)out = (size_t) (cgmemread(cg, "memory", "\nactive_anon")
						+
					   cgmemread(cg, "memory", "\nactive_file"));
		break;

	case RES_MEM_INACTIVE:
		*(size_t *)out = (size_t) (cgmemread(cg, "memory", "\ninactive_anon") +
					   cgmemread(cg, "memory", "\ninactive_file"));
		break;

	case RES_MEM_SWAPTOTAL:
		*(size_t *)out = (size_t)
				cgmemread(cg, "memory", "memsw.limit_in_bytes");
		break;

	case RES_MEM_SWAPFREE:
		*(size_t *)out = (size_t)
				(cgmemread(cg, "memory", "memsw.limit_in_bytes")
					-
				 cgmemread(cg, "memory", "memsw.usage_in_bytes"));
		break;

	case RES_MEM_INFOALL:
		return (getmeminfoall(cg, out));
	}

	return cg_error;
}

int populate_meminfo_cg(res_blk_t *res, int pid, int flags)
{
	char *cg;

	cg = get_cgroup(pid, MEMCGNAME);
	if (!cg) {
		printf("%s: get_cgroups error\n", __FUNCTION__);
		return -1;
	}

	clean_init(cg);

	for (int i = 0; i < res->res_count; i++) {
		switch (res->res_unit[i]->res_id) {
		case RES_MEM_FREE:
			(res->res_unit[i]->data).sz = (size_t)
				(cgmemread(cg, "memory", "limit_in_bytes") 
				 	-
				 cgmemread(cg, "memory", "usage_in_bytes"));
			break;

		case RES_MEM_AVAILABLE:
			(res->res_unit[i]->data).sz = (size_t) 
				(cgmemread(cg, "memory", "limit_in_bytes") 
				 	-
				 cgmemread(cg, "memory", "usage_in_bytes")
				       	+
				 cgmemread(cg, "memory.stat", "cache"));
			break;

		case RES_MEM_TOTAL:
			(res->res_unit[i]->data).sz = (size_t) 
				cgmemread(cg, "memory", "limit_in_bytes");
			break;

		case RES_MEM_ACTIVE:
			(res->res_unit[i]->data).sz = (size_t) 
				(cgmemread(cg, "memory.stat", "\nactive_anon")
				 	+
				 cgmemread(cg, "memory.stat", "\nactive_file"));
			break;

		case RES_MEM_INACTIVE:
			(res->res_unit[i]->data).sz = (size_t) 
				(cgmemread(cg, "memory.stat", "\ninactive_anon")
				 	+
				 cgmemread(cg, "memory.stat", "\ninactive_file"));
			break;

		case RES_MEM_SWAPTOTAL:
			(res->res_unit[i]->data).sz = 
				cgmemread(cg, "memory", "memsw.limit_in_bytes");
			break;

		case RES_MEM_SWAPFREE:
			(res->res_unit[i]->data).sz = 
				(cgmemread(cg, "memory", "memsw.limit_in_bytes")
					-
				 cgmemread(cg, "memory", "memsw.usage_in_bytes"));
			break;
		}
		res->res_unit[i]->status = cg_error;
	}
	return 0;
}
