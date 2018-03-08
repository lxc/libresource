#ifndef _RESOURCE_H
#include "resource.h"
#endif
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <ctype.h>
#include <net/if.h>
#include "resnet.h"
#include "resource_impl.h"

int netdev_fd = -1;

static inline int procnetdev_ver(char *buf)
{
	if (strstr(buf, "compressed"))
		return 3;
	if (strstr(buf, "bytes"))
		return 2;
	return 1;
}

static inline char *get_ifname(char *name, char *p)
{
	char *d;
	char *dname;

	while (isspace(*p))
		p++;

	while (*p) {
		if (isspace(*p))
			break;
		if (*p == ':') {
			d = p, dname = name;
			*name++ = *p++;
			while (isdigit(*p))
				*name++ = *p++;
			if (*p != ':') {
				p = d;
				name = dname;
			}
			if (*p == '\0')
				return NULL;
			p++;
			break;
		}
		*name++ = *p++;
	}
	*name++ = '\0';
	return p;
}

static inline int scan_net_stat(char *buf, res_net_ifstat_t *i, int ver)
{
	switch (ver) {
	case 3:
		sscanf(buf,
			"%llu %llu %lu %lu %lu %lu %lu %lu %llu %llu %lu %lu %lu %lu %lu %lu",
			&i->rx_bytes,
			&i->rx_packets,
			&i->rx_errors,
			&i->rx_dropped,
			&i->rx_fifo_err,
			&i->rx_frame_err,
			&i->rx_compressed,
			&i->rx_multicast,

			&i->tx_bytes,
			&i->tx_packets,
			&i->tx_errors,
			&i->tx_dropped,
			&i->tx_fifo_err,
			&i->tx_collisions,
			&i->tx_carrier_err,
			&i->tx_compressed);
		break;
	case 2:
		sscanf(buf,
			"%llu %llu %lu %lu %lu %lu %llu %llu %lu %lu %lu %lu %lu",
			&i->rx_bytes,
			&i->rx_packets,
			&i->rx_errors,
			&i->rx_dropped,
			&i->rx_fifo_err,
			&i->rx_frame_err,

			&i->tx_bytes,
			&i->tx_packets,
			&i->tx_errors,
			&i->tx_dropped,
			&i->tx_fifo_err,
			&i->tx_collisions,
			&i->tx_carrier_err);
		i->rx_multicast = 0;
		break;
	case 1:
		sscanf(buf,
			"%llu %lu %lu %lu %lu %llu %lu %lu %lu %lu %lu",
			&i->rx_packets,
			&i->rx_errors,
			&i->rx_dropped,
			&i->rx_fifo_err,
			&i->rx_frame_err,

			&i->tx_packets,
			&i->tx_errors,
			&i->tx_dropped,
			&i->tx_fifo_err,
			&i->tx_collisions,
			&i->tx_carrier_err);
		i->rx_bytes = 0;
		i->tx_bytes = 0;
		i->rx_multicast = 0;
		break;
	}
	return 0;
}

