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
        struct cpuinfo *cpu;
	int cpu_num, i;
	FILE *fp;
	int ret;

	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	cpu = (struct cpuinfo *) malloc(cpu_num*sizeof(struct cpuinfo));
	if (!cpu) {
		printf("Out of memory, malloc for cpu returned NULL\n");
		exit(1);
	}

        ret = res_read(RES_CPU_INFO, cpu, (cpu_num*sizeof(struct cpuinfo)), NULL, 0, 0);
	if (ret != 0) {
		printf("res_read returned error %d\n",ret);
		exit(1);
	}
	fp = fopen ("./cpu_info.txt", "w");
	if (fp == NULL) {
		printf("FP null for ./cpu_info.txt\n");
		exit(1);
	}
	for (i = 0; i < cpu_num; i++) {
		fprintf(fp, "processor\t: %u\n",cpu->processor);
		fprintf(fp, "vendor_id\t: %s\n",cpu->vendor_id);
		fprintf(fp, "cpu family\t: %s\n",cpu->cpu_family);
		fprintf(fp, "model\t\t: %s\n",cpu->model);
		fprintf(fp, "model name\t: %s\n",cpu->model_name);
		fprintf(fp, "stepping\t: %s\n",cpu->stepping);
		fprintf(fp, "microcode\t: %s\n",cpu->microcode);
		fprintf(fp, "cpu MHz\t\t: %s\n",cpu->cpu_mhz);
		fprintf(fp, "cache size\t: %s\n",cpu->cache_size);
		fprintf(fp, "physical id\t: %u\n",cpu->physical_id);
		fprintf(fp, "siblings\t: %u\n",cpu->siblings);
		fprintf(fp, "core id\t\t: %u\n",cpu->core_id);
		fprintf(fp, "cpu cores\t: %u\n",cpu->cpu_cores);
		fprintf(fp, "apicid\t\t: %u\n",cpu->apicid);
		fprintf(fp, "initial apicid\t: %u\n",cpu->initial_apicid);
		fprintf(fp, "fpu\t\t: %s\n",cpu->fpu);
		fprintf(fp, "fpu_exception\t: %s\n",cpu->fpu_exception);
		fprintf(fp, "cpuid level\t: %u\n",cpu->cpu_id_level);
		fprintf(fp, "wp\t\t: %s\n",cpu->wp);
		fprintf(fp, "flags\t\t: %s\n",cpu->flags);
		fprintf(fp, "vmx flags\t: %s\n",cpu->vmx_flags);
		fprintf(fp, "bugs\t\t: %s\n",cpu->bugs);
		fprintf(fp, "bogomips\t: %0.2f\n",cpu->bogomips);
		fprintf(fp, "clflush size\t: %u\n",cpu->clflush_size);
		fprintf(fp, "cache_alignment\t: %u\n",cpu->cache_alignment);
		fprintf(fp, "address sizes\t: %s\n",cpu->address_sizes);
		fprintf(fp, "power management:%s\n",cpu->power_mgmt);
		fprintf(fp, "\n");
		cpu++;
	}
	fclose(fp);
}
