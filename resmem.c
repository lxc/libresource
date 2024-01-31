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

#define startswith(str, buf) (strncmp(str, buf, sizeof(str) - 1) == 0)

/* read a specific information from file on the basis of a string.
 * String should tell what information is being read.
 */
static inline int get_info_infile(char *fname, char *res, void *out)
{
	const char *loc;
	char buf[MEMBUF_2048];

	if (file_to_buf(fname, buf, MEMBUF_2048) == -1)
		return -1;

	loc = strstr(buf, res);
	if (loc == NULL) {
		eprintf("%s is not found in buffer of file %s", res, fname);
		errno = ENODATA;
		return -1;
	}

	sscanf(loc, "%*s%zu", (size_t *)out);
	return 0;
}

/* read information from a cgroup file.
 */
static inline size_t cgmemread(char *cg, char *file)
{
	char buf[MEMBUF_128];
	char fn[FNAMELEN];
	unsigned long ret;

	snprintf(fn, FNAMELEN, "%s/%s%s/%s", DEFAULTCGFS,
		MEMCGNAME, cg, file);

	if (file_to_buf(fn, buf, MEMBUF_128) == -1)
		return 0;

	if (libres_ulong(buf, &ret) != 0) {
		return 0;
	} else {
		return ret / 1024;
	}
}

/* read memory limit from cgorup file
 */
static size_t cgmemlimit(char *cg, char *f)
{
	char *copy;
	char *dir;
	size_t mem = 0, retmem = 0;

	/* read memory limit for cgroup */
	if ((retmem = cgmemread(cg, f)) == -1) {
		return 0;
	}

	if ((copy = strdup(cg)) == NULL) {
		return 0;
	}

	dir = dirname(copy);

	/*read memory limit for parent cg */
	if (strcmp(dir, "/") != 0) {
		if((mem = cgmemread(dir, f)) == 0) {
			free(copy);
			return 0;
		}
		if (mem < retmem) {
			retmem = mem;
		}
	}

	free(copy);
	return retmem;
}

/* get memory stat from memory.stat file */
static inline size_t cgmemstat(char *cg, char *stat)
{
	char fn[FNAMELEN];
	size_t ret;

	snprintf(fn, FNAMELEN, "%s/%s%s/%s", DEFAULTCGFS,
		MEMCGNAME, cg, "memory.stat");

	if (get_info_infile(fn, stat, &ret) == -1)
		return 0;

	return (ret / 1024);
}

