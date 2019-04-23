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

#define startswith(str, buf) strncmp(str, buf, sizeof(str) - 1)

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

	snprintf(fn, FNAMELEN, "%s/%s%s/%s", DEFAULTCGFS,
		MEMCGNAME, cg, file);

	if (file_to_buf(fn, buf, MEMBUF_128) == -1)
		return 0;

	return (strtoul(buf, NULL, 10) / 1024);
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

	copy = strdup(cg);
	dir = dirname(copy);

	/*read memory limit for parant cg */
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
		if (startswith("cache", buf) == 0) {
			sscanf(buf, "%*s%zu", &cache);
			cache /= 1024;
		} else if (startswith("active_anon", buf) == 0) {
			sscanf(buf, "%*s%zu", &active_anon);
			active_anon /= 1024;
		} else if (startswith("inactive_anon", buf) == 0) {
			sscanf(buf, "%*s%zu", &inactive_anon);
			inactive_anon /= 1024;
		} else if (startswith("active_file", buf) == 0) {
			sscanf(buf, "%*s%zu", &active_file);
			active_file /= 1024;
		} else if (startswith("inactive_file", buf) == 0) {
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
		if (startswith("MemTotal", buf) == 0) {
			sscanf(buf, "%*s%zu", &memtotal);
		} else if (startswith("SwapTotal", buf) == 0) {
			sscanf(buf, "%*s%zu", &swaptotal);
		} else if (startswith("SwapFree", buf) == 0) {
			sscanf(buf, "%*s%zu", &swapfree);
		}
	}
	fclose(fp);

	mmusage = cgmemread(cg, "memory.usage_in_bytes");
	mmtot = cgmemlimit(cg, "memory.limit_in_bytes");
	if (memtotal < mmtot) {
		mmtot = memtotal;
	}

	((res_mem_infoall_t *)out)->memfree = mmtot - mmusage;
	((res_mem_infoall_t *)out)->memavailable = mmtot - mmusage + cache;
	((res_mem_infoall_t *)out)->memtotal = mmtot;
	((res_mem_infoall_t *)out)->active = active_anon + active_file;
	((res_mem_infoall_t *)out)->inactive = inactive_anon + inactive_file;

	swusage = cgmemread(cg, "memory.memsw.usage_in_bytes");
	swtot = cgmemlimit(cg, "memory.memsw.limit_in_bytes");

	if (swtot > 0 && swtot < swaptotal)
		swaptotal = swtot;

	if (swtot > 0 && swusage > 0) {
		swusage = swusage - mmusage;
		swapfree = (swusage < swaptotal) ? swaptotal - swusage : 0;
	}

	((res_mem_infoall_t *)out)->swaptotal = swaptotal;
	((res_mem_infoall_t *)out)->swapfree = swapfree;

	return 0;
}

