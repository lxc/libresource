/* Copyright (C) 2018, Oracle and/or its affiliates. All rights reserved
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

#ifndef _RESMEM_H
#define _RESMEM_H

#define MEMINFO_FILE "/proc/meminfo"

#define MEMBUF_8	8
#define MEMBUF_128	128
#define MEMBUF_2048	2048

#define MEMCGNAME	"memory"

extern int populate_meminfo(struct res_blk *res, int pid, int flags);
extern int populate_meminfo_cg(res_blk_t *res, int pid, int flags);
extern int getmeminfo(int res_id, void *out, size_t sz,
		void **hint, int pid, int flags);
extern int getmeminfo_cg(int res_id, void *out, size_t sz,
		void **hint, int pid, int flags);
extern int getmemexist(int res_id, void *exist, size_t sz, void *hint,
		int flags);
extern int get_info_infile(char *fname, char *res, void *out);

#endif /* _RESMEM_H */