/* Get all memory info for a cgroup */
static int getmeminfoall(char *cg, void *out)
{
	size_t memtotal, mmusage, mmtot, cache, active_anon, inactive_anon,
		active_file, inactive_file, swaptotal, swapfree, swusage,
		swtot;
	char fn[FNAMELEN];
	FILE *fp;
	int err;
	char buf[MEMBUF_128];
	res_mem_infoall_t *memall = out;

	snprintf(fn, FNAMELEN, "%s/%s%s/%s", DEFAULTCGFS,
		MEMCGNAME, cg, "memory.stat");

	fp = fopen(fn, "r");

	if (fp == NULL) {
		err = errno;
		eprintf("while opening File %s with errno: %d", fn, errno);
		errno = err;
		return -1;
	}

	while(fgets(buf, sizeof(buf), fp) != NULL) {
		if (startswith("cache", buf)) {
			sscanf(buf, "%*s%zu", &cache);
			cache /= 1024;
		} else if (startswith("active_anon", buf)) {
			sscanf(buf, "%*s%zu", &active_anon);
			active_anon /= 1024;
		} else if (startswith("inactive_anon", buf)) {
			sscanf(buf, "%*s%zu", &inactive_anon);
			inactive_anon /= 1024;
		} else if (startswith("active_file", buf)) {
			sscanf(buf, "%*s%zu", &active_file);
			active_file /= 1024;
		} else if (startswith("inactive_file", buf)) {
			sscanf(buf, "%*s%zu", &inactive_file);
			inactive_file /= 1024;
		}
	}
	fclose(fp);

	fp = fopen(MEMINFO_FILE, "r");
	if (fp == NULL) {
		err = errno;
		eprintf("while opening File %s with errno: %d",
			MEMINFO_FILE, errno);
		errno = err;
		return -1;
	}

	while(fgets(buf, sizeof(buf), fp) != NULL) {
		if (startswith("MemTotal", buf)) {
			sscanf(buf, "%*s%zu", &memtotal);
		} else if (startswith("SwapTotal", buf)) {
			sscanf(buf, "%*s%zu", &swaptotal);
		} else if (startswith("SwapFree", buf)) {
			sscanf(buf, "%*s%zu", &swapfree);
		}
	}
	fclose(fp);

	mmusage = cgmemread(cg, "memory.usage_in_bytes");
	mmtot = cgmemlimit(cg, "memory.limit_in_bytes");
	if (memtotal < mmtot) {
		mmtot = memtotal;
	}

	memall->memfree = mmtot - mmusage;
	memall->memavailable = mmtot - mmusage + cache;
	memall->memtotal = mmtot;
	memall->active = active_anon + active_file;
	memall->inactive = inactive_anon + inactive_file;

	swusage = cgmemread(cg, "memory.memsw.usage_in_bytes");
	swtot = cgmemlimit(cg, "memory.memsw.limit_in_bytes");

	if (swtot > 0 && swtot < swaptotal)
		swaptotal = swtot;

	if (swtot > 0 && swusage > 0) {
		swusage = swusage - mmusage;
		swapfree = (swusage < swaptotal) ? swaptotal - swusage : 0;
	}

	memall->swaptotal = swaptotal;
	memall->swapfree = swapfree;

	return 0;
}

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
	if (ret == -1)
		return -1;
	ret = populate_minfo(buf, exist, 1);
	return ret;
}

