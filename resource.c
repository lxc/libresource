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
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "resource_impl.h"
#include "resmem.h"
#include "resnet.h"

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
	res->res_count = res_count;

	/* Allocate and set each resource information properly */
	for (int i = 0; i < res_count; i++) {
		/* Allocate memory for resource info */
		temp = (res_unit_t *)malloc(sizeof(res_unit_t));
		memset(temp, 0, sizeof(res_unit_t));
		temp->status = RES_STATUS_EMPTY;

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
			break;

		case RES_NET_IFSTAT:
			temp->data.ptr = (res_net_ifstat_t *)
				malloc(sizeof(res_net_ifstat_t));
			break;

		case RES_MEM_INFOALL:
			temp->data.ptr = (res_mem_infoall_t *)
				malloc(sizeof(res_mem_infoall_t));
			break;

		default:
			eprintf("Invalid resource ID: %d", res_ids[i]);
			while (--i >= 0)
				free(res->res_unit[i]);
			free(temp);
			free(res);
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
			free((res->res_unit[i])->data.ptr);
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
int res_read(int res_id, void *out, void *hint, int pid, int flags)
{
	struct utsname t;
	int ret;
	int err;

	if (out == NULL) {
		switch (res_id) {
		/* In case of RES_NET_ALLIFSTAT memory is allocated on the
		 * basis of number of interfaces available on system.
		 * So out can be NULL in that case.
		 */
		case RES_NET_ALLIFSTAT:
			break;

		default:
			eprintf("out argument cannot be NULL");
			errno = EINVAL;
			return -1;
		}
	}

	/* Check if memory proc file is needed to open */
	if (res_id >= MEM_MIN && res_id < MEM_MAX)
		return getmeminfo(res_id, out, hint, pid, flags);

	/* Check if net proc file is needed to open */
	if (res_id >= NET_MIN && res_id < NET_MAX)
		return getnetinfo(res_id, out, hint, pid, flags);

	switch (res_id) {
	case RES_KERN_RELEASE:
		ret = uname(&t);
		if (ret == -1) {
			err = errno;
			eprintf("Error in reading kernel release");
			errno = err;
			return -1;
		}
		strncpy(out, t.release, RESOURCE_64);
		break;

	case RES_KERN_COMPILE_TIME:
		ret = uname(&t);
		if (ret == -1) {
			err = errno;
			eprintf("Error in reading version (for compile time)");
			errno = err;
			return -1;
		}
		sscanf(t.version, "%*s%*s%*s%[^\t\n]", (char *) out);
		break;

	default:
		eprintf("Resource Id is invalid");
		errno = EINVAL;
		return -1;
	}
	return 0;
}

/* Read bulk resource information */
int res_read_blk(res_blk_t *res, int pid, int flags)
{
	int ismeminforeq = 0;
	int isnetdevreq = 0;
	struct utsname t;
	int ret;

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
			(res->res_unit[i]->data).sz = sysconf(_SC_PAGESIZE);
			res->res_unit[i]->status = RES_STATUS_FILLED;
			break;

		case RES_KERN_RELEASE:
			ret = uname(&t);
			if (ret == -1) {
				res->res_unit[i]->status = errno;
			} else {
				strncpy((res->res_unit[i]->data).str,
					t.release, RESOURCE_64);
				res->res_unit[i]->status = RES_STATUS_FILLED;
			}
			break;

		case RES_KERN_COMPILE_TIME:
			ret = uname(&t);
			if (ret == -1) {
				res->res_unit[i]->status = errno;
			} else {
				sscanf(t.version, "%*s%*s%*s%[^\t\n]",
					(res->res_unit[i]->data).str);
				res->res_unit[i]->status = RES_STATUS_FILLED;
			}
			break;

		case RES_NET_IFSTAT:
		case RES_NET_ALLIFSTAT:
			isnetdevreq = 1;
			break;

		default:
			res->res_unit[i]->status = RES_STATUS_NOTSUPPORTED;
		}
	}

	if (ismeminforeq)
		populate_meminfo(res, pid, flags);

	if (isnetdevreq)
		populate_netinfo(res, pid, flags);

	return 0;
}
