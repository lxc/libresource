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

struct mem_table {
        char *mem_name;
        unsigned long *mem_value;
};

static int compare_mem_table_entry(const void *entry1, const void *entry2)
{
	return strcmp(((struct mem_table*)entry1)->mem_name,((struct mem_table*)entry2)->mem_name);
}

int populate_minfo(char *buffer, void *out, int ex)
{
	char *end, *start;
        char search_name[32];
        struct mem_table search_entry = {search_name, NULL};
        struct mem_table *result;
        int mem_table_count;
        struct memstat mem;

	struct mem_table mem_table_array[] = {
		{"MemTotal:", &mem.memtotal},
		{"MemFree:", &mem.memfree},
		{"MemAvailable:", &mem.memavailable},
		{"Buffers:", &mem.buffers},
		{"Cached:", &mem.cached},
		{"SwapCached:", &mem.swapcached},
		{"Active:", &mem.active},
		{"Inactive:", &mem.inactive},
		{"Active(anon):", &mem.active_anon},
		{"Inactive(anon):", &mem.inactive_anon},
		{"Active(file):", &mem.active_file},
		{"Inactive(file):", &mem.inactive_file},
		{"Unevictable:", &mem.unevictable},
		{"Mlocked:", &mem.mlocked},
		{"SwapTotal:", &mem.swaptotal},
		{"SwapFree:", &mem.swapfree},
		{"Zswap:", &mem.zswap},
		{"Zswapped:", &mem.zswapped},
		{"Dirty:", &mem.dirty},
		{"Writeback:", &mem.writeback},
		{"AnonPages:", &mem.anonpages},
		{"Mapped:", &mem.mapped},
		{"Shmem:", &mem.shmem},
		{"KReclaimable:", &mem.kreclaimable},
		{"Slab:", &mem.slab},
		{"SReclaimable:", &mem.sreclaimable},
		{"SUnreclaim:", &mem.sunreclaim},
		{"KernelStack:", &mem.kernelstack},
		{"PageTables:", &mem.pagetables},
		{"SecPageTables:", &mem.secpagetables},
		{"NFS_Unstable:", &mem.nfs_unstable},
		{"Bounce:", &mem.bounce},
		{"WritebackTmp:", &mem.writebacktmp},
		{"CommitLimit:", &mem.commitlimit},
		{"Committed_AS:", &mem.committed_as},
		{"VmallocTotal:", &mem.vmalloc_total},
		{"VmallocUsed:", &mem.vmalloc_used},
		{"VmallocChunk:", &mem.vmalloc_chunk},
		{"Percpu:", &mem.percpu},
		{"HardwareCorrupted:", &mem.hardware_corrupted},
		{"AnonHugePages:", &mem.anon_hugepages},
		{"ShmemHugePages:", &mem.shmem_hugepages},
		{"ShmemPmdMapped:", &mem.shmem_pmd_mapped},
		{"FileHugePages:", &mem.file_hugepages},
		{"FilePmdMapped:", &mem.file_pmd_mapped},
		{"CmaTotal:", &mem.cma_total},
		{"CmaFree:", &mem.cma_free},
		{"HugePages_Total:", &mem.hugepages_total},
		{"HugePages_Free:", &mem.hugepages_free},
		{"HugePages_Rsvd:", &mem.hugepages_rsvd},
		{"HugePages_Surp:", &mem.hugepages_surp},
		{"Hugepagesize:", &mem.hugepages_size},
		{"Hugetlb:", &mem.hugetlb},
		{"DirectMap4k:", &mem.directmap_4k},
		{"DirectMap2M:", &mem.directmap_2M},
		{"DirectMap1G:", &mem.directmap_1G},
	};

	memset(&mem, 0, sizeof(mem));

	mem_table_count = sizeof(mem_table_array)/sizeof(struct mem_table);
	qsort(mem_table_array, mem_table_count, sizeof(struct mem_table),
	      compare_mem_table_entry);
        //for (int i = 0; i < mem_table_count; i++)
        //      printf("%d: %s\n", i, mem_table_array[i]);
	start = buffer;
	while(1) {
		end = strchr(start, ' ');
		if (!end)
			break;
		end[0] = '\0';
		strcpy(search_entry.mem_name, start);
		result = bsearch(&search_entry, mem_table_array,
				 mem_table_count,
				 sizeof(struct mem_table),
				 compare_mem_table_entry);
		if (result) {
			end++;
			while (end && *end == ' ')
				end++;
			if (!end)
				break;
			if (*end == '\n')
				goto nextline;
			if (ex)
				*(result->mem_value) = 1;
			else
				*(result->mem_value) = strtoul(end, NULL, 10);
			//printf("Found %s value %lu\n",search_entry.mem_name, *(result->mem_value));
		} else {
			printf("Entry %s not found\n",search_entry.mem_name);
		}
nextline:
		start = strchr(end, '\n');
		if (!start)
			break;
		start++;
	}
	memcpy(out, &mem, sizeof(mem));
	return 0;
}

