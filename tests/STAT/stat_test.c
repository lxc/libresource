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
	struct stat_info stats, *st;
	int cpu_num, size, err = 0;
	FILE *fp;
	char buf[36];
	struct cpu_stat *all_cpu;

	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	size = sizeof(struct cpu_stat) * cpu_num;
	stats.all_cpu = (struct cpu_stat *) malloc(size);
	err = res_read(RES_STAT_INFO, &stats, sizeof(stats)+size, NULL, 0, 0);
	if (err != 0) {
		printf("res_read() err is %d\n",err);
		exit(1);
	}
	st = &stats;
	fp = fopen ("./stat_info.txt", "w");
	if (fp == NULL) {
		printf("stat_info.txt does not exist!\n");
		exit(1);
	}
	fprintf(fp, "cpu  %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu\n",
			st->cpu.user,
			st->cpu.nice,
			st->cpu.system,
			st->cpu.idle,
			st->cpu.iowait,
			st->cpu.irq,
			st->cpu.softirq,
			st->cpu.steal,
			st->cpu.guest,
			st->cpu.guest_nice);
	all_cpu = st->all_cpu;
	for (int i = 0; i < cpu_num; i++) {
		sprintf(buf, "cpu%d", i);
		fprintf(fp, "%s %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu\n",
				buf,
				all_cpu->user,
				all_cpu->nice,
				all_cpu->system,
				all_cpu->idle,
				all_cpu->iowait,
				all_cpu->irq,
				all_cpu->softirq,
				all_cpu->steal,
				all_cpu->guest,
				all_cpu->guest_nice);
		all_cpu++;
	}
	fprintf(fp, "intr %s\n", st->intr);
	fprintf(fp, "ctxt %Lu\n",st->ctxt);
	fprintf(fp, "btime %Lu\n", st->btime);
	fprintf(fp, "processes %Lu\n", st->processes);
	fprintf(fp, "procs_running %Lu\n", st->procs_running);
	fprintf(fp, "procs_blocked %Lu\n", st->procs_blocked);
	fprintf(fp, "softirq %s\n", st->softirq);
	fclose(fp);
}
