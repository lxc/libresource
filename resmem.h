#ifndef _RESMEM_H
#define _RESMEM_H

#define MEMINFO_FILE "/proc/meminfo"
extern int meminfo_fd;

#define VMINFO_FILE "/proc/vmstat"

#define MEMBUF_8	8
#define MEMBUF_128	128
#define MEMBUF_2048	2048

extern int populate_meminfo(struct res_blk *res, int pid, int flags);
extern int getmeminfo(int res_id, void *out, void *hint, int pid, int flags);

#endif /* _RESMEM_H */