int getmemexist(int res_id, void *exist, size_t sz, void *hint, int flags)
{
	int ret;
	char buf[4096];

	ret = file_to_buf("./mem_info.orig", buf, sizeof(buf));
	if (ret < 0)
		return ret;
	ret = populate_minfo(buf, exist, 1);
	return ret;
}

/* Read resource information corresponding to res_id */
int getmeminfo(int res_id, void *out, size_t sz, void **hint, int pid,
		int flags)
{
	char buf[4096];
	int ret;

	switch (res_id) {
		/* if process is part of a cgroup then return memory info
		 * for that cgroup only.
		 */
	case RES_MEM_FREE:
		return get_info_infile(MEMINFO_FILE, "MemFree:", out);

	case RES_MEM_AVAILABLE:
		return get_info_infile(MEMINFO_FILE, "MemAvailable:", out);

	case RES_MEM_TOTAL:
		return get_info_infile(MEMINFO_FILE, "MemTotal:", out);

	case RES_MEM_ACTIVE:
		return get_info_infile(MEMINFO_FILE, "Active:", out);

	case RES_MEM_INACTIVE:
		return get_info_infile(MEMINFO_FILE, "Inactive:", out);

	case RES_MEM_SWAPTOTAL:
		return get_info_infile(MEMINFO_FILE, "SwapTotal:", out);

	case RES_MEM_SWAPFREE:
		return get_info_infile(MEMINFO_FILE, "SwapFree:", out);

	case RES_MEM_INFOALL:

#ifdef TESTING
		ret = file_to_buf("./mem_info.orig", buf, sizeof(buf));
#else
		ret = file_to_buf(MEMINFO_FILE, buf, sizeof(buf));
#endif
		if (ret < 0)
			return ret;
		ret = populate_minfo(buf, out, 0);
		return ret;

	case RES_MEM_PAGESIZE:
		*(size_t *)out = sysconf(_SC_PAGESIZE);
		return 0;

	default:
		eprintf("Resource Id is invalid");
		return -EINVAL;
	}
	return 0;
}

int populate_meminfo(res_blk_t *res, int pid, int flags)
{
	for (int i = 0; i < res->res_count; i++) {
		switch (res->res_unit[i]->res_id) {
		case RES_MEM_FREE:
			return get_info_infile(MEMINFO_FILE, "MemFree:",
					&((res->res_unit[i]->data).sz));

		case RES_MEM_AVAILABLE:
			return get_info_infile(MEMINFO_FILE, "MemAvailable:",
				       &((res->res_unit[i]->data).sz));

		case RES_MEM_TOTAL:
			return get_info_infile(MEMINFO_FILE, "MemTotal:",
				       &((res->res_unit[i]->data).sz));

		case RES_MEM_ACTIVE:
			return get_info_infile(MEMINFO_FILE, "Active:",
					&((res->res_unit[i]->data).sz));

		case RES_MEM_INACTIVE:
			return get_info_infile(MEMINFO_FILE, "Inactive:",
					&((res->res_unit[i]->data).sz));

		case RES_MEM_SWAPTOTAL:
			return get_info_infile(MEMINFO_FILE, "SwapTotal:",
					&((res->res_unit[i]->data).sz));

		case RES_MEM_SWAPFREE:
			return get_info_infile(MEMINFO_FILE, "SwapFree:",
					&((res->res_unit[i]->data).sz));

		}
	}
	return 0;
}
