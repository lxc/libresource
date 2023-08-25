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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <resource.h>

int main(int argc, char **argv)
{
	unsigned long long value, val1[2], val[3];
	unsigned int val2;
	int ret;

	ret = res_read(RES_NET_IP_LOCAL_PORT_RANGE, &val1, sizeof(val1), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        }
	printf("RES_NET_IP_LOCAL_PORT_RANGE : %Lu %Lu\n",val1[0], val1[1]);
	ret = res_read(RES_NET_RMEM_MAX, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        }
	printf("RES_NET_RMEM_MAX : %Lu\n",value);
	ret = res_read(RES_NET_WMEM_MAX, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        }
	printf("RES_NET_WMEM_MAX : %Lu\n",value);
	ret = res_read(RES_NET_TCP_RMEM_MAX, &val, sizeof(val), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        }
	printf("RES_NET_TCP_RMEM_MAX : %Lu %Lu %Lu\n",val[0], val[1], val[2]);
	ret = res_read(RES_NET_TCP_WMEM_MAX, &val, sizeof(val), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        }
	printf("RES_NET_TCP_WMEM_MAX : %Lu %Lu %Lu\n",val[0], val[1], val[2]);
	ret = res_read(RES_CPU_CORECOUNT, &val2, sizeof(val2), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        }
	printf("RES_CPU_CORECOUNT : %u\n",val2);
	ret = res_read(RES_VMSTAT_PGPGIN, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        } else {
		printf("RES_VMSTAT_PGPGIN: %lu\n",value);
	}
	ret = res_read(RES_VMSTAT_PGPGOUT, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        } else {
		printf("RES_VMSTAT_PGPGOUT: %lu\n",value);
	}
	ret = res_read(RES_VMSTAT_SWAPIN, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        } else {
		printf("RES_VMSTAT_SWAPIN: %lu\n",value);
	}
	ret = res_read(RES_VMSTAT_SWAPOUT, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        } else {
		printf("RES_VMSTAT_SWAPOUT: %lu\n",value);
	}
	ret = res_read(RES_VMSTAT_PGALLOC, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        } else {
		printf("RES_VMSTAT_PGALLOC: %lu\n",value);
	}
	ret = res_read(RES_VMSTAT_PGSCAN, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        } else {
		printf("RES_VMSTAT_PGSCAN: %lu\n",value);
	}
	ret = res_read(RES_VMSTAT_PGSTEAL, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        } else {
		printf("RES_VMSTAT_PGSTEAL: %lu\n",value);
	}
	ret = res_read(RES_VMSTAT_PGREFILL, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0) {
                printf("res_read returned error %d\n",ret);
                exit(1);
        } else {
		printf("RES_VMSTAT_PGREFILL: %lu\n",value);
	}
}
