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
	int narps;
	struct arp_info *arp = NULL, *arpn;

	narps = res_read(RES_NET_ARP_ALL, NULL, 0, (void **)&arp, 0, 0);
	if (narps < 0) {
		printf("res_read() returned %d\n", narps);
		exit(1);
	}
	arpn = arp;
	for (int i=0;i<narps; i++) {
		if (arp->arp_ip_addr_len != 0) {
			printf("(%hhu.%hhu.%hhu.%hhu) at ", arp->arp_ip_addr[0], arp->arp_ip_addr[1], arp->arp_ip_addr[2], arp->arp_ip_addr[3]);
		}
		if (arp->arp_phys_addr_len != 0) {
			printf("%02hhx:%02x:%02hhx:%02hhx:%02hhx:%02hhx ", arp->arp_phys_addr[0], arp->arp_phys_addr[1], arp->arp_phys_addr[2], arp->arp_phys_addr[3], arp->arp_phys_addr[4], arp->arp_phys_addr[5]);
		}
		printf("\n");
		arp++;
	}
	if (arpn)
		free(arpn);
	exit(0);
}
