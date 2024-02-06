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
	int nroutes;
	struct rt_info *rt = NULL, *rtn;

	nroutes = res_read(RES_NET_ROUTE_ALL, NULL, 0, (void **)&rt, 0, 0);
	if (nroutes < 0) {
		printf("res_read() returned %d\n", nroutes);
		exit(1);
	}
	rtn = rt;
#ifdef PRINTLOGS
	for (int i=0;i<nroutes; i++) {
		if (rt->dst_prefix_len != 0) {
			printf("%hhu.%hhu.%hhu.%hhu/%02hhu\n",rt->dest[0], rt->dest[1], rt->dest[2], rt->dest[3], rt->dst_prefix_len);
		}
		rt++;
	}
#endif
	if (rtn)
		free(rtn);
	exit(0);
}
