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
#include <stdbool.h>
#include "resource_impl.h"
#include "net.h"
    
static int send_route_req(int net_sock)
{
	int err;
	struct route {
		struct nlmsghdr nl_hdr;
		struct rtmsg rt;
	} req;

	memset(&req, 0, sizeof(req));
	req.nl_hdr.nlmsg_type = RTM_GETROUTE;
	req.nl_hdr.nlmsg_len = sizeof(req);
	req.nl_hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nl_hdr.nlmsg_seq = 0;

	req.rt.rtm_family = AF_INET;
	req.rt.rtm_table = RT_TABLE_MAIN;

	err = send(net_sock, &req, req.nl_hdr.nlmsg_len, 0);
	if (err == -1) {
		err = errno;
		eprintf("Error in request send(), errno %d\n", err);
		return -err;
	}
	return 0;
}

#ifdef TESTING
#define IPA_F "%hhu.%hhu.%hhu.%hhu"
#define IPA_V(str) str[0], str[1], str[2], str[3]
#define V4_PREFIX_LEN 32

static inline bool is_zero(char *str, int len)
{
	for (int i = 0; i < len; i++)
		if (str[i] != 0)
			return false;
	return true;
}

static inline const char *scope_str(int scope)
{
	switch (scope) {
	case RT_SCOPE_UNIVERSE: return "universe";
	case RT_SCOPE_SITE:     return "site";
	case RT_SCOPE_LINK:     return "link";
	case RT_SCOPE_HOST:     return "host";
	case RT_SCOPE_NOWHERE:  return "nowhere";
	default: 		return "invalid";
	}
}

static inline const char *proto_str(int proto)
{
	switch (proto) {
	case RTPROT_DHCP: 	return "dhcp";
	case RTPROT_KERNEL: 	return "kernel";
	case RTPROT_STATIC: 	return "static";
	case RTPROT_BGP: 	return "bgp";
	case RTPROT_ISIS: 	return "isis";
	case RTPROT_OSPF: 	return "ospf";
	case RTPROT_RIP: 	return "rip";
	case RTPROT_EIGRP: 	return "eigrp";
	default: 		return "other";
	}
}

void print_rt_info(struct rt_info *rt, FILE *fp)
{
        char ifname[IF_NAMESIZE];

	if (rt->family == AF_INET) {
		if (rt->dst_prefix_len == 0) {
			fprintf(fp, "default");
		} else {
			fprintf(fp, IPA_F, IPA_V(rt->dest));
			if (rt->dst_prefix_len != V4_PREFIX_LEN)
				fprintf(fp, "/%02hhu", rt->dst_prefix_len);
		}
		if (!is_zero(rt->gate, MAX_BYTES_IPV4)) {
			fprintf(fp, " via ");
			fprintf(fp, IPA_F, IPA_V(rt->gate));
		}
		fprintf(fp, " dev %s", if_indextoname(rt->index, ifname));
		fprintf(fp, " proto %s", proto_str(rt->protocol));
		if (rt->scope != RT_SCOPE_UNIVERSE)
			fprintf(fp, " scope %s", scope_str(rt->scope));
		if (!is_zero(rt->prefsrc, MAX_BYTES_IPV4)) {
			fprintf(fp, " src ");
			fprintf(fp, IPA_F, IPA_V(rt->prefsrc));
		}
		if (rt->metric != 0)
			fprintf(fp, " metric %d \n", rt->metric);
		else
			fprintf(fp, " \n");
	}
}
#endif

int get_rt_attr(struct rtattr *at[], struct rt_info *rt, struct rtmsg *m)
{
	bzero(rt, sizeof(*rt));
	rt->family = m->rtm_family;
	rt->protocol = m->rtm_protocol;
	rt->scope = m->rtm_scope;
	if (at[RTA_TABLE])
		rt->table = *(int *)RTA_DATA(at[RTA_TABLE]);
	if (at[RTA_OIF])
		rt->index = *(int *)RTA_DATA(at[RTA_OIF]);
	if (at[RTA_DST]) {
		if (m->rtm_family == AF_INET) {
			memcpy(rt->dest, RTA_DATA(at[RTA_DST]), MAX_BYTES_IPV4);
			rt->dst_prefix_len = m->rtm_dst_len;
		}
		if (m->rtm_family == AF_INET6) {
			memcpy(rt->dest, RTA_DATA(at[RTA_DST]), MAX_BYTES_IPV6);
			rt->dst_prefix_len = m->rtm_dst_len;
		}
	}
	if (at[RTA_SRC]) {
		if (m->rtm_family == AF_INET) {
#ifdef PRINTLOGS
			printf("RTA_SRC for AF_INET!!\n");
#endif
		}
		if (m->rtm_family == AF_INET6) {
			memcpy(rt->src, RTA_DATA(at[RTA_SRC]), MAX_BYTES_IPV6);
			rt->src_prefix_len = m->rtm_src_len;
		}
	}
	if (at[RTA_PREFSRC]) {
		if (m->rtm_family == AF_INET)
			memcpy(rt->prefsrc, RTA_DATA(at[RTA_PREFSRC]), MAX_BYTES_IPV4);
		if (m->rtm_family == AF_INET6)
			memcpy(rt->prefsrc, RTA_DATA(at[RTA_PREFSRC]), MAX_BYTES_IPV6);
	}
	if (at[RTA_GATEWAY]) {
		if (m->rtm_family == AF_INET)
			memcpy(rt->gate, RTA_DATA(at[RTA_GATEWAY]), MAX_BYTES_IPV4);
		if (m->rtm_family == AF_INET6)
			memcpy(rt->gate, RTA_DATA(at[RTA_GATEWAY]), MAX_BYTES_IPV6);
	}
	if (at[RTA_PRIORITY])
		rt->metric = *(int *)RTA_DATA(at[RTA_PRIORITY]);
	return 0;
}

int parse_rt_attr(struct rtattr *at[], struct rtattr *rta, int len)
{
	memset(at, 0, sizeof(struct rtattr *) * (RTA_MAX + 1));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= RTA_MAX)
			at[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	return 0;
}

void print_rt(struct rtmsg *rt)
{
	printf("family: %hhu\n", rt->rtm_family);
	printf("rtm_dst_len %hhu\n",rt->rtm_dst_len);
	printf("rtm_src_len %hhu\n",rt->rtm_src_len);
	printf("rtm_type: %hhu\n", rt->rtm_type);
	printf("rtm_flags: %u\n",rt->rtm_flags);
	printf("rtm_protocol: %hhu\n",rt->rtm_protocol);
}

int get_net_route(void **out)
{
	int err, net_sock;

	net_sock = connect_netlink(0);
	if (net_sock < 0)
		return net_sock;

	err = send_route_req(net_sock);
	if (err < 0) {
		close(net_sock);
		return err;
	}

	err = handle_net_resp(NET_ROUTE, net_sock, out);
	close(net_sock);
	return err;
}

/* Get resource information related to network route */
int getrouteinfo(int res_id, void *out, size_t sz, void **hint, int flags)
{
	int len;

	switch (res_id) {
	case RES_NET_ROUTE_ALL:
		len = get_net_route(hint);
		break;
	default:
		eprintf("Resource Id is invalid");
		return -EINVAL;
	}
	return len;
}
