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

void print_meminfo()
{
        struct memstat mem;

        res_read(RES_MEM_INFOALL, &mem, sizeof(mem), NULL, 0, 0);
        printf("MemTotal: %lu kB\n", mem.memtotal);
        printf("\n");
}

void print_vmstat()
{
        struct vmstat data;
	unsigned long value;

	res_read(RES_VMSTAT_INFO, &data, sizeof(data), NULL, 0, 0);
	// Showing access of just the first element here:
	printf("nr_free_pages %lu\n", data.nr_free_pages);

	res_read(RES_VMSTAT_PGPGIN, &value, sizeof(value), NULL, 0, 0);
	printf("RES_VMSTAT_PGPGIN: %lu\n",value);
	res_read(RES_VMSTAT_PGPGOUT, &value, sizeof(value), NULL, 0, 0);
	printf("RES_VMSTAT_PGPGOUT: %lu\n",value);
	res_read(RES_VMSTAT_SWAPIN, &value, sizeof(value), NULL, 0, 0);
	printf("RES_VMSTAT_SWAPIN: %lu\n",value);
	res_read(RES_VMSTAT_SWAPOUT, &value, sizeof(value), NULL, 0, 0);
	printf("RES_VMSTAT_SWAPOUT: %lu\n",value);

	printf("\n");
}

void print_fs()
{
        unsigned long long fs_value, val[3];

        res_read(FS_AIONR, &fs_value, sizeof(fs_value), NULL, 0, 0);
        printf("FS_AIONR : %Lu\n",fs_value);
        res_read(FS_AIOMAXNR, &fs_value, sizeof(fs_value), NULL, 0, 0);
        printf("FS_AIOMAXNR : %Lu\n",fs_value);
        res_read(FS_FILENR, &val, sizeof(val), NULL, 0, 0);
        printf("FS_FILENR : %Lu %Lu %Lu\n",val[0], val[1], val[2]);
        res_read(FS_FILEMAXNR, &fs_value, sizeof(fs_value), NULL, 0, 0);
        printf("FS_FILEMAXNR : %Lu\n",fs_value);
        printf("\n");
}

void print_cpuinfo()
{
	struct cpuinfo *cpu;
        int cpu_num, i;

        cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
        cpu = (struct cpuinfo *) malloc(cpu_num*sizeof(struct cpuinfo));

        res_read(RES_CPU_INFO, cpu, (cpu_num*sizeof(struct cpuinfo)), NULL, 0, 0);
        for (i = 0; i < cpu_num; i++) {
                printf("processor\t: %u\n",cpu->processor);
                printf("vendor_id\t: %s\n",cpu->vendor_id);
		/* Keep accessing other fields in cpu-> */
		cpu++;
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	/* VMSTAT */
	print_vmstat();

	/* MEMINFO */
	print_meminfo();

	/* CPUINFO */
	print_cpuinfo();

	/* FS */
	print_fs();
}
