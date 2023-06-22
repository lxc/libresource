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
    
static int count, ecount;

static int connect_route()
{
	struct sockaddr_nl nl_saddr;
	int err, net_sock;

	net_sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (net_sock == -1) {
		err = errno;
		eprintf("Error in socket(), errno %d\n", err);
		errno = err;
		return -1;
	}
	bzero(&nl_saddr, sizeof(nl_saddr));
	nl_saddr.nl_family = AF_NETLINK;
	err = bind(net_sock, (struct sockaddr *)&nl_saddr, sizeof(nl_saddr));
	if (err == -1) {
		err = errno;
		eprintf("Error in bind(), errno %d\n", err);
		close(net_sock);
		errno = err;
		return -1;
	}
	return net_sock;
}

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

	err = send(net_sock, &req, req.nl_hdr.nlmsg_len, 0);
	if (err == -1) {
		err = errno;
		eprintf("Error in request send(), errno %d\n", err);
		errno = err;
		return -1;
	}
	return 0;
}

static int get_attr(struct rtattr *at[], struct rt_info *rt, struct rtmsg *m)
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

static int parse_attr(struct rtattr *at[], struct rtattr *rta, int len)
{
	memset(at, 0, sizeof(struct rtattr *) * (RTA_MAX + 1));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= RTA_MAX)
			at[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	return 0;
}

static int get_route_len(int net_sock)
{
	int recvl, err;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl nladdr;

	memset(&msg, 0, sizeof(msg));
	memset(&iov, 0, sizeof(iov));

	msg.msg_name = &nladdr;
	msg.msg_namelen = sizeof(nladdr);
	iov.iov_base = NULL;
	iov.iov_len = 0;

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	recvl = recvmsg(net_sock, &msg, MSG_PEEK | MSG_TRUNC);
	if (recvl < 0) {
		err = errno;
		eprintf("Error in get length recvmsg(), errno %d\n", err);
		errno = err;
		return -1;
	}
#ifdef PRINTLOGS
	printf("received len %d\n",recvl);
#endif
	return recvl;
}

static int get_max_routes(int len)
{
	int header_size, max_routes;

	header_size = (sizeof(struct nlmsghdr)) + (sizeof(struct rtmsg));
	max_routes = len/header_size;
	return max_routes;
}

static int handle_route_resp(int net_sock, void **out)
{
	int err, recvl, len, max_routes, rt_size;
	int rtind = 0, total_rt_size = 0;
	struct iovec iov;
	struct msghdr msg;
	struct nlmsghdr *r;
	struct nlmsgerr *nlmsg_err;
	struct rtmsg *rt;
	struct rtattr *at[RTA_MAX + 1];
	struct rt_info *routes = NULL, *iroutes = NULL;
	struct sockaddr_nl nladdr;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &nladdr;
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	while(1) {
		len = get_route_len(net_sock);
		if (len == -1) {
			if (routes)
				free(routes);
			return -1;
		}
		char *buf = (char *)malloc(len);
		if (!buf) {
			if (routes)
				free(routes);
			return -1;
		}
		memset(&iov, 0, sizeof(iov));
		iov.iov_base = buf;
		iov.iov_len = len;
		max_routes = get_max_routes(len);
		rt_size = max_routes * sizeof(struct rt_info);
		total_rt_size += rt_size;

		void *tmp = (struct rt_info *) realloc(routes, total_rt_size);
		if (!tmp) {
			if (routes)
				free(routes);
			free(buf);
			return -1;
		}
		routes = tmp;
		iroutes = routes + rtind;

		recvl = recvmsg(net_sock, &msg, 0);
		if (recvl < 0) {
			err = errno;
			free(routes);
			free(buf);
			eprintf("Error in response recvmsg(), errno %d\n", err);
			errno = err;
			return -1;
		}
#ifdef PRINTLOGS
		printf("received len %d\n",recvl);
#endif
		r = (struct nlmsghdr *) buf;
		while (NLMSG_OK(r, recvl)) {
			if (r->nlmsg_type == NLMSG_ERROR) {
				nlmsg_err = (struct nlmsgerr *)NLMSG_DATA(r);
				eprintf("NLMSG_ERROR error: %d\n",
						nlmsg_err->error);
				ecount++;
				free(buf);
				free(routes);
				errno = nlmsg_err->error;
				return -1;
			} else if (r->nlmsg_type == NLMSG_DONE) {
#ifdef PRINTLOGS
				printf("DONE\n");
#endif
				free(buf);
				*(struct rt_info **)out = routes;
				return rtind;
			}
			rt = (struct rtmsg *)NLMSG_DATA(r);
			len = r->nlmsg_len;
#ifdef PRINTLOGS
			printf("r->nlmsg_type is %d (RTM_NEWROUTE = 24)\n", r->nlmsg_type);
			printf("Received rtmsg, len %d\n", len);
			printf("family: %hhu\n", rt->rtm_family);
			printf("rtm_dst_len %hhu\n",rt->rtm_dst_len);
			printf("rtm_src_len %hhu\n",rt->rtm_src_len);
			printf("rtm_type: %hhu\n", rt->rtm_type);
			printf("rtm_flags: %u\n",rt->rtm_flags);
			printf("rtm_protocol: %hhu\n",rt->rtm_protocol);
#endif
			len -= NLMSG_LENGTH(sizeof(*rt));
			if (len < 0) {
				eprintf("nlmsg_len incorrect");
				errno = EINVAL;
				free(buf);
				free(routes);
				return -1;
			}
#ifdef PRINTLOGS
			printf("len after sub %lu is %d\n",sizeof(*rt), len);
#endif
			parse_attr(at, RTM_RTA(rt), len);
			if (rt->rtm_family == AF_INET || rt->rtm_family == AF_INET6) {
				get_attr(at, iroutes, rt);
				iroutes++;
				rtind++;
			} else {
#ifdef PRINTLOGS
				printf("Family is %d\n",rt->rtm_family);
#endif
			}
			r = NLMSG_NEXT(r, recvl);
			count++;
		}
		free(buf);
#ifdef PRINTLOGS
		printf("No. of routes: %d\n",rtind);
#endif
	}
	*(struct rt_info **)out = routes;
	return rtind;
}

int get_net_route(void **out)
{
	int err, net_sock;
	net_sock = connect_route();
	if (net_sock == -1)
		return -1;

	err = send_route_req(net_sock);
	if (err == -1) {
		close(net_sock);
		return -1;
	}

	err = handle_route_resp(net_sock, out);
	if (err == -1) {
		close(net_sock);
		return -1;
	}
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
		errno = EINVAL;
		return -1;
	}
	return len;
}