static inline int getallnetinfo(void *out, void *hint)
{	char buf[NETBUF_1024];
	FILE *fp;
	char ifname[IFNAMSIZ];
	int ver = 0;
	char *p;
	int inum = 0;
	int i;
	res_net_ifstat_t *lst;
	size_t msz = 0;
	int err;

	/* Hint should not be null */
	if (hint == NULL) {
		eprintf(
			"Hint should be provided for RES_NET_ALLIFSTAT");
		errno = EINVAL;
		return -1;
	}

	fp = fopen(NETDEV_FILE, "r");
	if (fp == NULL) {
		err = errno;
		eprintf("while opening File %s with errno: %d",
			NETDEV_FILE, errno);
		errno = err;
		return -1;
	}

	fgets(buf, sizeof(buf), fp);
	fgets(buf, sizeof(buf), fp);
	ver = procnetdev_ver(buf);
	inum = *(int *)hint;

	/* Hint should hold number of interfaces for which memory is
	 * allocated in out. If number of interfaces is 0 then allocated
	 * memory for all interfaces and fill it with interface stat.
	 */
	if (inum == 0) {
		i = 0;

		/* Memory is allocated for NET_ALLIFSTAT_SZ interfaces
		 * initially and then reallocated in chunks of
		 * NET_ALLIFSTAT_SZ till all information is read.
		 */
		msz = sizeof(res_net_ifstat_t) * NET_ALLIFSTAT_SZ;
		lst = (res_net_ifstat_t *)malloc(msz);

		while (fgets(buf, sizeof(buf), fp) != NULL) {
			if (i == NET_ALLIFSTAT_SZ) {
				msz = (sizeof(res_net_ifstat_t) * inum) +
					NET_ALLIFSTAT_SZ;
				lst = (res_net_ifstat_t *)realloc(lst, msz);
				i = 0;
			}
			p = get_ifname(ifname, buf);
			strcpy(lst[inum].ifname, ifname);
			scan_net_stat(p, &lst[inum], ver);
			inum++;
			i++;
		}
		*((res_net_ifstat_t **)out) = lst;
		*(int *)hint = inum;
	} else {
		/* If number of interfaces is not 0 then read interfaces
		 * stat sequentially and put it in memory allocated
		 * by user in out. Memory might be allocated for more interfaces
		 * than there are on system. So read sequentially till either
		 * all interfaces are read or memory allocated is totally
		 * filled.
		 */
		if (out == NULL) {
			errno = EINVAL;
			return -1;
		}

		for (i = 0;
			fgets(buf, sizeof(buf), fp) != NULL && i < inum;
			i++) {
			p = get_ifname(ifname, buf);
			strcpy(((res_net_ifstat_t *)out)[i].ifname, ifname);
			scan_net_stat(p, &(((res_net_ifstat_t *)out)[i]), ver);
		}

		/* Write how many interfaces are read */
		*(int *)hint = i;
	}
	fclose(fp);
	return 0;
}

/* Get resource information related to network */
int getnetinfo(int res_id, void *out, void *hint, int pid, int flags)
{
	char buf[NETBUF_1024];
	FILE *fp;
	int err = 0;
	char ifname[IFNAMSIZ];
	int ver = 0;
	char *p;

	switch (res_id) {
	case RES_NET_IFSTAT:
		/* Interface name should be provided */
		if (hint == NULL) {
			eprintf(
				"Interface name is not provided for RES_NET_IFSTAT");
			errno = EINVAL;
			return -1;
		}

		/* Interface name is provided. Open /proc/net/dev and read
		 * through it.
		 */
		fp = fopen(NETDEV_FILE, "r");
		if (fp == NULL) {
			err = errno;
			eprintf("while opening File %s with errno: %d",
				NETDEV_FILE, errno);
			errno = err;
			return -1;
		}

		/* Skip first line */
		fgets(buf, sizeof(buf), fp);

		fgets(buf, sizeof(buf), fp);
		/* Get what information /proc/net/dev holds so that we can
		 * read it in correct format.
		 */
		ver = procnetdev_ver(buf);

		/* Read each interface till we find the interface which we
		 * are looking for.
		 */
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			/* get interface name in the line */
			p = get_ifname(ifname, buf);

			if ((strncasecmp((char *)hint, ifname,
				sizeof(*(char *)hint))) == 0) {
				/* Interface found, scan information */
				strcpy(((res_net_ifstat_t *)out)->ifname,
					ifname);
				scan_net_stat(p, (res_net_ifstat_t *)out, ver);
				break;
			}
		}

		fclose(fp);
		break;

	/* Information for all interfaces is asked for */
	case RES_NET_ALLIFSTAT:
		return getallnetinfo(out, hint);

	default:
		eprintf("Resource Id is invalid");
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int populate_netinfo(res_blk_t *res, int pid, int flags)
{
	void *p = NULL;

	for (int i = 0; i < res->res_count; i++) {
		switch (res->res_unit[i]->res_id) {
		case RES_NET_IFSTAT:
			if (getnetinfo(RES_NET_IFSTAT,
				res->res_unit[i]->data.ptr,
				res->res_unit[i]->hint, 0, 0) == -1) {
				res->res_unit[i]->status = errno;
			} else
				res->res_unit[i]->status = RES_STATUS_FILLED;
			break;
		case RES_NET_ALLIFSTAT:
			res->res_unit[i]->hint = (int)0;

			if (getnetinfo(RES_NET_ALLIFSTAT, &p,
				&(res->res_unit[i]->hint), 0, 0) == -1) {
				res->res_unit[i]->status = errno;
			} else {
				res->res_unit[i]->status = RES_STATUS_FILLED;
				res->res_unit[i]->data.ptr = p;
			}
			break;
		default:
			res->res_unit[i]->status = RES_STATUS_NOTSUPPORTED;
		}
	}
	return 0;
}
