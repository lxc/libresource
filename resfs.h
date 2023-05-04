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

#ifndef _RESFS_H
#define _RESFS_H

#define AIONR "/proc/sys/fs/aio-nr"
#define AIOMAXNR "/proc/sys/fs/aio-max-nr"
#define FILENR "/proc/sys/fs/file-nr"
#define FILEMAXNR "/proc/sys/fs/file-max"

extern int getfsinfo(int res_id, void *out, size_t sz, void **hint, int flags);
#endif /* _RESFS_H */