/* Read resource information corresponding to res_id */
int getmeminfo(int res_id, void *out, void *hint, int pid, int flags)
{
	char buf[MEMBUF_128];
	FILE *fp;
	int err = 0;
	size_t active_anon, active_file, inactive_anon, inactive_file, cache,
		swaptotal, swapfree, swtot, swusage, mmusage, mmtot, memtotal;
	int ret = 0;

	char *cg = get_cgroup(pid, MEMCGNAME);

	if (cg) {
		clean_init(cg);
	}

	switch (res_id) {
		/* if process is part of a cgroup then return memory info
		 *for that cgroup only.
		 */
	case RES_MEM_FREE:
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
		ret = get_info_infile(MEMINFO_FILE, "MemTotal:", out);
		if (cg) {
			mmtot = cgmemlimit(cg, "memory.limit_in_bytes");
			if (ret != -1 && *(size_t *)out > mmtot) {
				*(size_t *)out = mmtot;
			}
		}
		return ret;

	case RES_MEM_ACTIVE:
		if (cg) {
			active_anon = cgmemstat(cg, "\nactive_anon");
			active_file = cgmemstat(cg, "\nactive_file");
			*(size_t *)out = active_anon + active_file;
			return 0;
		} else {
			return get_info_infile(MEMINFO_FILE, "Active:", out);
		}

	case RES_MEM_INACTIVE:
		if (cg) {
			inactive_anon = cgmemstat(cg, "\ninactive_anon");
			inactive_file = cgmemstat(cg, "\ninactive_file");
			*(size_t *)out = inactive_anon + inactive_file;
			return 0;
		} else {
			return get_info_infile(MEMINFO_FILE, "Inactive:", out);
		}

	case RES_MEM_SWAPTOTAL:
		ret = get_info_infile(MEMINFO_FILE, "SwapTotal:", out);
		if (cg) {
			swtot = cgmemlimit(cg,
				"memory.memsw.limit_in_bytes");
			if (ret != -1 && swtot > 0 && *(size_t *)out > swtot) {
				*(size_t *)out = swtot;
			}
		}
		return ret;

	case RES_MEM_SWAPFREE:
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
		if (cg) {
			if (getmeminfoall(cg, out) == -1) {
				return -1;
			}
			break;
		}

		fp = fopen(MEMINFO_FILE, "r");
		if (fp == NULL) {
			err = errno;
			eprintf("while opening File %s with errno: %d",
				MEMINFO_FILE, errno);
			errno = err;
			return -1;
		}
		/* Read through file and populate all information which
		 * is required.
		 */
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			if (startswith("MemTotal:", buf) == 0) {
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->memtotal);
			} else if (startswith("MemFree:", buf) == 0) {
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->memfree);
			} else if (startswith("MemAvailable:", buf) == 0) {
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->memavailable);
			} else if (startswith("Active", buf) == 0) {
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->active);
			} else if (startswith("Inactive", buf) == 0) {
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->inactive);
			} else if (startswith("SwapTotal", buf) == 0) {
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->swaptotal);
			} else if (startswith("SwapFree", buf) == 0) {
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->swapfree);
			}
		}
		fclose(fp);
		break;

	case RES_MEM_PAGESIZE:
		*(size_t *)out = sysconf(_SC_PAGESIZE);
		break;

	default:
		eprintf("Resource Id is invalid");
		errno = EINVAL;
		return -1;
	}

	return 0;
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

	if (cg) {
		clean_init(cg);
	}

	if (file_to_buf(MEMINFO_FILE, buf, MEMBUF_2048) == -1) {
		for (int i = 0 ; i < res->res_count; i++)
			res->res_unit[i]->status = errno;
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


	for (int i = 0; i < res->res_count; i++) {
		loc = NULL;
		switch (res->res_unit[i]->res_id) {
		case RES_MEM_FREE:
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
			if (!memtotal)
				SCANMEMSTR("MemTotal:", memtotal);
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
			if (cg) {
				active_anon = cgmemstat(cg, "active_anon");
				active_file = cgmemstat(cg, "active_file");
				(res->res_unit[i]->data).sz =
					active_anon + active_file;
				res->res_unit[i]->status = RES_STATUS_FILLED;
			} else {
				SCANMEMSTR("Active:", memactive);
			}
			break;

		case RES_MEM_INACTIVE:
			if (cg) {
				inactive_anon = cgmemstat(cg, "inactive_anon");
				inactive_file = cgmemstat(cg, "inactive_file");
				(res->res_unit[i]->data).sz =
					inactive_anon + inactive_file;
				res->res_unit[i]->status = RES_STATUS_FILLED;
			} else {
				SCANMEMSTR("Inactive:", meminactive);
			}
			break;

		case RES_MEM_SWAPTOTAL:
			if (!swaptotal)
				SCANMEMSTR("SwapTotal:", swaptotal);
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

		case RES_MEM_PAGESIZE:
			(res->res_unit[i]->data).sz = sysconf(_SC_PAGESIZE);
			break;

		case RES_MEM_INFOALL:
			if (cg) {
				if (getmeminfoall(cg,
					(res->res_unit[i]->data).ptr) == -1) {
					res->res_unit[i]->status = ENODATA;
					return -1;
				}
				res->res_unit[i]->status = RES_STATUS_FILLED;
				break;
			}

			loc = strstr(buf, "MemTotal:");
			sscanf(loc, "%*s%zu", &((res_mem_infoall_t *)
				(res->res_unit[i]->data).ptr)->memtotal);
			loc = strstr(buf, "MemFree:");
			sscanf(loc, "%*s%zu", &((res_mem_infoall_t *)
				(res->res_unit[i]->data).ptr)->memfree);
			loc = strstr(buf, "MemAvailable:");
			sscanf(loc, "%*s%zu", &((res_mem_infoall_t *)
				(res->res_unit[i]->data).ptr)->memavailable);
			loc = strstr(buf, "Active:");
			sscanf(loc, "%*s%zu", &((res_mem_infoall_t *)
				(res->res_unit[i]->data).ptr)->active);
			loc = strstr(buf, "Inactive:");
			sscanf(loc, "%*s%zu", &((res_mem_infoall_t *)
				(res->res_unit[i]->data).ptr)->inactive);
			loc = strstr(buf, "SwapTotal:");
			sscanf(loc, "%*s%zu", &((res_mem_infoall_t *)
				(res->res_unit[i]->data).ptr)->swaptotal);
			loc = strstr(buf, "SwapFree:");
			sscanf(loc, "%*s%zu", &((res_mem_infoall_t *)
				(res->res_unit[i]->data).ptr)->swapfree);

			res->res_unit[i]->status = RES_STATUS_FILLED;
			break;
		default:
			res->res_unit[i]->status = RES_STATUS_NOTSUPPORTED;
		}
	}
	return 0;
}
