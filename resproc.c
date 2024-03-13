/* Copyright (C) 2018, Oracle and/or its affiliates. All rights reserved
 *
 * This file is part of libresource.
 *
 * libresource is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * libresource is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General public License for more details.
 *
 * You should have received a copy of the GNU Lesser General public License
 * along with libresource. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _RESOURCE_H
#include "resource.h"
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "resproc.h"
#include "resource_impl.h"
#include <sys/stat.h>
#include <pwd.h>

/* Reading information from /proc/<pid>/statm */
static void statm_to_proc(char* buf, res_proc_infoall_t *p) {
	sscanf(buf, "%ld %ld %ld %ld %ld %ld %ld",
		&p->size, &p->resident, &p->share,
		&p->text, &p->lib, &p->data, &p->dt);
}

/* Reading information from /proc/<pid>/stat */
static void stat_to_proc(char* buf, res_proc_infoall_t *p) {
	unsigned int num;
	char* tmp;

	/* fill in default values for older kernels */
	p->processor = 0;
	p->rtprio = -1;
	p->policy = -1;
	p->nlwp = 0;

	/* Fill the command to run process */
	buf = strchr(buf, '(') + 1;
	tmp = strrchr(buf, ')');
	num = tmp - buf;
	if (num >= sizeof(p->cmd))
		num = sizeof(p->cmd) - 1;
	memcpy(p->cmd, buf, num);
	p->cmd[num] = '\0';
	buf = tmp + 2;

	/* Now we are just after command in the string starting from status
	 * of the process.
	 */
	sscanf(buf,
		"%c "
		"%d %d %d %d %d "
		"%lu %lu %lu %lu %lu "
		 /* utime stime cutime cstime */
		"%lu %lu %lu %lu "
		"%ld %ld "
		"%d "
		"%ld "
		/* start_time */
		"%lu "
		"%lu "
		"%ld "
		"%lu "
		/* start_code, end_code, start_stack */
		"%lu %lu %lu "
		"%lu %lu "
		/* discard, no RT signals & Linux 2.1 used hex */
		"%*s %*s %*s %*s "
		"%lu "
		"%*s %*s "
		"%d %d "
		"%lu %lu",
		&p->state,
		&p->ppid, &p->pgrp, &p->session, &p->tty, &p->tpgid,
		&p->flags, &p->min_flt, &p->cmin_flt, &p->maj_flt, &p->cmaj_flt,
		&p->utime, &p->stime, &p->cutime, &p->cstime,
		&p->priority, &p->nice,
		&p->nlwp,
		&p->alarm,
		&p->start_time,
		&p->vsize,
		&p->rss,
		&p->rss_rlim,
		&p->start_code, &p->end_code, &p->start_stack,
		&p->kstk_esp, &p->kstk_eip,
		/* p->signal, p->blocked, p->sigignore, p->sigcatch, can't use */
		&p->wchan,
		/* &p->nswap, &p->cnswap,  nswap and cnswap dead for 2.4.xx and up */
		/* -- Linux 2.0.35 ends here -- */
		&p->exit_signal, &p->processor,
		/* -- Linux 2.2.8 to 2.5.17 end here -- */
		&p->rtprio, &p->policy
	);

	if (!p->nlwp)
		p->nlwp = 1;
}

static void get_user_info(char *path, res_proc_infoall_t *p) {
	static struct stat sb;
	struct passwd *pw = NULL;


	/* Get user id and use that to get useranme */
	if (stat(path, &sb) == -1)
		eprintf("Error in reading stat");

	p->euid = sb.st_uid;
	pw = getpwuid(p->euid);
	if (!pw || strlen(pw->pw_name) >= RES_USR_NAME_SZ)
		snprintf(p->euser, RES_USR_NAME_SZ, "%u", p->euid);
	else
		strncpy(p->euser, pw->pw_name, RES_USR_NAME_SZ);
}

/* Read resource information corresponding to res_id */
int getprocinfo(int res_id, void *out, size_t sz, void *hint, int pid, int flags)
{
	char fstat[FNAMELEN];
	char fstatm[FNAMELEN];
	res_proc_infoall_t *p;
	char buf[PROCBUF_1024];

	switch (res_id) {
	case RES_PROC_INFOALL:
		CHECK_SIZE(sz, sizeof(res_proc_infoall_t));

		p = (res_proc_infoall_t *)out;

		if (pid) {
			p->pid = pid;
			snprintf(fstat, FNAMELEN, "/proc/%d/stat", pid);
			snprintf(fstatm, FNAMELEN, "/proc/%d/statm", pid);
        	} else {
			p->pid = getpid();
			snprintf(fstat, FNAMELEN, "/proc/self/stat");
			snprintf(fstatm, FNAMELEN, "/proc/self/statm");
        	}

		get_user_info(fstat, p);
		file_to_buf(fstat, buf, PROCBUF_1024);
		stat_to_proc(buf, p);
		file_to_buf(fstatm, buf, PROCBUF_1024);
		statm_to_proc(buf, p);

		break;

	default:
		eprintf("Resource Id is invalid");
		errno = EINVAL;
		return -1;
	}

#undef CHECK_SIZE

	return 0;
}

int populate_procinfo(res_blk_t *res, int pid, int flags)
{
        for (int i = 0; i < res->res_count; i++) {
                switch (res->res_unit[i]->res_id) {
                case RES_PROC_INFOALL:
			getprocinfo(RES_PROC_INFOALL,
				(res->res_unit[i]->data).ptr,
				res->res_unit[i]->data_sz,
				NULL, pid, flags);
			res->res_unit[i]->status = 0;
		}
	}
	return 0;
}
