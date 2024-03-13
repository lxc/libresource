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

#ifndef _RESOURCE_H
#include "resource.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "resfs.h"
#include "resource_impl.h"
#include <errno.h>
#include <libgen.h>

int getfsinfo(int res_id, void *out, size_t sz, void **hint, int flags)
{
	int ret;
	unsigned long long *fs;
	char buffer[512];

	switch (res_id) {
	case FS_AIONR:
		ret = file_to_buf(AIONR, buffer, sizeof(buffer));
		if (ret < 0)
			return ret;
		ret = sscanf(buffer, "%Lu", (unsigned long long *) out);
		if (ret != 1)
			return -1;
		break;
	case FS_AIOMAXNR:
		ret = file_to_buf(AIOMAXNR, buffer, sizeof(buffer));
		if (ret < 0)
			return ret;
		ret = sscanf(buffer, "%Lu", (unsigned long long *) out);
		if (ret != 1)
			return -1;
		break;
	case FS_FILENR:
		fs = (unsigned long long *) out;
		ret = file_to_buf(FILENR, buffer, sizeof(buffer));
		if (ret < 0)
			return ret;
		ret = sscanf(buffer, "%Lu %Lu %Lu", &fs[0], &fs[1], &fs[2]);
		if (ret != 3)
			return -1;
		break;
	case FS_FILEMAXNR:
		ret = file_to_buf(FILEMAXNR, buffer, sizeof(buffer));
		if (ret < 0)
			return ret;
		ret = sscanf(buffer, "%Lu", (unsigned long long *) out);
		if (ret != 1)
			return -1;
		break;
	default:
		eprintf("Resource Id is invalid");
		errno = EINVAL;
		return -1;
	}
	return 0;

}

