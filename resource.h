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
 * This is main header file which declares all resource ids and interfaces.
 *
 */

#ifndef	_RESOURCE_H
#define	_RESOURCE_H

#include <net/if.h>

/* libresource version */
#define LIBRESOURCE_API_VERSION (1.0.0)

/* Possible status for libresource information returned */
/* libresource information was fetched correctly */
#define RES_STATUS_FILLED 0
/* There was some error in fetching libresource information. In most cases
 * errno will be set.
 */
#define RES_STATUS_EMPTY -1
/* Resource information is not supported yet, or Invalid resource
 * information.
 */
#define RES_STATUS_NOTSUPPORTED	-2
/* If partial information was read for a libresource information. For example
 * a string was read partially.
 */
#define RES_STATUS_TRUNCATED -3


/* Maximum size of a resource information data type which can be returned
 * without explicitly allocating memory for it. If resource information
 * size is larger than this this a pointer to allocated memory will be
 * returned.
 */
#define RES_UNIT_OUT_SIZE	256

/* This union is used to return resource information of various types */
union r_data {
	int i;
	size_t sz;
	long l;
	char str[RES_UNIT_OUT_SIZE];
	void *ptr;
};

/* In case of res_read_blk, each resource information will be represented by
 * following structure.
 */
typedef struct res_unit {
	int status;
	unsigned int res_id;
	void *hint;
	union r_data data;
} res_unit_t;

/* In case of bulk read (res_read_blk), this structure will hold all required
 * information needed to do so.
 */
typedef struct res_blk {
	int res_count;
	res_unit_t *res_unit[0];
} res_blk_t;

/* Resource information is divided in broad categories and each
 * category is assigned a number range for its resource information
 * Memory related			(RES_MEM_*)		1024-
 * Network related			(RES_NET_*)		2048-
 * General kernel related		(RES_KERN_*)		3072-
 * This is done to facilitate any future optimization which can be made
 * on the basis of resource information (hashing etc ?)
 */
#define MEM_MIN				1024
#define RES_MEM_HUGEPAGEALL		1025
#define RES_MEM_HUGEPAGESIZE		1026
#define RES_MEM_INACTIVE		1027
#define RES_MEM_INFOALL			1028
#define RES_MEM_AVAILABLE		1029
#define RES_MEM_FREE			1030
#define RES_MEM_TOTAL			1031
#define RES_MEM_PAGESIZE		1032
#define RES_MEM_SWAPFREE		1037
#define RES_MEM_SWAPTOTAL		1038
#define RES_MEM_ACTIVE			1039
#define MEM_MAX				1040

#define NET_MIN				2048
#define RES_NET_IFSTAT			2049
#define RES_NET_ALLIFSTAT		2050
#define NET_MAX				2051

#define KERN_MIN			3072
#define RES_KERN_COMPILE_TIME		3073
#define RES_KERN_RELEASE		3074
#define KERN_MAX			3075

/* Structure to return RES_MEM_INFOALL resource information */
typedef struct res_mem_infoall {
	size_t memfree;
	size_t memtotal;
	size_t memavailable;
	size_t active;
	size_t inactive;
	size_t swaptotal;
	size_t swapfree;
} res_mem_infoall_t;

/* Structure to return RES_MEM_ALLIFSTAT resource information */
typedef struct res_net_ifstat {
	char ifname[IFNAMSIZ];
	unsigned long long rx_bytes;
	unsigned long long rx_packets;
	unsigned long rx_errors;
	unsigned long rx_dropped;
	unsigned long rx_fifo_err;
	unsigned long rx_frame_err;
	unsigned long rx_compressed;
	unsigned long rx_multicast;
	unsigned long long tx_bytes;
	unsigned long long tx_packets;
	unsigned long tx_errors;
	unsigned long tx_dropped;
	unsigned long tx_fifo_err;
	unsigned long tx_collisions;
	unsigned long tx_carrier_err;
	unsigned long tx_compressed;
} res_net_ifstat_t;

/* Allocating memory and building a res_blk structure to return bulk
 * resource information.
 */
extern res_blk_t *res_build_blk(int *res_ids, int res_count);

/* Reading bulk resource information. Memory must be properly allocated and
 * all fields should be properly filled to return error free resource
 * information. res_build_blk call is suggested to allocate build res_blk_t
 * structure.
 */
extern int res_read_blk(res_blk_t *resblk, int pid, int flags);

/* Free allocated memory from res_build_blk */
extern void res_destroy_blk(res_blk_t *resblk);

/* Read a resource information. Memory for out should be properly allocated */
extern int res_read(int res_id, void *out, void *hint, int pid, int flags);

#endif /* RESOURCE_H */