/* Read resource information corresponding to res_id */
int getmeminfo(int res_id, void *out, size_t sz, void **hint, int pid, int flags)
{
	char buf[4096];
	size_t active_anon, active_file, inactive_anon, inactive_file, cache,
		swaptotal, swapfree, swtot, swusage, mmusage, mmtot, memtotal;
	int ret = 0;

	char *cg = get_cgroup(pid, MEMCGNAME);

	if (cg) {
		clean_init(cg);
	}

	switch (res_id) {
		/* if process is part of a cgroup then return memory info
		 * for that cgroup only.
		 */
	case RES_MEM_FREE:
		CHECK_SIZE(sz, sizeof(size_t));

		if (cg) {
			mmusage = cgmemread(cg, "memory.usage_in_bytes");
			if (get_info_infile(MEMINFO_FILE, "MemTotal:",
				&memtotal) == -1)
				return -1;
			mmtot = cgmemlimit(cg, "memory.limit_in_bytes");
			if (memtotal < mmtot) {
				mmtot = memtotal;
			}

			*(size_t *)out = mmtot - mmusage;
			return 0;
		} else {
			return get_info_infile(MEMINFO_FILE, "MemFree:", out);
		}

	case RES_MEM_AVAILABLE:
		CHECK_SIZE(sz, sizeof(size_t));

		if (cg) {
			mmusage = cgmemread(cg, "memory.usage_in_bytes");
			if (get_info_infile(MEMINFO_FILE, "MemTotal:",
				&memtotal) == -1)
				return -1;

			mmtot = cgmemlimit(cg, "memory.limit_in_bytes");

			if (memtotal < mmtot) {
				mmtot = memtotal;
			}
			cache = cgmemstat(cg, "cache");

			*(size_t *)out = mmtot - mmusage + cache;

			return 0;
		} else {
			return get_info_infile(MEMINFO_FILE, "MemAvailable:",
				out);
		}

	case RES_MEM_TOTAL:
		CHECK_SIZE(sz, sizeof(size_t));

		ret = get_info_infile(MEMINFO_FILE, "MemTotal:", out);
		if (cg) {
			mmtot = cgmemlimit(cg, "memory.limit_in_bytes");
			if (ret != -1 && *(size_t *)out > mmtot) {
				*(size_t *)out = mmtot;
			}
		}
		return ret;

	case RES_MEM_ACTIVE:
		CHECK_SIZE(sz, sizeof(size_t));

		if (cg) {
			active_anon = cgmemstat(cg, "\nactive_anon");
			active_file = cgmemstat(cg, "\nactive_file");
			*(size_t *)out = active_anon + active_file;
			return 0;
		} else {
			return get_info_infile(MEMINFO_FILE, "Active:", out);
		}

	case RES_MEM_INACTIVE:
		CHECK_SIZE(sz, sizeof(size_t));

		if (cg) {
			inactive_anon = cgmemstat(cg, "\ninactive_anon");
			inactive_file = cgmemstat(cg, "\ninactive_file");
			*(size_t *)out = inactive_anon + inactive_file;
			return 0;
		} else {
			return get_info_infile(MEMINFO_FILE, "Inactive:", out);
		}

	case RES_MEM_SWAPTOTAL:
		CHECK_SIZE(sz, sizeof(size_t));

		ret = get_info_infile(MEMINFO_FILE, "SwapTotal:", out);
		if (cg) {
			swtot = cgmemlimit(cg,
				"memory.memsw.limit_in_bytes");
			if (ret != -1 && swtot > 0 &&
				*(size_t *)out > swtot) {
				*(size_t *)out = swtot;
			}
		}
		return ret;

	case RES_MEM_SWAPFREE:
		CHECK_SIZE(sz, sizeof(size_t));

		if (cg) {
			swusage = cgmemread(cg, "memory.memsw.usage_in_bytes");
			swtot = cgmemlimit(cg,
				"memory.memsw.limit_in_bytes");

			if (swusage > 0 &&  swtot > 0) {
				mmusage = cgmemread(cg,
					"memory.usage_in_bytes");
				if (get_info_infile(MEMINFO_FILE,
					"SwapTotal:", &swaptotal) == -1)
					return -1;

				if (swtot > swaptotal) {
					swtot = swaptotal;
				}

				swusage = swusage - mmusage;
				if (swusage < swtot)
					swapfree = swtot - swusage;
				else
					swapfree = 0;
				*(size_t *)out = swapfree;
				return 0;
			}
		}
		return get_info_infile(MEMINFO_FILE, "SwapFree:", out);

	case RES_MEM_INFOALL:
		CHECK_SIZE(sz, sizeof(res_mem_infoall_t));

		if (cg) {
			if (getmeminfoall(cg, out) == -1) {
				return -1;
			}
			break;
		}

#ifdef TESTING
		ret = file_to_buf("./mem_info.orig", buf, sizeof(buf));
#else
		ret = file_to_buf(MEMINFO_FILE, buf, sizeof(buf));
#endif
		if (ret == -1)
			return -1;
		ret = populate_minfo(buf, out, 0);
		break;

	case RES_MEM_PAGESIZE:
		CHECK_SIZE(sz, sizeof(long));

		*(size_t *)out = sysconf(_SC_PAGESIZE);
		break;

	default:
		eprintf("Resource Id is invalid");
		errno = EINVAL;
		return -1;
	}

#undef CHECK_SIZE

	return ret;
}

