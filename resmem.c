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
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "resmem.h"
#include "resource_impl.h"
#include <errno.h>

/* read a specific information from file on the basis of a string.
 * String should tell what information is being read.
 */
static inline int get_info_infile(char *fname, char *res, void *out)
{
	const char *loc;
	char buf[MEMBUF_2048];

	if (file_to_buf(fname, buf) == -1)
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

/* Read resource information corresponding to res_id */
int getmeminfo(int res_id, void *out, void *hint, int pid, int flags)
{
	char buf[MEMBUF_128];
	FILE *fp;
	int err = 0;

	switch (res_id) {

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
			if (strncmp("MemTotal:", buf, MEMBUF_8) == 0)
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->memtotal);

			else if (strncmp("MemFree:", buf, MEMBUF_8) == 0)
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->memfree);

			else if (strncmp("MemAvailable:", buf, MEMBUF_8) == 0)
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->memavailable);

			else if (strncmp("Active:", buf, MEMBUF_8) == 0)
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->active);

			else if (strncmp("Inactive:", buf, MEMBUF_8) == 0)
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->inactive);

			else if (strncmp("SwapTotal:", buf, MEMBUF_8) == 0)
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->swaptotal);

			else if (strncmp("SwapFree:", buf, MEMBUF_8) == 0)
				sscanf(buf, "%*s%zu",
				&((res_mem_infoall_t *)out)->swapfree);
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
	size_t temp;
	const char *loc;
	char buf[MEMBUF_2048];

	if (file_to_buf(MEMINFO_FILE, buf) == -1) {
		for (int i = 0 ; i < res->res_count; i++)
			res->res_unit[i]->status = errno;
		return -1;
	}

/* Macro to read memory related information corresponding to a string
 * from buffer.
 */
#define SCANMEMSTR(str) do {\
	loc = strstr(buf, str);\
	if (loc == NULL) {\
		eprintf("%s is not found in file %s", str, MEMINFO_FILE);\
			res->res_unit[i]->status = ENODATA;\
	} else {\
		sscanf(loc, "%*s%zu", &temp);\
		(res->res_unit[i]->data).sz = temp;\
		res->res_unit[i]->status = RES_STATUS_FILLED;\
	} \
} while (0)\


	for (int i = 0; i < res->res_count; i++) {
		loc = NULL;
		switch (res->res_unit[i]->res_id) {
		case RES_MEM_FREE:
			SCANMEMSTR("MemFree:");
			break;

		case RES_MEM_AVAILABLE:
			SCANMEMSTR("MemAvailable:");
			break;

		case RES_MEM_TOTAL:
			SCANMEMSTR("MemTotal:");
			break;

		case RES_MEM_ACTIVE:
			SCANMEMSTR("Active:");
			break;

		case RES_MEM_INACTIVE:
			SCANMEMSTR("Inactive:");
			break;

		case RES_MEM_SWAPTOTAL:
			SCANMEMSTR("SwapTotal:");
			break;

		case RES_MEM_SWAPFREE:
			SCANMEMSTR("SwapFree:");
			break;

		case RES_MEM_PAGESIZE:
			(res->res_unit[i]->data).sz = sysconf(_SC_PAGESIZE);
			break;

		case RES_MEM_INFOALL:
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

