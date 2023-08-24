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

#ifndef _RESNET_H
#define _RESNET_H

#define NETDEV_FILE "/proc/net/dev"
#define IP_PORT_RANGE "/proc/sys/net/ipv4/ip_local_port_range"
#define TCP_RMEM_MAX "/proc/sys/net/ipv4/tcp_rmem"
#define TCP_WMEM_MAX "/proc/sys/net/ipv4/tcp_wmem"
#define CORE_RMEM_MAX "/proc/sys/net/core/rmem_max"
#define CORE_WMEM_MAX "/proc/sys/net/core/wmem_max"

#define NET_ALLIFSTAT_SZ    	1
#define NETBUF_1024		1024

extern int populate_netinfo(struct res_blk *res, int pid, int flags);
extern int getnetinfo(int res_id, void *out, size_t sz, void *hint, int pid,
		int flags);
extern int getrouteinfo(int res_id, void *out, size_t sz, void **hint, int flags);
extern int getarpinfo(int res_id, void *out, size_t sz, void **hint, int flags);
extern int getdevinfo(int res_id, void *out, size_t sz, void **hint, int flags);

#endif /* _RESNET_H */
