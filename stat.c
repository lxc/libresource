/* Copyright (C) 2023, Oracle and/or its affiliates. All rights reserved
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
#include "resvm.h"
#include "resource_impl.h"
#include <errno.h>
#include <libgen.h>
#include "stat.h"

static char buffer[20000];

static int populate_statcpu(struct cpu_stat *all_cpu, int cpu_num)
{
	char buf[36];
	const char *cstr;

	if (!all_cpu) {
		return -1;
	}
	for (int i = 0; i < cpu_num; i++) {
		snprintf(buf, 36, "cpu%d", i);
		cstr = strstr(buffer, buf);
		if (cstr) {
			cstr += strlen(buf)+1;
			sscanf(cstr, "%Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
					&all_cpu->user,
					&all_cpu->nice,
					&all_cpu->system,
					&all_cpu->idle,
					&all_cpu->iowait,
					&all_cpu->irq,
					&all_cpu->softirq,
					&all_cpu->steal,
					&all_cpu->guest,
					&all_cpu->guest_nice);
		}
		all_cpu++;
	}
	return 0;
}

static int populate_statinfo(void *out, int test)
{
	char *cstr;
	struct stat_info *st;
	int cpu_num;

	st = (struct stat_info *) out;
	cstr = strstr(buffer, "cpu ");
	if (cstr) {
		sscanf(cstr, "cpu  %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
				&st->cpu.user,
				&st->cpu.nice,
				&st->cpu.system,
				&st->cpu.idle,
				&st->cpu.iowait,
				&st->cpu.irq,
				&st->cpu.softirq,
				&st->cpu.steal,
				&st->cpu.guest,
				&st->cpu.guest_nice);
	}
	cstr = strstr(buffer, "intr ");
	memset(&st->intr, '\0', sizeof(st->intr));
	if (cstr) {
		unsigned int bytes = 0;
		char *cstr1 = cstr + 5;
		while (*cstr1 != '\n' && bytes < 8999) {
			cstr1++;
			bytes++;
		}
		strncpy(st->intr, cstr+5, bytes);
	}
	cstr = strstr(buffer, "ctxt ");
	if (cstr)
		sscanf(cstr, "ctxt %Lu", &st->ctxt);
	cstr = strstr(buffer, "btime ");
	if (cstr)
		sscanf(cstr, "btime %Lu", &st->btime);
	cstr = strstr(buffer, "processes ");
	if (cstr)
		sscanf(cstr, "processes %Lu", &st->processes);
	cstr = strstr(buffer, "procs_running ");
	if (cstr)
		sscanf(cstr, "procs_running %Lu", &st->procs_running);
	cstr = strstr(buffer, "procs_blocked ");
	if (cstr)
		sscanf(cstr, "procs_blocked %Lu", &st->procs_blocked);
	cstr = strstr(buffer, "softirq ");
	memset(&st->softirq, '\0', sizeof(st->softirq));
	if (cstr) {
		unsigned int bytes = 0;
		char *cstr1 = cstr + 8;
		while (*cstr1 != '\n' && bytes < 8999) {
			cstr1++;
			bytes++;
		}
		strncpy(st->softirq, cstr+8, bytes);
	}

	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	populate_statcpu(st->all_cpu, cpu_num);
	return 0;
}

int getstatexist(int res_id, void *exist, size_t sz, void *hint, int flags)
{
	int ret;
	ret = file_to_buf("./stat_info.orig", buffer, sizeof(buffer));
	if (ret < 0)
		return ret;
	return (populate_statinfo(exist, 1));
}

int getstatinfo(int res_id, void *out, size_t sz, void **hint, int flags)
{
	int ret, cpu_num, cpu_size;

#ifdef TESTING
	ret = file_to_buf("./stat_info.orig", buffer, sizeof(buffer));
#else	
	ret = file_to_buf(STAT_FILE, buffer, sizeof(buffer));
#endif
	if (ret < 0)
		return ret;

	switch (res_id) {
	case RES_STAT_INFO:
		cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
		cpu_size = sizeof(struct cpu_stat) * cpu_num;
		CHECK_SIZE(sz, (sizeof(struct stat_info) + cpu_size));
		ret = populate_statinfo(out, 0);
		break;
        default:
                eprintf("Resource Id is invalid");
                errno = EINVAL;
                return -1;

	}
	return ret;
}
