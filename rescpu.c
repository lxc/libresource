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
#include "rescpu.h"
#include "resource_impl.h"
#include <errno.h>
#include <libgen.h>
#include <ctype.h>

static int populate_cpuinfo(void *out, char *buffer, int total_len, int probe)
{
	char *end, *start, *start2, *end2;
	char end3[1];
	int bytes_done = 0;
	struct cpuinfo *cpu = (struct cpuinfo *)out;

	start = buffer;
	while(1) {
		if (bytes_done >= total_len)
			break;
		if (*start == '\n') {
			cpu++;
			start++;
			bytes_done++;
		}
		end = strchr(start, ':');
		if (!end)
			break;
		bytes_done += (end-start);
		if (*(end+1) == '\n') {
			start2 = end+1;
			bytes_done++;
			end[0] = '\0';
			end2 = end3;
			end2[0] = '\0';
			goto next;
		}
		end2 = end+2;
		bytes_done += 2;
		if (start != end) {
			while ((*(end-1) == ' ') || (*(end-1) == '\t')) {
				end--;
				if (start == end)
					break;
			}
		}
		end[0] = '\0';
		start2 = strchr(end2, '\n');
		if (!start2)
			break;
		start2[0] = '\0';
		bytes_done += strlen(end2);
		if (start == end || start2 == end2) 
			goto nextline;

next:
		if (!strcmp(start, "processor")) {
			/*
			 * If probe is set, then this is being called during
			 * testing from res_exist(). In that case, set value of
			 * each field to 1 to indicate whether that field exists
			 * in /proc/cpuinfo. This is needed as not all values are
			 * present on all OSes.
			 */
			if (probe)
				cpu->processor = 1;
			else
				cpu->processor = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "vendor_id")) {
			if (probe)
				cpu->vendor_id[0] = 1;
			else
				strcpy(cpu->vendor_id, end2);
		}
		if (!strcmp(start, "cpu family")) {
			if (probe)
				cpu->cpu_family[0] = 1;
			else
				strcpy(cpu->cpu_family, end2);
		}
		if (!strcmp(start, "model")) {
			if (probe)
				cpu->model[0] = 1;
			else
				strcpy(cpu->model, end2);
		}
		if (!strcmp(start, "model name")) {
			if (probe)
				cpu->model_name[0] = 1;
			else
				strcpy(cpu->model_name, end2);
		}
		if (!strcmp(start, "stepping")) {
			if (probe)
				cpu->stepping[0] = 1;
			else
				strcpy(cpu->stepping, end2);
		}
		if (!strcmp(start, "microcode")) {
			if (probe)
				cpu->microcode[0] = 1;
			else
				strcpy(cpu->microcode, end2);
		}
		if (!strcmp(start, "cpu MHz")) {
			if (probe)
				cpu->cpu_mhz[0] = 1;
			else
				strcpy(cpu->cpu_mhz, end2);
		}
		if (!strcmp(start, "cache size")) {
			if (probe)
				cpu->cache_size[0] = 1;
			else
				strcpy(cpu->cache_size, end2);
		}
		if (!strcmp(start, "physical id")) {
			if (probe)
				cpu->physical_id = 1;
			else
				cpu->physical_id = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "siblings")) {
			if (probe)
				cpu->siblings = 1;
			else
				cpu->siblings = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "core id")) {
			if (probe)
				cpu->core_id = 1;
			else
				cpu->core_id = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "cpu cores")) {
			if (probe)
				cpu->cpu_cores = 1;
			else
				cpu->cpu_cores = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "apicid")) {
			if (probe)
				cpu->apicid = 1;
			else
				cpu->apicid = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "initial apicid")) {
			if (probe)
				cpu->initial_apicid = 1;
			else
				cpu->initial_apicid = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "fpu")) {
			if (probe)
				cpu->fpu[0] = 1;
			else
				strcpy(cpu->fpu, end2);
		}
		if (!strcmp(start, "fpu_exception")) {
			if (probe)
				cpu->fpu_exception[0] = 1;
			else
				strcpy(cpu->fpu_exception, end2);
		}
		if (!strcmp(start, "cpuid level")) {
			if (probe)
				cpu->cpu_id_level = 1;
			else
				cpu->cpu_id_level = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "wp")) {
			if (probe)
				cpu->wp[0] = 1;
			else
				strcpy(cpu->wp, end2);
		}
		if (!strcmp(start, "flags")) {
			if (probe)
				cpu->flags[0] = 1;
			else
				strcpy(cpu->flags, end2);
		}
		if (!strcmp(start, "vmx flags")) {
			if (probe)
				cpu->vmx_flags[0] = 1;
			else
				strcpy(cpu->vmx_flags, end2);
		}
		if (!strcmp(start, "bugs")) {
			if (probe)
				cpu->bugs[0] = 1;
			else
				strcpy(cpu->bugs, end2);
		}
		if (!strcmp(start, "bogomips")) {
			if (probe)
				cpu->bogomips = 1;
			else
				cpu->bogomips = atof(end2);
		}
		if (!strcmp(start, "TLB size")) {
			if (probe)
				cpu->tlb_size[0] = 1;
			else
				strcpy(cpu->tlb_size, end2);
		}
		if (!strcmp(start, "clflush size")) {
			if (probe)
				cpu->clflush_size = 1;
			else
				cpu->clflush_size = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "cache_alignment")) {
			if (probe)
				cpu->cache_alignment = 1;
			else
				cpu->cache_alignment = strtoul(end2, NULL, 10);
		}
		if (!strcmp(start, "address sizes")) {
			if (probe)
				cpu->address_sizes[0] = 1;
			else
				strcpy(cpu->address_sizes, end2);
		}
		if (!strcmp(start, "power management")) {
			if (probe)
				cpu->power_mgmt[0] = 1;
			else
				strcpy(cpu->power_mgmt, end2);
		}
