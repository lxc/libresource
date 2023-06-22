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

//#define PRINTLOGS

static int ecount, count;

static int connect_dev()
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
	//nl_saddr.nl_pid = getpid();

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
		errno = err;
		return -1;
	}
	return 0;
}

static int parse_attr(struct rtattr *at[], struct rtattr *rta, int len)
{
	memset(at, 0, sizeof(struct rtattr *) * (IFLA_STATS_MAX + 1));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= IFLA_STATS_MAX)
			at[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	return 0;
}

static int get_if_len(int net_sock)
{
	int recvl, err;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl nladdr;

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

static int get_max_ifaces(int len)
{
	int header_size, max_if;

	header_size = (sizeof(struct nlmsghdr)) +
			(sizeof(struct if_stats_msg)) +
			(sizeof(struct rtnl_link_stats64));
#ifdef PRINTLOGS
	printf("header_size %d len %d\n",header_size, len);
#endif
	max_if = len/header_size;
	return max_if;
}

static int handle_stats_resp(int net_sock, void **out)
{
	int err, recvl, len, max_ifs, if_size;
	int ifind = 0, total_if_size = 0;
	struct iovec iov;
	struct msghdr msg;
	struct nlmsghdr *r;
	struct nlmsgerr *nlmsg_err;
	struct if_stats_msg *ifsm;
	struct rtattr *at[IFLA_STATS_MAX+1];
	struct rtattr *rta;
	struct ifstats *ifaces = NULL, *ifs = NULL;
	struct sockaddr_nl nladdr;

	msg.msg_name = &nladdr;
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	while(1) {
		len = get_if_len(net_sock);
		if (len == -1) {
			if (ifaces)
				free(ifaces);
			return -1;
		}
		char *buf = (char *)malloc(len);
		if (!buf) {
			if (ifaces)
				free(ifaces);
			return -1;
		}
		iov.iov_base = buf;
		iov.iov_len = len;
		max_ifs = get_max_ifaces(len);
		if_size = max_ifs * sizeof(struct ifstats);
		total_if_size += if_size;
#ifdef PRINTLOGS
		printf("max_ifs %d, if_size %d total_if_size %d\n",max_ifs, if_size, total_if_size);
#endif
		void *tmp = (struct ifstats *) realloc(ifaces, total_if_size);
		if (!tmp) {
			if (ifaces)
				free(ifaces);
			free(buf);
			return -1;
		}
		ifaces = tmp;
		ifs = ifaces + ifind;

		recvl = recvmsg(net_sock, &msg, 0);
		if (recvl < 0) {
			err = errno;
			free(ifaces);
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
				free(ifaces);
				errno = nlmsg_err->error;
				return -1;
			} else if (r->nlmsg_type == NLMSG_DONE) {
#ifdef PRINTLOGS
				printf("DONE\n");
#endif
				free(buf);
				*(struct ifstats **)out = ifaces;
				return ifind;
			}
			ifsm = NLMSG_DATA(r);
			len = r->nlmsg_len;
#ifdef PRINTLOGS
			printf("r->nlmsg_type is %d\n", r->nlmsg_type);
			printf("Received if_stats_msg, len %d\n", len);
			printf("ifindex is %d\n", ifsm->ifindex);
#endif
			len -= NLMSG_LENGTH(sizeof(*ifsm));
			if (len < 0) {
				eprintf("nlmsg_len incorrect");
				errno = EINVAL;
				free(buf);
				free(ifaces);
				return -1;
			}
#ifdef PRINTLOGS
			printf("len after sub %lu is %d\n",sizeof(*ifsm), len);
#endif
			rta = ((struct rtattr *)(((char *)(ifsm)) + NLMSG_ALIGN(sizeof(struct if_stats_msg))));
			parse_attr(at, rta, len);
			memcpy(&ifs->st64, RTA_DATA(at[IFLA_STATS_LINK_64]), sizeof(*ifs));
			if (if_indextoname(ifsm->ifindex, ifs->ifname) == NULL)
				eprintf("Error in if_indextoname %d\n",errno);
#ifdef PRINTLOGS
			else
				printf("Interface name %s\n",ifs->ifname);
			printf("rx_packets %llu\n",ifs->st64.rx_packets);
			printf("tx_packets %llu\n",ifs->st64.tx_packets);
			printf("rx_bytes %llu\n",ifs->st64.rx_bytes);
			printf("tx_bytes %llu\n",ifs->st64.tx_bytes);
#endif
			ifs++;
			ifind++;
			r = NLMSG_NEXT(r, recvl);
			count++;
		}
		free(buf);
#ifdef PRINTLOGS
		printf("No. of interfaces: %d\n",ifind);
#endif
	}
	*(struct ifstats **)out = ifaces;
	return ifind;
}

int get_net_dev(void **out)
{
	int err, net_sock;
	net_sock = connect_dev();
	if (net_sock == -1)
		return -1;
	err = send_stats_req(net_sock);
	if (err == -1) {
		close(net_sock);
		return -1;
	}
	err = handle_stats_resp(net_sock, out);
	if (err == -1) {
		close(net_sock);
		return -1;
	}
#ifdef PRINTLOGS
	printf("count %d error count %d\n",count, ecount);
#endif

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
		errno = EINVAL;
		return -1;
	}
	return len;	
}