int populate_meminfo(res_blk_t *res, int pid, int flags)
{
	const char *loc;
	char buf[MEMBUF_2048];
	size_t active_anon = 0, active_file = 0, inactive_anon = 0,
		inactive_file = 0, cache = 0, swaptotal = 0, swapfree = 0,
		swtot = 0, swusage = 0, mmusage = 0, mmtot = 0, memtotal = 0,
		memavailable = 0, memfree = 0, memactive = 0, meminactive = 0;

	char *cg = get_cgroup(pid, MEMCGNAME);
	res_mem_infoall_t *mminfo;

	if (cg) {
		clean_init(cg);
	}

	if (file_to_buf(MEMINFO_FILE, buf, MEMBUF_2048) == -1) {
		for (int i = 0 ; i < res->res_count; i++) {
			switch (res->res_unit[i]->res_id) {
			case RES_MEM_FREE:
			case RES_MEM_AVAILABLE:
			case RES_MEM_TOTAL:
			case RES_MEM_ACTIVE:
			case RES_MEM_INACTIVE:
			case RES_MEM_SWAPTOTAL:
			case RES_MEM_SWAPFREE:
			case RES_MEM_INFOALL:
				res->res_unit[i]->status = errno;
			}
		}
		return -1;
	}

/* Macro to read memory related information corresponding to a string
 * from buffer.
 */
#define SCANMEMSTR(str, info) do {\
	loc = strstr(buf, str);\
	if (loc == NULL) {\
		eprintf("%s is not found in file %s", str, MEMINFO_FILE);\
			res->res_unit[i]->status = ENODATA;\
	} else {\
		sscanf(loc, "%*s%zu", &info);\
		(res->res_unit[i]->data).sz = info;\
		res->res_unit[i]->status = RES_STATUS_FILLED;\
	} \
} while (0)\

/* Macro to check if enough memory is allocated to hold the data.
 */
