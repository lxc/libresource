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
	unsigned long long value, val[3];
	int ret;

	ret = res_read(FS_AIONR, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0)
		printf("res_read returned error %d\n",ret);
	else
		printf("FS_AIONR : %Lu\n",value);
	ret = res_read(FS_AIOMAXNR, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0)
		printf("res_read returned error %d\n",ret);
	else
		printf("FS_AIOMAXNR : %Lu\n",value);
	ret = res_read(FS_FILENR, &val, sizeof(val), NULL, 0, 0);
	if (ret != 0)
		printf("res_read returned error %d\n",ret);
	else
		printf("FS_FILENR : %Lu %Lu %Lu\n",val[0], val[1], val[2]);
	ret = res_read(FS_FILEMAXNR, &value, sizeof(value), NULL, 0, 0);
	if (ret != 0)
		printf("res_read returned error %d\n",ret);
	else
		printf("FS_FILEMAXNR : %Lu\n",value);
}
