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

        default:
                printf("Resource Id is invalid");
                errno = EINVAL;
                return -1;
        }
	return 0;
}

