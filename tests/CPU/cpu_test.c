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
        struct cpuinfo *cpu, *exist;
	int cpu_num, i;
	FILE *fp;
	int ret;

	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	cpu = (struct cpuinfo *) malloc(cpu_num*sizeof(struct cpuinfo));
	if (!cpu) {
		printf("Out of memory, malloc for cpu returned NULL\n");
		exit(1);
	}

	exist = (struct cpuinfo *) malloc(cpu_num*sizeof(struct cpuinfo));
	if (!exist) {
		printf("Out of memory, malloc for exist returned NULL\n");
		exit(1);
	}
        ret = res_read(RES_CPU_INFO, cpu, (cpu_num*sizeof(struct cpuinfo)),
			NULL, 0, 0);
	if (ret != 0) {
		printf("res_read returned error %d\n",ret);
		exit(1);
	}

        ret = res_exist(RES_CPU_INFO, exist, (cpu_num*sizeof(struct cpuinfo)),
			NULL, 0, 0);
	if (ret != 0) {
		printf("res_exist returned error %d\n",ret);
		exit(1);
	}
	fp = fopen ("./cpu_info.txt", "w");
	if (fp == NULL) {
		printf("FP null for ./cpu_info.txt\n");
		exit(1);
	}
	for (i = 0; i < cpu_num; i++) {
		if (exist->processor)
			fprintf(fp, "processor\t: %u\n",cpu->processor);
		if (exist->vendor_id[0])
			fprintf(fp, "vendor_id\t: %s\n",cpu->vendor_id);
		if (exist->cpu_family[0])
			fprintf(fp, "cpu family\t: %s\n",cpu->cpu_family);
		if (exist->model[0])
			fprintf(fp, "model\t\t: %s\n",cpu->model);
		if (exist->model_name[0])
			fprintf(fp, "model name\t: %s\n",cpu->model_name);
		if (exist->stepping[0])
			fprintf(fp, "stepping\t: %s\n",cpu->stepping);
		if (exist->microcode[0])
			fprintf(fp, "microcode\t: %s\n",cpu->microcode);
		if (exist->cpu_mhz[0])
			fprintf(fp, "cpu MHz\t\t: %s\n",cpu->cpu_mhz);
		if (exist->cache_size[0])
			fprintf(fp, "cache size\t: %s\n",cpu->cache_size);
		if (exist->physical_id)
			fprintf(fp, "physical id\t: %u\n",cpu->physical_id);
		if (exist->siblings)
			fprintf(fp, "siblings\t: %u\n",cpu->siblings);
		if (exist->core_id)
			fprintf(fp, "core id\t\t: %u\n",cpu->core_id);
		if (exist->cpu_cores)
			fprintf(fp, "cpu cores\t: %u\n",cpu->cpu_cores);
		if (exist->apicid)
			fprintf(fp, "apicid\t\t: %u\n",cpu->apicid);
		if (exist->initial_apicid)
			fprintf(fp, "initial apicid\t: %u\n",cpu->initial_apicid);
		if (exist->fpu[0])
			fprintf(fp, "fpu\t\t: %s\n",cpu->fpu);
		if (exist->fpu_exception[0])
			fprintf(fp, "fpu_exception\t: %s\n",cpu->fpu_exception);
		if (exist->cpu_id_level)
			fprintf(fp, "cpuid level\t: %u\n",cpu->cpu_id_level);
		if (exist->wp[0])
			fprintf(fp, "wp\t\t: %s\n",cpu->wp);
		if (exist->flags[0])
			fprintf(fp, "flags\t\t: %s\n",cpu->flags);
		if (exist->vmx_flags[0])
			fprintf(fp, "vmx flags\t: %s\n",cpu->vmx_flags);
		if (exist->bugs[0])
			fprintf(fp, "bugs\t\t: %s\n",cpu->bugs);
		if (exist->bogomips)
			fprintf(fp, "bogomips\t: %0.2f\n",cpu->bogomips);
		if (exist->tlb_size[0])
			fprintf(fp, "TLB size\t: %s\n",cpu->tlb_size);
		if (exist->clflush_size)
			fprintf(fp, "clflush size\t: %u\n",cpu->clflush_size);
		if (exist->cache_alignment)
			fprintf(fp, "cache_alignment\t: %u\n",cpu->cache_alignment);
		if (exist->address_sizes[0])
			fprintf(fp, "address sizes\t: %s\n",cpu->address_sizes);
		if (exist->power_mgmt[0])
			fprintf(fp, "power management:%s\n",cpu->power_mgmt);
		fprintf(fp, "\n");
		cpu++;
	}
	fclose(fp);
}