nextline:
		start = start2;
		start++;
		bytes_done++;
	}
	return 0;
}

static inline int get_buffer(char *fname, char *buf, unsigned int bufsz)
{
	FILE *fp;
	int count;

	fp = fopen(fname, "r");
	if (fp == NULL) {
		eprintf("Error opening File %s with errno: %d",
			fname, errno);
		return -1;
	}
	count = fread(buf, sizeof(char), bufsz-1, fp);
	if (count < 0) {
		eprintf("in read from File %s with errno: %d", fname, errno);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	buf[count] = '\0';
	return count;
}

int getcpuexist(int res_id, void *exist, size_t sz, void *hint, int flags)
{
	char *buffer = NULL;
	int len, cpu_num;

	memset(exist, 0, sz);
	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	buffer = (char *) malloc(cpu_num * sizeof(struct cpuinfo));
	if (buffer == NULL) {
		errno = ENOMEM;
		return -1;
	}
	len = get_buffer("./cpu_info.orig", buffer, sz);
	if (len == -1)
		return -1;
	len = populate_cpuinfo(exist, buffer, len, 1);
	free(buffer);
	return len;
}

int getcpuinfo(int res_id, void *out, size_t sz, void **hint, int flags)
{
	int len, cpu_num;
	char *buffer = NULL;

	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	memset(out, 0, sz);

	switch (res_id) {
	case RES_CPU_INFO:
		CHECK_SIZE(sz, (cpu_num * sizeof(struct cpuinfo)));
		buffer = (char *) malloc(cpu_num * sizeof(struct cpuinfo));
		if (buffer == NULL) {
			errno = ENOMEM;
			return -1;
		}
#ifdef TESTING
		len = get_buffer("./cpu_info.orig", buffer, sz);
#else
		len = get_buffer(CPUINFO_FILE, buffer, sz);
#endif
		if (len == -1)
			return -1;
		len = populate_cpuinfo(out, buffer, len, 0);
		free(buffer);
		break;
	case RES_CPU_CORECOUNT: 
		*(unsigned int *) out = cpu_num;
		break;
	default:
		eprintf("Resource Id is invalid");
		errno = EINVAL;
		return -1;
	}
	return 0;
}
