#ifndef _RESNET_H
#define _RESNET_H

#define NETDEV_FILE "/proc/net/dev"

#define NET_ALLIFSTAT_SZ    4
#define NETBUF_1024		1024

extern int netdev_fd;

extern int populate_netinfo(struct res_blk *res, int pid, int flags);
extern int getnetinfo(int res_id, void *out, void *hint, int pid, int flags);

#endif /* _RESNET_H */
