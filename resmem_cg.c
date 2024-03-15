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
static inline int cgmem(char *cg, char *name, unsigned long *out)
{
	char fn[FNAMELEN];
	int ret;
	char buf[2048];

	snprintf(fn, FNAMELEN, "%s/%s", cg, name);

	ret = file_to_buf(fn, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	if (strncmp(buf, "max", 3) == 0) {
		ret = get_info_infile(MEMINFO_FILE, "MemTotal:", out);
		if (ret < 0)
			return ret;
		/*
		 * MemTotal from /proc/meminfo returns value in kB
		 * Convert to bytes
		 */
		*out *= 1000;
		return ret;
	}
	*out = strtoul(buf, NULL, 10);
	return 0;
}

static inline int cgmemval(char *cg, char *name, char *elem,
			   unsigned long *out)
{
	char fn[FNAMELEN];

	snprintf(fn, FNAMELEN, "%s/%s", cg, name);

	return (get_info_infile(fn, elem, out));
}

static inline int cg_free_mem(char *cg, unsigned long *out)
{
	unsigned long mcurr;
	int err;

	if ((err = cgmem(cg, "memory.max", out)) < 0)
		return err;
	if ((err = cgmem(cg, "memory.current", &mcurr)) < 0)
		return err;
	*out -= mcurr;
	return 0;
}

static inline int cg_free_swap(char *cg, unsigned long *out)
{
	unsigned long scurr;
	int err;

	if ((err = cgmem(cg, "memory.swap.max", out)) < 0)
		return err;
	if ((err = cgmem(cg, "memory.swap.current", &scurr)) < 0)
		return err;
	*out -= scurr;
	return 0;
}

static inline int cg_active(char *cg, unsigned long *out)
{
	unsigned long afile;
	int err;

	if ((err = cgmemval(cg, "memory.stat", "active_anon", out)) < 0)
		return err;
	if ((err = cgmemval(cg, "memory.stat", "active_file", &afile)) < 0)
		return err;
	*out += afile;
	return 0;
}

static inline int cg_inactive(char *cg, unsigned long *out)
{
	unsigned long ifile;
	int err;

	if ((err = cgmemval(cg, "memory.stat", "inactive_anon", out)) < 0)
		return err;
	if ((err = cgmemval(cg, "memory.stat", "inactive_file", &ifile)) < 0)
		return err;
	*out += ifile;
	return 0;
}

/* Get all memory info for a cgroup */
static int getmeminfoall(char *cg, void *out)
{
	res_mem_infoall_t *memall = out;

	cg_free_mem(cg, &memall->memfree);
	cgmem(cg, "memory.max", &memall->memtotal);
	cgmem(cg, "memory.current", &memall->memavailable);
	cg_active(cg, &memall->active);
	cg_inactive(cg, &memall->inactive);
	cgmem(cg, "memory.swap.max", &memall->swaptotal);
	cg_free_swap(cg, &memall->swapfree);
	return 0;
}

static inline int cg_res_val(int res_id, char *cg, unsigned long *out)
{
	switch (res_id) {
	case RES_MEM_FREE:
		return cg_free_mem(cg, out);

	case RES_MEM_AVAILABLE:
		return cgmem(cg, "memory.current", out);
		break;

	case RES_MEM_TOTAL:
		return cgmem(cg, "memory.max", out);

	case RES_MEM_ACTIVE:
		return cg_active(cg, out);

	case RES_MEM_INACTIVE:
		return cg_inactive(cg, out);

	case RES_MEM_SWAPTOTAL:
		return cgmem(cg, "memory.swap.max", out);

	case RES_MEM_SWAPFREE:
		return cg_free_swap(cg, out);
	}
	return -1;
}

/*
 * Returns: 0 on success, <0 if error
 */
int getmeminfo_cg(int res_id, void *out, size_t sz, void **hint, int pid,
		  int flags)
{
	char *cg;
	int err;

	cg = get_cgroup(pid, MEMCGNAME);
	if (!cg)
		return -1;

	//clean_init(cg);

	if (res_id == RES_MEM_INFOALL) {
		CHECK_SIZE(sz, sizeof(res_mem_infoall_t));
		err = getmeminfoall(cg, out);
		free(cg);
		return err;
	}

	CHECK_SIZE(sz, sizeof(unsigned long));
	err = cg_res_val(res_id, cg, (unsigned long *)out);
	free(cg);
	return err;
}

int populate_meminfo_cg(res_blk_t *res, int pid, int flags)
{
	char *cg;
	int err;

	cg = get_cgroup(pid, MEMCGNAME);
	if (!cg)
		return -1;

	//clean_init(cg);

	for (int i = 0; i < res->res_count; i++) {
		err = cg_res_val(res->res_unit[i]->res_id, cg,
				 &((res->res_unit[i]->data).sz));
		res->res_unit[i]->status = err;
	}
	free(cg);
	return 0;
}
