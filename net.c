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
#include <stdbool.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include "resource_impl.h"
#include "net.h"

static int count, ecount;

int connect_netlink(int groups)
{
	struct sockaddr_nl nl_saddr;
	int err, net_sock;

	net_sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (net_sock == -1) {
		err = errno;
		eprintf("Error in socket(), errno %d\n", err);
		return -err;
	}
	bzero(&nl_saddr, sizeof(nl_saddr));
	nl_saddr.nl_family = AF_NETLINK;
	if (groups)
		nl_saddr.nl_groups = RTMGRP_NEIGH;

	err = bind(net_sock, (struct sockaddr *)&nl_saddr, sizeof(nl_saddr));
	if (err == -1) {
		err = errno;
		eprintf("Error in bind(), errno %d\n", err);
		close(net_sock);
		return -err;
	}
	return net_sock;
}

int get_net_len(int net_sock)
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
		return -err;
	}
#ifdef PRINTLOGS
	printf("received len %d\n",recvl);
#endif
	return recvl;
}

static inline int get_max_net(int net_type, int len)
{
	int header_size, net_sz;

	switch(net_type) {
	case NET_ROUTE:
		net_sz = sizeof(struct rtmsg);
		break;
	case NET_ARP:
		net_sz = sizeof(struct ndmsg);
		break;
	case NET_IF:
		net_sz = (sizeof(struct if_stats_msg)) +
			 (sizeof(struct rtnl_link_stats64));
		break;
	}
	header_size = (sizeof(struct nlmsghdr)) + net_sz;
	return (len/header_size);
}

static inline int sz_net(int net_type)
{
	switch(net_type) {
	case NET_ROUTE:
		return (sizeof(struct rt_info));
	case NET_ARP:
		return (sizeof(struct arp_info));
	case NET_IF:
		return (sizeof(struct ifstats));
	}
	return 0;
}

char net_file[3][20] = {"./route_info.txt",
			"./arp_info.txt",
			"./if_info.txt"};

static inline bool check_family_type(unsigned int family, unsigned int type)
{
	if (family == AF_INET || family == AF_INET6) {
		if ((type == RTN_BROADCAST) ||
		    (type == RTN_MULTICAST) ||
		    (type == RTN_LOCAL)) {
			return true;
		}
		return false;
	} else {
		return true;
	}
}

