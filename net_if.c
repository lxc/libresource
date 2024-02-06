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

static int send_stats_req(int net_sock)
{
	int err;
	struct stats {
		struct nlmsghdr nl_hdr;
		struct if_stats_msg if_stm;
	} req;

	memset(&req, 0, sizeof(req));
	req.nl_hdr.nlmsg_type = RTM_GETSTATS;
	req.nl_hdr.nlmsg_len = sizeof(req);
	req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nl_hdr.nlmsg_seq = 0;
	req.nl_hdr.nlmsg_pid = getpid();
	
	req.if_stm.filter_mask = IFLA_STATS_LINK_64;
	req.if_stm.family = AF_UNSPEC;
	req.if_stm.ifindex = 0;

	err = send(net_sock, &req, req.nl_hdr.nlmsg_len, 0);
	if (err == -1) {
		err = errno;
		eprintf("Error in request send(), errno %d\n", err);
		return -err;
	}
	return 0;
}

int parse_if_attr(struct rtattr *at[], struct rtattr *rta, int len)
{
	memset(at, 0, sizeof(struct rtattr *) * (IFLA_STATS_MAX + 1));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= IFLA_STATS_MAX)
			at[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	return 0;
}

void print_if_info(struct ifstats *ifs, FILE *fp)
{
	fprintf(fp, "Interface name %s\n", ifs->ifname);
	fprintf(fp, "rx_packets %llu\n", ifs->st64.rx_packets);
	fprintf(fp, "tx_packets %llu\n", ifs->st64.tx_packets);
	fprintf(fp, "rx_bytes %llu\n", ifs->st64.rx_bytes);
	fprintf(fp, "tx_bytes %llu\n", ifs->st64.tx_bytes);
}

void print_if(struct ifstats *ifs)
{
	printf("Interface name %s\n", ifs->ifname);
	printf("rx_packets %llu\n", ifs->st64.rx_packets);
	printf("tx_packets %llu\n", ifs->st64.tx_packets);
	printf("rx_bytes %llu\n", ifs->st64.rx_bytes);
	printf("tx_bytes %llu\n", ifs->st64.tx_bytes);
}

int get_net_dev(void **out)
{
	int err, net_sock;

	net_sock = connect_netlink(0);
	if (net_sock < 0)
		return net_sock;

	err = send_stats_req(net_sock);
	if (err < 0) {
		close(net_sock);
		return err;
	}

	err = handle_net_resp(NET_IF, net_sock, out);
	close(net_sock);
	return err;
}

/* Get resource information related to network route */
int getdevinfo(int res_id, void *out, size_t sz, void **hint, int flags)
{
	int len;

	switch (res_id) {
	case RES_NET_DEV_ALL:
		len = get_net_dev(hint);
		break;
	default:
		eprintf("Resource Id is invalid");
		return -EINVAL;
	}
	return len;	
}
