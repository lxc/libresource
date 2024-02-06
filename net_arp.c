#ifndef _RESOURCE_H
#include "resource.h"
#endif
#include <net/if.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <linux/if_arp.h>
#include <linux/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include "resource_impl.h"
#include "net.h"
    
static int send_arp_req(int net_sock)
{
	int err;
	struct arp {
		struct nlmsghdr nl_hdr;
		struct ndmsg nd;
	} req;

	memset(&req, 0, sizeof(req));
	req.nl_hdr.nlmsg_type = RTM_GETNEIGH;
	req.nl_hdr.nlmsg_len = sizeof(req);
	//req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
	req.nl_hdr.nlmsg_seq = 0;
	//req.nl_hdr.nlmsg_pid = getpid();
	req.nd.ndm_family = AF_INET;

	err = send(net_sock, &req, req.nl_hdr.nlmsg_len, 0);
	if (err == -1) {
		err = errno;
		eprintf("Error in request send(), errno %d\n", err);
		return -err;
	}
	return 0;
}

int parse_arp_attr(struct rtattr *at[], struct rtattr *rta, int len)
{
	memset(at, 0, sizeof(struct rtattr *) * (NDA_MAX + 1));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= NDA_MAX)
			at[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	return 0;
}

#ifdef TESTING
int get_arp_attr(struct rtattr *at[], struct arp_info *arp,
		struct ndmsg *n,
		FILE *fp)
#else
int get_arp_attr(struct rtattr *at[], struct arp_info *arp,
		struct ndmsg *n)
#endif
{
#ifdef TESTING
	char ether_type[] = "ether";
	char ifname[IF_NAMESIZE];
#endif
	bzero(arp, sizeof(*arp));
	if (n->ndm_family == AF_INET) {
		if (at[NDA_DST]) {
			arp->arp_ip_addr_len = RTA_PAYLOAD(at[NDA_DST]);
			memcpy(arp->arp_ip_addr, RTA_DATA(at[NDA_DST]), arp->arp_ip_addr_len);
			arp->arp_ip_addr[arp->arp_ip_addr_len] = '\0';
#ifdef PRINTLOGS
			printf("ARP IP len: %lu\n",arp->arp_ip_addr_len);
			printf("ARP IP ADDR: %hhu.%hhu.%hhu.%hhu\n", arp->arp_ip_addr[0], arp->arp_ip_addr[1], arp->arp_ip_addr[2], arp->arp_ip_addr[3]);
#endif
#ifdef TESTING
			fprintf(fp, "(%hhu.%hhu.%hhu.%hhu) at ", arp->arp_ip_addr[0], arp->arp_ip_addr[1], arp->arp_ip_addr[2], arp->arp_ip_addr[3]);
#endif
		}
		if (at[NDA_LLADDR]) {
			arp->arp_phys_addr_len = RTA_PAYLOAD(at[NDA_LLADDR]);
			memcpy(arp->arp_phys_addr, RTA_DATA(at[NDA_LLADDR]), arp->arp_phys_addr_len);
#ifdef PRINTLOGS
			printf("ARP PHYS len: %lu\n",arp->arp_phys_addr_len);
			printf("ARP PHYS ADDR: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n", arp->arp_phys_addr[0], arp->arp_phys_addr[1], arp->arp_phys_addr[2], arp->arp_phys_addr[3], arp->arp_phys_addr[4], arp->arp_phys_addr[5]);
#endif
#ifdef TESTING
			fprintf(fp, "%02hhx:%02x:%02hhx:%02hhx:%02hhx:%02hhx ", arp->arp_phys_addr[0], arp->arp_phys_addr[1], arp->arp_phys_addr[2], arp->arp_phys_addr[3], arp->arp_phys_addr[4], arp->arp_phys_addr[5]);
#endif
		}
#ifdef TESTING
		fprintf(fp, "[%s] on %s\n", ether_type, if_indextoname(n->ndm_ifindex, ifname));
#endif
	}
	return 0;
}

int get_net_arp(void **out)
{
	int err, net_sock;

	net_sock = connect_netlink(1);
	if (net_sock < 0)
		return net_sock;

	err = send_arp_req(net_sock);
	if (err < 0) {
		close(net_sock);
		return err;
	}

	err = handle_net_resp(NET_ARP, net_sock, out);
	close(net_sock);
	return err;
}

/* Get resource information related to network route */
int getarpinfo(int res_id, void *out, size_t sz, void **hint, int flags)
{
	int len;

	switch (res_id) {
	case RES_NET_ARP_ALL:
		len = get_net_arp(hint);
		break;
	default:
		eprintf("Resource Id is invalid");
		return -EINVAL;
	}
	return len;
}