int handle_net_resp(int net_type, int net_sock, void **out)
{
	int recvl, len, max_net, net_size;
	int netind = 0, total_net_size = 0, ret = 0;
	struct iovec iov;
	struct msghdr msg;
	struct nlmsghdr *r;
	struct nlmsgerr *nlmsg_err;
	struct sockaddr_nl nladdr;
	char *buf;
	void *ntwrks = NULL;
	struct rtmsg *rt;
	struct rtattr *at[RTA_MAX + 1];
	struct rt_info *iroutes = NULL;
	struct ndmsg *nd_msg;
	struct rtattr *at_arp[NDA_MAX + 1], *rta;
	struct arp_info *iarps = NULL;
	struct if_stats_msg *ifsm;
	struct rtattr *at_if[IFLA_STATS_MAX+1], *rta_if;
	struct ifstats *ifs = NULL;

#ifdef TESTING
	FILE *fp;
	fp = fopen(net_file[net_type], "w");
	if (!fp) {
		eprintf("Error opening file %s with errno: %d",
				net_file[net_type], errno);
		return -1;
	}
#endif

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &nladdr;
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	while(1) {
		len = get_net_len(net_sock);
		if (len < 0) {
			ret = len;
			goto err1;
		}
		buf = (char *)malloc(len);
		if (!buf) {
			ret = -ENOMEM;
			goto err1;
		}
		memset(&iov, 0, sizeof(iov));
		iov.iov_base = buf;
		iov.iov_len = len;
		max_net = get_max_net(net_type, len);
		net_size = max_net * sz_net(net_type);
		total_net_size += net_size;

		void *tmp = realloc(ntwrks, total_net_size);
		if (!tmp) {
			ret = -ENOMEM;
			goto err2;
		}
		ntwrks = tmp;

		switch(net_type) {
		case NET_ROUTE:
			iroutes = (struct rt_info *)ntwrks + netind;
			break;
		case NET_ARP:
			iarps = (struct arp_info *)ntwrks + netind;
			break;
		case NET_IF:
			ifs = (struct ifstats *)ntwrks + netind;
			break;
		}

		recvl = recvmsg(net_sock, &msg, 0);
		if (recvl < 0) {
			ret = -errno;
			eprintf("Error in resp recvmsg(), errno %d\n", -ret);
			goto err2;
		}
		r = (struct nlmsghdr *) buf;
#ifdef PRINTLOGS
		printf("recvmsg len %d\n", recvl);
		printf("r->nlmsg_type is %d\n", r->nlmsg_type);
#endif
		while (NLMSG_OK(r, recvl)) {
			if (r->nlmsg_type == NLMSG_ERROR) {
				nlmsg_err = (struct nlmsgerr *)NLMSG_DATA(r);
				eprintf("NLMSG_ERROR error: %d\n",
						nlmsg_err->error);
				ecount++;
				ret = -(nlmsg_err->error);
				goto err2;
			} else if (r->nlmsg_type == NLMSG_DONE) {
#ifdef PRINTLOGS
				printf("DONE\n");
#endif
				free(buf);
				switch(net_type) {
				case NET_ROUTE:
					*(struct rt_info **)out =
						(struct rt_info *)ntwrks;
					break;
				case NET_ARP:
					*(struct arp_info **)out =
						(struct arp_info *)ntwrks;
					break;
				case NET_IF:
					*(struct ifstats **)out =
						(struct ifstats *)ntwrks;
					break;
				}
				ret = netind;
				goto out;
			}
			len = r->nlmsg_len;
			switch (net_type) {
			case NET_ROUTE:
				if (r->nlmsg_type != RTM_NEWROUTE) {
					printf("r->nlmsg_type incorrect %u\n",
							r->nlmsg_type);
					ret = -EINVAL;
					goto err2;
				}
				rt = (struct rtmsg *)NLMSG_DATA(r);
				len -= NLMSG_LENGTH(sizeof(*rt));
				if (len < 0) {
					eprintf("nlmsg_len incorrect");
					ret = -EINVAL;
					goto err2;
				}
#ifdef PRINTLOGS
				print_rt(rt);
				printf("len after sub %lu is %d\n",
						sizeof(*rt), len);
#endif
				parse_rt_attr(at, RTM_RTA(rt), len);
				if (check_family_type(rt->rtm_family,
							rt->rtm_type)) {
					r = NLMSG_NEXT(r, recvl);
					continue;
				}
				get_rt_attr(at, iroutes, rt);
#ifdef TESTING
				print_rt_info(iroutes, fp);
#endif
				iroutes++;
				netind++;
				break;
			case NET_ARP:
				if (r->nlmsg_type != RTM_NEWNEIGH) {
					printf("r->nlmsg_type incorrect %u\n",
							r->nlmsg_type);
					ret = -EINVAL;
					goto err2;
				}
				nd_msg = (struct ndmsg *)NLMSG_DATA(r);
				len -= NLMSG_LENGTH(sizeof(*nd_msg));
				if (len < 0) {
					eprintf("nlmsg_len incorrect");
					ret = -EINVAL;
					goto err2;
				}
#ifdef PRINTLOGS
				print_arp(nd_msg);
				printf("len after sub %lu is %d\n",
						sizeof(*nd_msg), len);
#endif
				rta = ((struct rtattr *) (((char *) (nd_msg)) + NLMSG_ALIGN(sizeof(struct ndmsg))));
				parse_arp_attr(at_arp, rta, len);
				if (check_family_type(nd_msg->ndm_family,
							nd_msg->ndm_type)) {
					r = NLMSG_NEXT(r, recvl);
					continue;
				}
#ifdef TESTING
				get_arp_attr(at_arp, iarps, nd_msg, fp);
#else
				get_arp_attr(at_arp, iarps, nd_msg);
#endif
				iarps++;
				netind++;
				break;
			case NET_IF:
				if (r->nlmsg_type != RTM_NEWSTATS) {
					printf("r->nlmsg_type incorrect %u\n",
							r->nlmsg_type);
					ret = -EINVAL;
					goto err2;
				}
				ifsm = (struct if_stats_msg *)NLMSG_DATA(r);
				len -= NLMSG_LENGTH(sizeof(*ifsm));
				if (len < 0) {
					eprintf("nlmsg_len incorrect");
					ret = -EINVAL;
					goto err2;
				}
#ifdef PRINTLOGS
				printf("ifindex is %d\n", ifsm->ifindex);
				printf("len after sub %lu is %d\n",
						sizeof(*ifsm), len);
#endif
				rta_if = ((struct rtattr *)(((char *)(ifsm)) + NLMSG_ALIGN(sizeof(struct if_stats_msg))));
				parse_if_attr(at_if, rta_if, len);
				bzero(ifs, sizeof(*ifs));
				if (if_indextoname(ifsm->ifindex,
						ifs->ifname) == NULL) {
					printf("Error in if_indextoname %d\n",
							errno);
					ret = -EINVAL;
					goto err2;
				}
				memcpy(&ifs->st64,
					RTA_DATA(at_if[IFLA_STATS_LINK_64]),
					sizeof(*ifs));
#ifdef TESTING 
				//print_if_info(ifs, fp);
#endif
#ifdef PRINTLOGS
				print_if(ifs);
#endif
				ifs++;
				netind++;
				break;
			}
			r = NLMSG_NEXT(r, recvl);
			count++;
		}
		free(buf);
#ifdef PRINTLOGS
		printf("No. of routes/arp/if: %d\n",netind);
#endif
	}
err2:
	free(buf);
err1:
	if (ntwrks)
		free(ntwrks);
out:
#ifdef TESTING
	if (fp)
		fclose(fp);
#endif
	return ret;
}
