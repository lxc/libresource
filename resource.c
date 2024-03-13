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
 *
 * This is main file which is starting point to all interfaces provided
 * with libresource.
 */

#include "resource.h"
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "resource_impl.h"
#include "resmem.h"
#include "resnet.h"
#include "resproc.h"
#include "resvm.h"
#include "stat.h"
#include "rescpu.h"
#include "resfs.h"

/* Allocate memory for bulk resource information and initiate it
 * properly.
 */
res_blk_t *res_build_blk(int *res_ids, int res_count)
{
	res_unit_t *temp;
	res_blk_t *res = NULL;

	if (!res_ids || res_count <= 0) {
		errno = EINVAL;
		return NULL;
	}

	/* Allocate memory to hold addresses of individual resource
	 * information.
	 */
	res = (res_blk_t *)
		malloc(sizeof(res_blk_t) + (sizeof(res_unit_t *)) * res_count);
	if (res == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	res->res_count = res_count;

	/* Allocate and set each resource information properly */
	for (int i = 0; i < res_count; i++) {
		/* Allocate memory for resource info */
		temp = (res_unit_t *)malloc(sizeof(res_unit_t));
		if (temp == NULL) {
			res_destroy_blk(res);
			errno = ENOMEM;
			return NULL;
		}
		memset(temp, 0, sizeof(res_unit_t));

		/* Some resource information are big and need extra allocation.
		 * In these cases an address is returned which hold actual
		 * resource information.
		 */
		switch (res_ids[i]) {
		case RES_MEM_ACTIVE:
		case RES_MEM_INACTIVE:
		case RES_MEM_AVAILABLE:
		case RES_MEM_FREE:
		case RES_MEM_TOTAL:
		case RES_MEM_PAGESIZE:
		case RES_MEM_SWAPFREE:
		case RES_MEM_SWAPTOTAL:
		case RES_KERN_COMPILE_TIME:
		case RES_KERN_RELEASE:
		case RES_NET_ALLIFSTAT:
			temp->data_sz = sizeof(union r_data);
			break;

		case RES_NET_IFSTAT:
			temp->data.ptr = (res_net_ifstat_t *)
				malloc(sizeof(res_net_ifstat_t));
			if (temp->data.ptr == NULL) {
				free(temp);
				res_destroy_blk(res);
				errno = ENOMEM;
				return NULL;
			}
			temp->data_sz = sizeof(res_net_ifstat_t);
			break;

		case RES_MEM_INFOALL:
			temp->data.ptr = (res_mem_infoall_t *)
				malloc(sizeof(res_mem_infoall_t));
			if (temp->data.ptr == NULL) {
				free(temp);
				res_destroy_blk(res);
				errno = ENOMEM;
				return NULL;
			}
			temp->data_sz = sizeof(res_mem_infoall_t);
			break;

		case RES_PROC_INFOALL:
			temp->data.ptr = (res_proc_infoall_t *)
				malloc(sizeof(res_proc_infoall_t));
			if (temp->data.ptr == NULL) {
				free(temp);
				res_destroy_blk(res);
				errno = ENOMEM;
				return NULL;
			}
			temp->data_sz = sizeof(res_proc_infoall_t);
			break;

		default:
			eprintf("Invalid resource ID: %d", res_ids[i]);
			free(temp);
			res_destroy_blk(res);
			errno = EINVAL;
			return NULL;
		}

		temp->res_id = res_ids[i];
		res->res_unit[i] = temp;
	}

	return res;
}

/* Free resources allocated in res_build */
void res_destroy_blk(res_blk_t *res)
{
	for (int i = 0; i < res->res_count; i++) {
		/* Some resource information are big and needed extra
		 * Free memory allocated for that.
		 */
		switch (res->res_unit[i]->res_id) {
		case RES_NET_IFSTAT:
		case RES_MEM_INFOALL:
		case RES_PROC_INFOALL:
			free((res->res_unit[i])->data.ptr);
			(res->res_unit[i])->data.ptr = NULL;
			break;
		}

		/* Free memory tho hold this resource info */
		free(res->res_unit[i]);
	}
	free(res);
}

/* read resource information corresponding to res_id, out should have been
 * properly allocated by caller if required.
 */
int res_read(int res_id, void *out, size_t out_sz, void **hint, int pid, int flags)
{
	if (out == NULL) {
		switch (res_id) {
		/* In case of RES_NET_ALLIFSTAT memory is allocated on the
		 * basis of number of interfaces available on system.
		 * So out can be NULL in that case.
		 */
		case RES_NET_ALLIFSTAT:
		case RES_NET_ROUTE_ALL:
		case RES_NET_ARP_ALL:
		case RES_NET_DEV_ALL:
			break;

		default:
			eprintf("out argument cannot be NULL");
			errno = EINVAL;
			return -1;
		}
	}

	if (res_id >= PROC_MIN && res_id < PROC_MAX)
		return getprocinfo(res_id, out, out_sz, hint, pid, flags);

	if (res_id >= MEM_MIN && res_id < MEM_MAX) {
		if (pid > 0) {
			return getmeminfo_cg(res_id, out, out_sz, hint,
					     pid, flags);
		} else {
			return getmeminfo(res_id, out, out_sz, hint,
					  pid, flags);
		}
	}

	if (res_id >= RES_NET_MIN && res_id < RES_NET_MAX)
		return getnetinfo(res_id, out, out_sz, hint, pid, flags);

	if (res_id >= VM_MIN && res_id < VM_MAX)
		return getvmstatinfo(res_id, out, out_sz, hint, flags);

	if (res_id >= CPU_MIN && res_id < CPU_MAX)
		return getcpuinfo(res_id, out, out_sz, hint, flags);

	if (res_id >= ROUTE_MIN && res_id < ROUTE_MAX)
		return getrouteinfo(res_id, out, out_sz, hint, flags);

	if (res_id >= ARP_MIN && res_id < ARP_MAX)
		return getarpinfo(res_id, out, out_sz, hint, flags);

	if (res_id >= DEV_MIN && res_id < DEV_MAX)
		return getdevinfo(res_id, out, out_sz, hint, flags);

	if (res_id >= STAT_MIN && res_id < STAT_MAX)
		return getstatinfo(res_id, out, out_sz, hint, flags);

	if (res_id >= FS_MIN && res_id < FS_MAX)
		return getfsinfo(res_id, out, out_sz, hint, flags);

	return 0;
}

int res_exist(int res_id, void *out, size_t out_sz, void *hint, int pid, int flags)
{
	if (res_id >= VM_MIN && res_id < VM_MAX)
		return getvmexist(res_id, out, out_sz, hint, flags);
	if (res_id >= MEM_MIN && res_id < MEM_MAX)
		return getmemexist(res_id, out, out_sz, hint, flags);
	if (res_id >= CPU_MIN && res_id < CPU_MAX)
		return getcpuexist(res_id, out, out_sz, hint, flags);
	return 0;
}

/* Read bulk resource information */
int res_read_blk(res_blk_t *res, int pid, int flags)
{
	bool ismeminforeq = 0;
	bool isnetdevreq = 0;
	bool isprocreq = 0;
	struct utsname t;
	int ret;
	char *out;
	size_t len;
	char *rawdata;

	/* Loop through all resource information. If it can be filled through
	 * a syscall or such method then fill it. Else set flags which tell
	 * what files might have the information.
	 */
	for (int i = 0; i < res->res_count; i++) {
		switch (res->res_unit[i]->res_id) {
		case RES_MEM_TOTAL:
		case RES_MEM_FREE:
		case RES_MEM_AVAILABLE:
		case RES_MEM_ACTIVE:
		case RES_MEM_INACTIVE:
		case RES_MEM_SWAPFREE:
		case RES_MEM_SWAPTOTAL:
		case RES_MEM_INFOALL:
			ismeminforeq = 1;
			break;

		case RES_MEM_PAGESIZE:
			if (res->res_unit[i]->data_sz < sizeof(long)) {
				res->res_unit[i]->status = ENOMEM;
			} else {
				(res->res_unit[i]->data).sz = sysconf(_SC_PAGESIZE);
				res->res_unit[i]->status = 0;
			}
			break;

		case RES_KERN_RELEASE:
			ret = uname(&t);
			if (ret == -1) {
				res->res_unit[i]->status = errno;
			} else {
				out = (res->res_unit[i]->data).str;
				len = sizeof(union r_data);
				strncpy(out, t.release, len-1);
				out[len-1] = '\0';
				res->res_unit[i]->status = 0;
			}
			break;

		case RES_KERN_COMPILE_TIME:
			ret = uname(&t);
			if (ret == -1) {
				res->res_unit[i]->status = errno;
			} else {
				rawdata = skip_spaces(t.version,
					strlen(t.version), 3);
				out = (res->res_unit[i]->data).str;
				len = sizeof(union r_data);
				strncpy(out, rawdata, len-1);
				out[len-1] = '\0';
				res->res_unit[i]->status = 0;
			}
			break;

		case RES_NET_IFSTAT:
		case RES_NET_ALLIFSTAT:
			isnetdevreq = 1;
			break;
		case RES_PROC_INFOALL:
			isprocreq = 1;

		default:
			res->res_unit[i]->status = -1;
		}
	}

	if (isprocreq)
		populate_procinfo(res, pid, flags);
	if (ismeminforeq) {
		if (pid > 0) {
			return (populate_meminfo_cg(res, pid, flags));
		} else {
			return (populate_meminfo(res, pid, flags));
		}
	}

	if (isnetdevreq)
		populate_netinfo(res, pid, flags);

	return 0;
}