#define CHECK_SIZE(sz, data_sz)						\
	if (sz < data_sz) {						\
		eprintf("memory (%ld) is not enough to hold data (%ld)",\
                        sz, data_sz);					\
		res->res_unit[i]->status = ENOMEM;			\
		break;							\
	}

	for (int i = 0; i < res->res_count; i++) {
		loc = NULL;
		switch (res->res_unit[i]->res_id) {
		case RES_MEM_FREE:
			CHECK_SIZE(res->res_unit[i]->data_sz, sizeof(size_t));

			if (cg) {
				if (!mmusage)
					mmusage = cgmemread(cg,
						"memory.usage_in_bytes");
				if (!memtotal)
					SCANMEMSTR("MemTotal:", memtotal);
				if (!mmtot)
					mmtot = cgmemlimit(cg,
						"memory.limit_in_bytes");

				if (memtotal < mmtot) {
					mmtot = memtotal;
				}

				(res->res_unit[i]->data).sz = mmtot - mmusage;
				res->res_unit[i]->status = RES_STATUS_FILLED;
			} else {
				SCANMEMSTR("MemFree:", memfree);
			}
			break;

		case RES_MEM_AVAILABLE:
			CHECK_SIZE(res->res_unit[i]->data_sz, sizeof(size_t));

			if (cg) {
				if (!mmusage)
					mmusage = cgmemread(cg,
						"memory.usage_in_bytes");
				if (!memtotal)
					SCANMEMSTR("MemTotal:", memtotal);
				if (!mmtot)
					mmtot = cgmemlimit(cg,
						"memory.limit_in_bytes");

				if (memtotal < mmtot) {
					mmtot = memtotal;
				}

				cache = cgmemstat(cg, "cache");
				(res->res_unit[i]->data).sz = 
					mmtot - mmusage + cache;
				res->res_unit[i]->status = RES_STATUS_FILLED;
			} else {
				SCANMEMSTR("MemAvailable:", memavailable);
			}
			break;

		case RES_MEM_TOTAL:
			CHECK_SIZE(res->res_unit[i]->data_sz, sizeof(size_t));

			if (!memtotal)
				SCANMEMSTR("MemTotal:", memtotal);
			else
				(res->res_unit[i]->data).sz = memtotal;

			if (cg) {
				if (!mmtot)
					mmtot = cgmemlimit(cg,
						"memory.limit_in_bytes");
				if (memtotal > mmtot)
					(res->res_unit[i]->data).sz = mmtot;
			}
			res->res_unit[i]->status = RES_STATUS_FILLED;
			break;

		case RES_MEM_ACTIVE:
			CHECK_SIZE(res->res_unit[i]->data_sz, sizeof(size_t));

			if (cg) {
				active_anon = cgmemstat(cg, "\nactive_anon");
				active_file = cgmemstat(cg, "\nactive_file");
				(res->res_unit[i]->data).sz =
					active_anon + active_file;
				res->res_unit[i]->status = RES_STATUS_FILLED;
			} else {
				SCANMEMSTR("Active:", memactive);
			}
			break;

		case RES_MEM_INACTIVE:
			CHECK_SIZE(res->res_unit[i]->data_sz, sizeof(size_t));

			if (cg) {
				inactive_anon = cgmemstat(cg,
					"\ninactive_anon");
				inactive_file = cgmemstat(cg,
					"\ninactive_file");
				(res->res_unit[i]->data).sz =
					inactive_anon + inactive_file;
				res->res_unit[i]->status = RES_STATUS_FILLED;
			} else {
				SCANMEMSTR("Inactive:", meminactive);
			}
			break;

		case RES_MEM_SWAPTOTAL:
			CHECK_SIZE(res->res_unit[i]->data_sz, sizeof(size_t));

			if (!swaptotal)
				SCANMEMSTR("SwapTotal:", swaptotal);
			else
				(res->res_unit[i]->data).sz = swaptotal;

			if (cg) {
				swtot = cgmemlimit(cg,
					"memory.memsw.limit_in_bytes");
				if (swtot > 0 && swaptotal > swtot) {
					(res->res_unit[i]->data).sz = swtot;
				}
			}
			res->res_unit[i]->status = RES_STATUS_FILLED;
			break;

		case RES_MEM_SWAPFREE:
			CHECK_SIZE(res->res_unit[i]->data_sz, sizeof(size_t));

			SCANMEMSTR("SwapFree:", swapfree);

			if (cg) {
				swusage = cgmemread(cg,
					"memory.memsw.usage_in_bytes");
				swtot = cgmemlimit(cg,
					"memory.memsw.limit_in_bytes");

				if (!swusage || !swtot) {
					break;
				}

				if (!mmusage)
					mmusage = cgmemread(cg,
						"memory.usage_in_bytes");
				if (!swaptotal)
					SCANMEMSTR("SwapTotal:", swaptotal);

				if (swtot > swaptotal) {
					swtot = swaptotal;
				}

				swusage = swusage - mmusage;
				if (swusage < swtot)
					swapfree = swtot - swusage;
				else
					swapfree = 0;
				(res->res_unit[i]->data).sz = swapfree;
				res->res_unit[i]->status = RES_STATUS_FILLED;
			}
			break;

		case RES_MEM_INFOALL:
			CHECK_SIZE(res->res_unit[i]->data_sz,
				sizeof(res_mem_infoall_t));

			if (cg) {
				if (getmeminfoall(cg,
					(res->res_unit[i]->data).ptr) == -1) {
					res->res_unit[i]->status = ENODATA;
					return -1;
				}
				res->res_unit[i]->status = RES_STATUS_FILLED;
				break;
			}
			mminfo = (res_mem_infoall_t *)
				(res->res_unit[i]->data).ptr;

			loc = strstr(buf, "MemTotal:");
			sscanf(loc, "%*s%zu", &mminfo->memtotal);
			loc = strstr(buf, "MemFree:");
			sscanf(loc, "%*s%zu", &mminfo->memfree);
			loc = strstr(buf, "MemAvailable:");
			sscanf(loc, "%*s%zu", &mminfo->memavailable);
			loc = strstr(buf, "Active:");
			sscanf(loc, "%*s%zu", &mminfo->active);
			loc = strstr(buf, "Inactive:");
			sscanf(loc, "%*s%zu", &mminfo->inactive);
			loc = strstr(buf, "SwapTotal:");
			sscanf(loc, "%*s%zu", &mminfo->swaptotal);
			loc = strstr(buf, "SwapFree:");
			sscanf(loc, "%*s%zu", &mminfo->swapfree);

			res->res_unit[i]->status = RES_STATUS_FILLED;
			break;
		}
	}

#undef CHECK_SIZE

	return 0;
}
