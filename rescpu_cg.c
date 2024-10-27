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
#include "rescpu.h"
#include "resource_impl.h"
#include <errno.h>
#include <libgen.h>

static int get_info_infile(char *fname, char *res, void *out)
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

static int cgcpuval(char *cg, char *name, char *elem, unsigned long *out)
{
	char fn[FNAMELEN];

	snprintf(fn, FNAMELEN, "%s/%s", cg, name);

	return (get_info_infile(fn, elem, out));
}

static int cgcpu(char *cg, char *name, unsigned long *out)
{
	char fn[FNAMELEN];
	char buf[2048];
	int ret;

	snprintf(fn, FNAMELEN, "%s/%s", cg, name);

	ret = file_to_buf(fn, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	*out = strtoul(buf, NULL, 10);
	return 0;
}

static int get_cpu_max(char *cg, char *name, unsigned long *out)
{
	char buf[MEMBUF_2048];
	char fn[FNAMELEN];
	int err;

	snprintf(fn, FNAMELEN, "%s/%s", cg, name);

	if ((err = file_to_buf(fn, buf, MEMBUF_2048)) < 0)
		return err;

	sscanf(buf, "%*s%zu", (size_t *)out);
	return 0;
}

static int cg_res_val(int res_id, char *cg, unsigned long *out)
{
	switch (res_id) {
	case RES_CPU_STAT_USAGE:
		return cgcpuval(cg, "cpu.stat", "usage_usec", out);
	case RES_CPU_STAT_USER:
		return cgcpuval(cg, "cpu.stat", "user_usec", out);
	case RES_CPU_STAT_SYSTEM:
		return cgcpuval(cg, "cpu.stat", "system_usec", out);
	case RES_CPU_WEIGHT:
		return cgcpu(cg, "cpu.weight", out);
	case RES_CPU_WEIGHT_NICE:
		return cgcpu(cg, "cpu.weight.nice", out);
	case RES_CPU_MAX:
		return get_cpu_max(cg, "cpu.max", out);
	}

	return -1;
}

int getcpuinfo_cg(int res_id, void *out, size_t sz, void **hint, int pid,
		  int flags)
{
	char *cg;
	int err;

	cg = get_cgroup(pid, CPUCGNAME);
	if (!cg)
		return -1;

	err = cg_res_val(res_id, cg, (unsigned long *)out);
	free(cg);
	return err;
}

