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
	struct memstat mem, exist;
	FILE *fp;
	int ret;

	ret = res_read(RES_MEM_INFOALL, &mem, sizeof(mem), NULL, 0, 0);
	if (ret != 0) {
		printf("res_read returned error %d\n",ret);
		exit(1);
	}
	ret = res_exist(RES_MEM_INFOALL, &exist, sizeof(exist), NULL, 0, 0);
	if (ret != 0) {
		printf("res_exist returned error %d\n",ret);
		exit(1);
	}
	fp = fopen ("./mem_info.txt", "w");
	if (fp == NULL) {
		printf("mem_info.txt does not exist!\n");
		exit(1);
	}
	if (exist.memtotal)
		fprintf(fp, "MemTotal: %lu kB\n", mem.memtotal);
	if (exist.memfree)
		fprintf(fp, "MemFree: %lu kB\n", mem.memfree);
	if (exist.memavailable)
		fprintf(fp, "MemAvailable: %lu kB\n", mem.memavailable);
	if (exist.buffers)
		fprintf(fp, "Buffers: %lu kB\n", mem.buffers);
	if (exist.cached)
		fprintf(fp, "Cached: %lu kB\n", mem.cached);
	if (exist.swapcached)
		fprintf(fp, "SwapCached: %lu kB\n", mem.swapcached);
	if (exist.active)
		fprintf(fp, "Active: %lu kB\n", mem.active);
	if (exist.inactive)
		fprintf(fp, "Inactive: %lu kB\n", mem.inactive);
	if (exist.active_anon)
		fprintf(fp, "Active(anon): %lu kB\n", mem.active_anon);
	if (exist.inactive_anon)
		fprintf(fp, "Inactive(anon): %lu kB\n", mem.inactive_anon);
	if (exist.active_file)
		fprintf(fp, "Active(file): %lu kB\n", mem.active_file);
	if (exist.inactive_file)
		fprintf(fp, "Inactive(file): %lu kB\n", mem.inactive_file);
	if (exist.unevictable)
		fprintf(fp, "Unevictable: %lu kB\n", mem.unevictable);
	if (exist.mlocked)
		fprintf(fp, "Mlocked: %lu kB\n", mem.mlocked);
	if (exist.swaptotal)
		fprintf(fp, "SwapTotal: %lu kB\n", mem.swaptotal);
	if (exist.swapfree)
		fprintf(fp, "SwapFree: %lu kB\n", mem.swapfree);
	if (exist.zswap)
		fprintf(fp, "Zswap: %lu kB\n", mem.zswap);
	if (exist.zswapped)
		fprintf(fp, "Zswapped: %lu kB\n", mem.zswapped);
	if (exist.dirty)
		fprintf(fp, "Dirty: %lu kB\n", mem.dirty);
	if (exist.writeback)
		fprintf(fp, "Writeback: %lu kB\n", mem.writeback);
	if (exist.anonpages)
		fprintf(fp, "AnonPages: %lu kB\n", mem.anonpages);
	if (exist.mapped)
		fprintf(fp, "Mapped: %lu kB\n", mem.mapped);
	if (exist.shmem)
		fprintf(fp, "Shmem: %lu kB\n", mem.shmem);
	if (exist.kreclaimable)
		fprintf(fp, "KReclaimable: %lu kB\n", mem.kreclaimable);
	if (exist.slab)
		fprintf(fp, "Slab: %lu kB\n", mem.slab);
	if (exist.sreclaimable)
		fprintf(fp, "SReclaimable: %lu kB\n", mem.sreclaimable);
	if (exist.sunreclaim)
		fprintf(fp, "SUnreclaim: %lu kB\n", mem.sunreclaim);
	if (exist.kernelstack)
		fprintf(fp, "KernelStack: %lu kB\n", mem.kernelstack);
	if (exist.pagetables)
		fprintf(fp, "PageTables: %lu kB\n", mem.pagetables);
	if (exist.secpagetables)
		fprintf(fp, "SecPageTables: %lu kB\n", mem.secpagetables);
	if (exist.nfs_unstable)
		fprintf(fp, "NFS_Unstable: %lu kB\n", mem.nfs_unstable);
	if (exist.bounce)
		fprintf(fp, "Bounce: %lu kB\n", mem.bounce);
	if (exist.writebacktmp)
		fprintf(fp, "WritebackTmp: %lu kB\n", mem.writebacktmp);
	if (exist.commitlimit)
		fprintf(fp, "CommitLimit: %lu kB\n", mem.commitlimit);
	if (exist.committed_as)
		fprintf(fp, "Committed_AS: %lu kB\n", mem.committed_as);
	if (exist.vmalloc_total)
		fprintf(fp, "VmallocTotal: %lu kB\n", mem.vmalloc_total);
	if (exist.vmalloc_used)
		fprintf(fp, "VmallocUsed: %lu kB\n", mem.vmalloc_used);
	if (exist.vmalloc_chunk)
		fprintf(fp, "VmallocChunk: %lu kB\n", mem.vmalloc_chunk);
	if (exist.percpu)
		fprintf(fp, "Percpu: %lu kB\n", mem.percpu);
	if (exist.hardware_corrupted)
		fprintf(fp, "HardwareCorrupted: %lu kB\n", mem.hardware_corrupted);
	if (exist.anon_hugepages)
		fprintf(fp, "AnonHugePages: %lu kB\n", mem.anon_hugepages);
	if (exist.shmem_hugepages)
		fprintf(fp, "ShmemHugePages: %lu kB\n", mem.shmem_hugepages);
	if (exist.shmem_pmd_mapped)
		fprintf(fp, "ShmemPmdMapped: %lu kB\n", mem.shmem_pmd_mapped);
	if (exist.file_hugepages)
		fprintf(fp, "FileHugePages: %lu kB\n", mem.file_hugepages);
	if (exist.file_pmd_mapped)
		fprintf(fp, "FilePmdMapped: %lu kB\n", mem.file_pmd_mapped);
	if (exist.cma_total)
		fprintf(fp, "CmaTotal: %lu kB\n", mem.cma_total);
	if (exist.cma_free)
		fprintf(fp, "CmaFree: %lu kB\n", mem.cma_free);
	if (exist.hugepages_total)
		fprintf(fp, "HugePages_Total: %lu\n", mem.hugepages_total);
	if (exist.hugepages_free)
		fprintf(fp, "HugePages_Free: %lu\n", mem.hugepages_free);
	if (exist.hugepages_rsvd)
		fprintf(fp, "HugePages_Rsvd: %lu\n", mem.hugepages_rsvd);
	if (exist.hugepages_surp)
		fprintf(fp, "HugePages_Surp: %lu\n", mem.hugepages_surp);
	if (exist.hugepages_size)
		fprintf(fp, "Hugepagesize: %lu kB\n", mem.hugepages_size);
	if (exist.hugetlb)
		fprintf(fp, "Hugetlb: %lu kB\n", mem.hugetlb);
	if (exist.directmap_4k)
		fprintf(fp, "DirectMap4k: %lu kB\n", mem.directmap_4k);
	if (exist.directmap_2M)
		fprintf(fp, "DirectMap2M: %lu kB\n", mem.directmap_2M);
	if (exist.directmap_1G)
		fprintf(fp, "DirectMap1G: %lu kB\n", mem.directmap_1G);
	fclose(fp);
	// Run "cat /proc/meminfo > mem_info1.orig"
	// ./resmem
	// then "cat ./mem_info1.orig | sed 's/[ ]\+/ /g' > mem_info.orig"
	// then "diff mem_info.orig mem_info.txt"
}
