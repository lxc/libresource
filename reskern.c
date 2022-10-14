/*
 * Copyright (C) 2022, Oracle and/or its affiliates. All rights reserved
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
 */

#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>
#include "resource.h"
#include "resource_impl.h"

char FILENAME[] = "/home/opc/pids";

int get_pidarr(int **pidarr, int LEN) {
        FILE *fp = NULL;
        int i = 0;
        int *tmparr;

        fp = fopen(FILENAME, "r");
        if (fp == NULL) {
            printf("File %s not found\n",FILENAME);
            return 1;
        }
        *pidarr = (int *) malloc(LEN*sizeof(int));
        if (!*pidarr) {
            printf("Out of memory, pidarr NULL\n");
            return 1;
        }
        i = 0;
        tmparr=*pidarr;
        while ((fscanf(fp, "%d", *pidarr) == 1) && (i < LEN)) {
            //printf("i: %d, pid %d\n",i, **pidarr);
            (*pidarr)++;
            i++;
        }
        *pidarr=tmparr;
        if (i != LEN) {
            printf("i & LEN mismatch, i %d, LEN %d\n", i, LEN);
            LEN = i;
        }
        fclose(fp);
        return 0;
}

int syscall_start_times(int *pidarr, unsigned long long *stimes, int LEN) {
        		unsigned long long a;

        a = syscall(474, RES_KERN_PID_START_TIME, LEN, pidarr, stimes);
        if (a != 0) {
            printf("Error, System call sys_get_vector returned %llu\n",a);
            return 1;
        } 
        return 0;
}

void print_pid_start_times(int *pidarr, unsigned long long *stimes, int LEN) {
	int i;
	for (i = 0; i < LEN; i++) {
    	    printf("[%d] start time of %d is %llu\n", i, pidarr[i], stimes[i]);
	}
}

/* Read PID start time directly from kernel instead of /proc/$pid/stat
 * This reads the start time from the task structure of the process
 * directly from kernel, making it more efficient than going through /proc
 * The field it reads is (from man page of /proc):
 * (22) starttime  %llu
                     The time the process started after system boot.  In
                     kernels before Linux 2.6, this value was expressed
                     in jiffies.  Since Linux 2.6, the value is
                     expressed in clock ticks (divide by
                     sysconf(_SC_CLK_TCK)).
 * Input is a pointer to an array of integers containing the PIDs (int *pidarr)
 * Output is a pointer to an array of unsigned long long (stimes)
 * This should be allocated by the user as before calling res_read_kern as:
 * stimes = (unsigned long long *)malloc(LEN*sizeof(unsigned long long));
 * Appropriately, it should be freed by the caller after it's use
 * len_sz is the length of in or out arrays (the no. of PIDs)
 */
int res_read_kern(int res_id, void *out, size_t len_sz, void *in)
{
	struct utsname t;
        int ret;
        int err;
        char *rawdata;

        switch (res_id) {
        case RES_KERN_RELEASE:
                ret = uname(&t);
                if (ret == -1) {
                        err = errno;
                        printf("Error in reading kernel release");
                        errno = err;
                        return -1;
                }
                strncpy(out, t.release, len_sz-1);
                ((char *)out)[len_sz-1] = '\0';
                break;

        case RES_KERN_COMPILE_TIME:
                ret = uname(&t);
                if (ret == -1) {
                        err = errno;
                        printf("Error in reading version (for compile time)");
                        errno = err;
                        return -1;
                }
                rawdata = skip_spaces(t.version, strlen(t.version), 3);
                strncpy(out, rawdata, len_sz-1);
                ((char *)out)[len_sz-1] = '\0';
                break;

        case RES_KERN_PID_START_TIME:
		if (out == NULL) {
			printf("out argument cannot be NULL");
                        errno = EINVAL;
                        return -1;
		}
		syscall_start_times((int *)in, (unsigned long long *)out, (int)len_sz);
                break;

        default:
                printf("Resource Id is invalid");
                errno = EINVAL;
                return -1;
        }
	return 0;
}

