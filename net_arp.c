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

static int connect_arp()
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
	nl_saddr.nl_groups = RTMGRP_NEIGH;
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
		errno = err;
		return -1;
	}
	return 0;
}

static int parse_attr(struct rtattr *at[], struct rtattr *rta, int len)
{
	memset(at, 0, sizeof(struct rtattr *) * (NDA_MAX + 1));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= NDA_MAX)
			at[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	return 0;
}

static int get_arp_len(int net_sock)
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
#ifdef PRINTLOGS1
	printf("received len %d\n",recvl);
#endif
	return recvl;
}

static int get_max_arp(int len)
{
	int header_size, max_arp;
	header_size = (sizeof(struct nlmsghdr)) + (sizeof(struct ndmsg));
	max_arp = len/header_size;
	return max_arp;
}

#ifdef TESTING
static int get_attr(struct rtattr *at[], struct arp_info *arp, struct ndmsg *n,
		FILE *fp)
#else
static int get_attr(struct rtattr *at[], struct arp_info *arp, struct ndmsg *n)
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

static int handle_arp_resp(int net_sock, void **out)
{
	int err, recvl, len, max_arp, arp_size, ret = 0;
	int aind = 0, total_arp_size = 0;
	struct iovec iov;
	struct msghdr msg;
	struct nlmsghdr *r;
	struct nlmsgerr *nlmsg_err;
	struct ndmsg *nd_msg;
	struct rtattr *at[NDA_MAX + 1], *rta;
	struct arp_info *arps = NULL, *iarps = NULL;
	struct sockaddr_nl nladdr;
#ifdef TESTING
	FILE *fp = NULL;
	fp = fopen ("./arp_info.txt", "w");
	if (!fp)
		return -1;
#endif

	msg.msg_name = &nladdr;
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	while(1) {
		len = get_arp_len(net_sock);
		if (len == -1) {
			if (arps)
				free(arps);
			ret = -1;
			goto out;
		}
		char *buf = (char *)malloc(len);
		if (!buf) {
			if (arps)
				free(arps);
			ret = -1;
			goto out;
		}
		iov.iov_base = buf;
		iov.iov_len = len;
		max_arp = get_max_arp(len);
		arp_size = max_arp * sizeof(struct arp_info);
		total_arp_size += arp_size;

		void *tmp = (struct arp_info *) realloc(arps, total_arp_size);
		if (!tmp) {
			if (arps)
				free(arps);
			free(buf);
			ret = -1;
			goto out;
		}
		arps = tmp;
		iarps = arps + aind;

		recvl = recvmsg(net_sock, &msg, 0);
		if (recvl < 0) {
			err = errno;
			free(arps);
			free(buf);
			eprintf("Error in response recvmsg(), errno %d\n", err);
			errno = err;
			ret = -1;
			goto out;
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
				free(arps);
				errno = nlmsg_err->error;
				ret = -1;
				goto out;
			} else if (r->nlmsg_type == NLMSG_DONE) { 
#ifdef PRINTLOGS
				printf("DONE\n");
#endif
				free(buf);
				*(struct arp_info **)out = arps;
				ret = aind;
				goto out;
			}
			nd_msg = (struct ndmsg *)NLMSG_DATA(r);
			len = r->nlmsg_len;
#ifdef PRINTLOGS
			printf("r->nlmsg_type is %d (RTM_NEWNEIGH = 28)\n", r->nlmsg_type);
			printf("Received ndmsg, len %d\n", len);
			printf("ifindex is %d\n", nd_msg->ndm_ifindex);
			printf("family: %u\n", nd_msg->ndm_family);
			printf("ndm_type: %u\n", nd_msg->ndm_type);
			printf("ndm_flags: %u\n",nd_msg->ndm_flags);
			printf("ndm_state: %u\n", nd_msg->ndm_state);
#endif
			len -= NLMSG_LENGTH(sizeof(*nd_msg));
			if (len < 0) {
				eprintf("nlmsg_len incorrect");
				errno = EINVAL;
				free(buf);
				free(arps);
				ret = -1;
				goto out;
			}
#ifdef PRINTLOGS
			printf("len after sub %ld is %d\n",sizeof(*nd_msg), len);
#endif
			rta = ((struct rtattr *) (((char *) (nd_msg)) + NLMSG_ALIGN(sizeof(struct ndmsg))));
			parse_attr(at, rta, len);
#ifdef TESTING
			get_attr(at, iarps, nd_msg, fp);
#else
			get_attr(at, iarps, nd_msg);
#endif
			iarps++;
			aind++;
			r = NLMSG_NEXT(r, recvl);
			count++;
		}
		free(buf);
		printf("No. of ARPs: %d\n",aind);
	}
	*(struct arp_info **)out = arps;
	ret = aind;
out:
#ifdef TESTING
	if (fp)
		fclose(fp);
#endif
	return ret;
}

int get_net_arp(void **out)
{
	int err, net_sock;

	net_sock = connect_arp();
	if (net_sock == -1)
		return -1;

	err = send_arp_req(net_sock);
	if (err == -1) {
		close(net_sock);
		return -1;
	}

	err = handle_arp_resp(net_sock, out);
	if (err == -1) {
		close(net_sock);
		return -1;
	}
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
		errno = EINVAL;
		return -1;
	}
	return len;
}
