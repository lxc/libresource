/* Copyright (C) 2018, Oracle and/or its affiliates. All rights reserved
 *
 * This file is part of libresource.
 *
 * libresource is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * libresource is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libresource. If not, see <http://www.gnu.org/licenses/>.
 *
 * This is main header file which declares all resource ids and interfaces.
 *
 */

#ifndef	_RESOURCE_H
#define	_RESOURCE_H

#include <net/if.h>
#include <stdint.h>
#include <linux/if_link.h>

/* libresource version */
#define LIBRESOURCE_API_VERSION (1.0.0)


/* Maximum size of a resource information data type which can be returned
 * without explicitly allocating memory for it. If resource information
 * size is larger than this this a pointer to allocated memory will be
 * returned.
 */
#define RES_UNIT_OUT_SIZE	256

/* Size of user name length */
#define RES_USR_NAME_SZ		32

/* This union is used to return resource information of various types */
union r_data {
	int i;
	size_t sz;
	long l;
	char str[RES_UNIT_OUT_SIZE];
	void *ptr;
};

/* In case of res_read_blk, each resource information will be represented by
 * following structure.
 */
typedef struct res_unit {
	int status;
	unsigned int res_id;
	void *hint;
	union r_data data;
	size_t data_sz;
} res_unit_t;

/* In case of bulk read (res_read_blk), this structure will hold all required
 * information needed to do so.
 */
typedef struct res_blk {
	int res_count;
	res_unit_t *res_unit[0];
} res_blk_t;

/* Resource information is divided in broad categories and each
 * category is assigned a number range for its resource information
 * Memory related			(RES_MEM_*)		1024-
 * Network related			(RES_NET_*)		2048-
 * General kernel related		(RES_KERN_*)		3072-
 * Kernel related or get resource 
 * from kernel directly(skip /proc)     (RES_KERN_*)            3072-
 * This is done to facilitate any future optimization which can be made
 * on the basis of resource information (hashing etc ?)
 */
#define MEM_MIN				1024
#define RES_MEM_HUGEPAGEALL		1025
#define RES_MEM_HUGEPAGESIZE		1026
#define RES_MEM_INACTIVE		1027
#define RES_MEM_INFOALL			1028
#define RES_MEM_AVAILABLE		1029
#define RES_MEM_FREE			1030
#define RES_MEM_TOTAL			1031
#define RES_MEM_PAGESIZE		1032
#define RES_MEM_SWAPFREE		1037
#define RES_MEM_SWAPTOTAL		1038
#define RES_MEM_ACTIVE			1039
#define MEM_MAX				1040

#define ROUTE_MIN                       2000
#define RES_NET_ROUTE_ALL               2001
#define ROUTE_MAX                       2010

#define ARP_MIN				2011
#define RES_NET_ARP_ALL			2012
#define RES_GET_ARP_SIZE		2013
#define ARP_MAX				2020

#define DEV_MIN                         2021
#define RES_NET_DEV_ALL                 2022
#define DEV_MAX                         2030

#define RES_NET_MIN			2048
#define RES_NET_IFSTAT			2049
#define RES_NET_ALLIFSTAT		2050
#define RES_NET_IP_LOCAL_PORT_RANGE     2051
#define RES_NET_TCP_RMEM_MAX            2052
#define RES_NET_TCP_WMEM_MAX            2053
#define RES_NET_RMEM_MAX                2054
#define RES_NET_WMEM_MAX                2055
#define RES_NET_MAX			2051

#define KERN_MIN			3072
#define RES_KERN_COMPILE_TIME		3073
#define RES_KERN_RELEASE		3074
#define KERN_MAX                        3075

#define PROC_MIN			4096
#define RES_PROC_INFOALL		4097
#define PROC_MAX			4098

#define VM_MIN				5000
#define RES_VMSTAT_INFO			5001
#define RES_VMSTAT_PGPGIN		5002
#define RES_VMSTAT_PGPGOUT		5003
#define RES_VMSTAT_SWAPIN		5004
#define RES_VMSTAT_SWAPOUT		5005
#define RES_VMSTAT_PGALLOC		5006
#define RES_VMSTAT_PGREFILL		5007
#define RES_VMSTAT_PGSCAN		5008
#define RES_VMSTAT_PGSTEAL		5009
#define VM_MAX				5010

#define CPU_MIN				6000
#define RES_CPU_INFO			6001
#define RES_CPU_CORECOUNT		6002
#define CPU_MAX 			6010

#define STAT_MIN			7000
#define RES_STAT_INFO			7001
#define STAT_MAX			7002

#define FS_MIN                          8000
#define FS_AIONR                        8001
#define FS_AIOMAXNR                     8002
#define FS_FILENR                       8003
#define FS_FILEMAXNR                    8004
#define FS_MAX                          8010

/* Structure to return RES_MEM_INFOALL resource information */
typedef struct res_mem_infoall {
	size_t memfree;
	size_t memtotal;
	size_t memavailable;
	size_t active;
	size_t inactive;
	size_t swaptotal;
	size_t swapfree;
} res_mem_infoall_t;

#define CPU_STR 100
#define CPU_STR1 900
#define CPU_STR2 400

struct cpuinfo {
	unsigned int processor;
	char vendor_id[CPU_STR];
	char cpu_family[CPU_STR];
	char model[CPU_STR];
	char model_name[CPU_STR];
	char stepping[CPU_STR];
	char microcode[CPU_STR];
	char cpu_mhz[CPU_STR];
	char cache_size[CPU_STR];
	unsigned int physical_id;
	unsigned int siblings;
	unsigned int core_id;
	unsigned int cpu_cores;
	unsigned int apicid;
	unsigned int initial_apicid;
	char fpu[CPU_STR];
	char fpu_exception[CPU_STR];
	unsigned int cpu_id_level;
	char wp[CPU_STR];
	char flags[CPU_STR1];
	char vmx_flags[CPU_STR1];
	char bugs[CPU_STR2];
	float bogomips;
	unsigned int clflush_size;
	unsigned int cache_alignment;
	char address_sizes[CPU_STR];
	char power_mgmt[CPU_STR];
	char tlb_size[CPU_STR];
};

struct memstat {
	unsigned long memtotal;
	unsigned long memfree;
	unsigned long memavailable;
	unsigned long buffers;
	unsigned long cached;
	unsigned long swapcached;
	unsigned long active;
	unsigned long inactive;
	unsigned long active_anon;
	unsigned long inactive_anon;
	unsigned long active_file;
	unsigned long inactive_file;
	unsigned long unevictable;
	unsigned long mlocked;
	unsigned long swaptotal;
	unsigned long swapfree;
	unsigned long zswap;
	unsigned long zswapped;
	unsigned long dirty;
	unsigned long writeback;
	unsigned long anonpages;
	unsigned long mapped;
	unsigned long shmem;
	unsigned long kreclaimable;
	unsigned long slab;
	unsigned long sreclaimable;
	unsigned long sunreclaim;
	unsigned long kernelstack;
	unsigned long pagetables;
	unsigned long secpagetables;
	unsigned long nfs_unstable;
	unsigned long bounce;
	unsigned long writebacktmp;
	unsigned long commitlimit;
	unsigned long committed_as;
	unsigned long vmalloc_total;
	unsigned long vmalloc_used;
	unsigned long vmalloc_chunk;
	unsigned long percpu;
	unsigned long hardware_corrupted;
	unsigned long anon_hugepages;
	unsigned long shmem_hugepages;
	unsigned long shmem_pmd_mapped;
	unsigned long file_hugepages;
	unsigned long file_pmd_mapped;
	unsigned long cma_total;
	unsigned long cma_free;
	unsigned long hugepages_total;
	unsigned long hugepages_free;
	unsigned long hugepages_rsvd;
	unsigned long hugepages_surp;
	unsigned long hugepages_size;
	unsigned long hugetlb;
	unsigned long directmap_4k;
	unsigned long directmap_2M;
	unsigned long directmap_1G;
};

#define MAX_BYTES_IPV6  16
#define MAX_BYTES_IPV4   4

struct arp_info {
	unsigned long arp_ip_addr_len;
	char arp_ip_addr[MAX_BYTES_IPV6];
	unsigned long arp_phys_addr_len;
	char arp_phys_addr[6];
};

struct rt_info {
	int table;
	int family;
	int index;
	int protocol;
	int scope;
	char dest[MAX_BYTES_IPV6];
	int dst_prefix_len;
	char src[MAX_BYTES_IPV6];
	int src_prefix_len;
	char prefsrc[MAX_BYTES_IPV6];
	char gate[MAX_BYTES_IPV6];
	int metric;
};

struct ifstats {
        char ifname[IFNAMSIZ];
        struct rtnl_link_stats64 st64;
};

/* Structure to return RES_MEM_ALLIFSTAT resource information */
typedef struct res_net_ifstat {
	char ifname[IFNAMSIZ];
	uint64_t rx_bytes;
	uint64_t rx_packets;
	uint64_t rx_errors;
	uint64_t rx_dropped;
	uint64_t rx_fifo_err;
	uint64_t rx_frame_err;
	uint64_t rx_compressed;
	uint64_t rx_multicast;
	uint64_t tx_bytes;
	uint64_t tx_packets;
	uint64_t tx_errors;
	uint64_t tx_dropped;
	uint64_t tx_fifo_err;
	uint64_t tx_collisions;
	uint64_t tx_carrier_err;
	uint64_t tx_compressed;
} res_net_ifstat_t;

typedef struct proc_info {
	int pid;
	char cmd[16];
	char state;
	int ppid;
	int pgrp;
	int session;
	int tty;
	int tpgid;
	uint64_t flags;
	uint64_t min_flt;
	uint64_t cmin_flt;
	uint64_t maj_flt;
	uint64_t cmaj_flt;
	uint64_t utime;
	uint64_t stime;
	uint64_t cutime;
	uint64_t cstime;
	uint64_t start_time;
	long priority;
	long nice;
	int nlwp;
	long alarm;
	uint64_t vsize;
	long rss;
	uint64_t rss_rlim;
	uint64_t start_code;
	uint64_t end_code;
	uint64_t start_stack;
	uint64_t kstk_esp;
	uint64_t kstk_eip;
	uint64_t wchan;
	int exit_signal;
	int processor;
	uint64_t rtprio;
	uint64_t policy;
	int euid;
	char euser[RES_USR_NAME_SZ];

	/* Information from /prco/[pid]/statm */
	uint64_t size;
	uint64_t resident;
	uint64_t share;
	uint64_t text;
	uint64_t lib;
	uint64_t data;
	uint64_t dt;
} res_proc_infoall_t;

struct vmstat {
	unsigned long nr_free_pages;
	unsigned long nr_zone_inactive_anon;
	unsigned long nr_zone_active_anon;
	unsigned long nr_zone_inactive_file;
	unsigned long nr_zone_active_file;
	unsigned long nr_zone_unevictable;
	unsigned long nr_zone_write_pending;
	unsigned long nr_mlock;
	unsigned long nr_bounce;
	unsigned long nr_zspages;
	unsigned long nr_free_cma;
	unsigned long numa_hit;
	unsigned long numa_miss;
	unsigned long numa_foreign;
	unsigned long numa_interleave;
	unsigned long numa_local;
	unsigned long numa_other;
	unsigned long nr_inactive_anon;
	unsigned long nr_active_anon;
	unsigned long nr_inactive_file;
	unsigned long nr_active_file;
	unsigned long nr_unevictable;
	unsigned long nr_slab_reclaimable;
	unsigned long nr_slab_unreclaimable;
	unsigned long nr_isolated_anon;
	unsigned long nr_isolated_file;
	unsigned long workingset_nodes;
	unsigned long workingset_refault_anon;
	unsigned long workingset_refault_file;
	unsigned long workingset_activate_anon;
	unsigned long workingset_activate_file;
	unsigned long workingset_restore_anon;
	unsigned long workingset_restore_file;
	unsigned long workingset_nodereclaim;
	unsigned long nr_anon_pages;
	unsigned long nr_mapped;
	unsigned long nr_file_pages;
	unsigned long nr_dirty;
	unsigned long nr_writeback;
	unsigned long nr_writeback_temp;
	unsigned long nr_shmem;
	unsigned long nr_shmem_hugepages;
	unsigned long nr_shmem_pmdmapped;
	unsigned long nr_file_hugepages;
	unsigned long nr_file_pmdmapped;
	unsigned long nr_anon_transparent_hugepages;
	unsigned long nr_vmscan_write;
	unsigned long nr_vmscan_immediate_reclaim;
	unsigned long nr_dirtied;
	unsigned long nr_written;
	unsigned long nr_throttled_written;
	unsigned long nr_kernel_misc_reclaimable;
	unsigned long nr_foll_pin_acquired;
	unsigned long nr_foll_pin_released;
	unsigned long nr_kernel_stack;
	unsigned long nr_page_table_pages;
	unsigned long nr_sec_page_table_pages;
	unsigned long nr_swapcached;
	unsigned long pgpromote_success;
	unsigned long pgpromote_candidate;
	unsigned long nr_dirty_threshold;
	unsigned long nr_dirty_background_threshold;
	unsigned long pgpgin;
	unsigned long pgpgout;
	unsigned long pswpin;
	unsigned long pswpout;
	unsigned long pgalloc_dma;
	unsigned long pgalloc_dma32;
	unsigned long pgalloc_normal;
	unsigned long pgalloc_movable;
	unsigned long pgalloc_device;
	unsigned long allocstall_dma;
	unsigned long allocstall_dma32;
	unsigned long allocstall_normal;
	unsigned long allocstall_movable;
	unsigned long allocstall_device;
	unsigned long pgskip_dma;
	unsigned long pgskip_dma32;
	unsigned long pgskip_normal;
	unsigned long pgskip_movable;
	unsigned long pgskip_device;
	unsigned long pgfree;
	unsigned long pgactivate;
	unsigned long pgdeactivate;
	unsigned long pglazyfree;
	unsigned long pgfault;
	unsigned long pgmajfault;
	unsigned long pglazyfreed;
	unsigned long pgrefill;
	unsigned long pgreuse;
	unsigned long pgsteal_kswapd;
	unsigned long pgsteal_direct;
	unsigned long pgsteal_khugepaged;
	unsigned long pgdemote_kswapd;
	unsigned long pgdemote_direct;
	unsigned long pgdemote_khugepaged;
	unsigned long pgscan_kswapd;
	unsigned long pgscan_direct;
	unsigned long pgscan_khugepaged;
	unsigned long pgscan_direct_throttle;
	unsigned long pgscan_anon;
	unsigned long pgscan_file;
	unsigned long pgsteal_anon;
	unsigned long pgsteal_file;
	unsigned long zone_reclaim_failed;
	unsigned long pginodesteal;
	unsigned long slabs_scanned;
	unsigned long kswapd_inodesteal;
	unsigned long kswapd_low_wmark_hit_quickly;
	unsigned long kswapd_high_wmark_hit_quickly;
	unsigned long pageoutrun;
	unsigned long pgrotated;
	unsigned long drop_pagecache;
	unsigned long drop_slab;
	unsigned long oom_kill;
	unsigned long numa_pte_updates;
	unsigned long numa_huge_pte_updates;
	unsigned long numa_hint_faults;
	unsigned long numa_hint_faults_local;
	unsigned long numa_pages_migrated;
	unsigned long pgmigrate_success;
	unsigned long pgmigrate_fail;
	unsigned long thp_migration_success;
	unsigned long thp_migration_fail;
	unsigned long thp_migration_split;
	unsigned long compact_migrate_scanned;
	unsigned long compact_free_scanned;
	unsigned long compact_isolated;
	unsigned long compact_stall;
	unsigned long compact_fail;
	unsigned long compact_success;
	unsigned long compact_daemon_wake;
	unsigned long compact_daemon_migrate_scanned;
	unsigned long compact_daemon_free_scanned;
	unsigned long htlb_buddy_alloc_success;
	unsigned long htlb_buddy_alloc_fail;
	unsigned long cma_alloc_success;
	unsigned long cma_alloc_fail;
	unsigned long unevictable_pgs_culled;
	unsigned long unevictable_pgs_scanned;
	unsigned long unevictable_pgs_rescued;
	unsigned long unevictable_pgs_mlocked;
	unsigned long unevictable_pgs_munlocked;
	unsigned long unevictable_pgs_cleared;
	unsigned long unevictable_pgs_stranded;
	unsigned long thp_fault_alloc;
	unsigned long thp_fault_fallback;
	unsigned long thp_fault_fallback_charge;
	unsigned long thp_collapse_alloc;
	unsigned long thp_collapse_alloc_failed;
	unsigned long thp_file_alloc;
	unsigned long thp_file_fallback;
	unsigned long thp_file_fallback_charge;
	unsigned long thp_file_mapped;
	unsigned long thp_split_page;
	unsigned long thp_split_page_failed;
	unsigned long thp_deferred_split_page;
	unsigned long thp_split_pmd;
	unsigned long thp_scan_exceed_none_pte;
	unsigned long thp_scan_exceed_swap_pte;
	unsigned long thp_scan_exceed_share_pte;
	unsigned long thp_split_pud;
	unsigned long thp_zero_page_alloc;
	unsigned long thp_zero_page_alloc_failed;
	unsigned long thp_swpout;
	unsigned long thp_swpout_fallback;
	unsigned long balloon_inflate;
	unsigned long balloon_deflate;
	unsigned long balloon_migrate;
	unsigned long swap_ra;
	unsigned long swap_ra_hit;
	unsigned long ksm_swpin_copy;
	unsigned long cow_ksm;
	unsigned long zswpin;
	unsigned long zswpout;
	unsigned long direct_map_level2_splits;
	unsigned long direct_map_level3_splits;
	unsigned long nr_unstable;
};

struct cpu_stat {
	unsigned long long user;
	unsigned long long nice;
	unsigned long long system;
	unsigned long long idle;
	unsigned long long iowait;
	unsigned long long irq;
	unsigned long long softirq;
	unsigned long long steal;
	unsigned long long guest;
	unsigned long long guest_nice;
};

struct stat_info {
	struct cpu_stat cpu;
	struct cpu_stat *all_cpu; /* Array of len cpu_num */
	int cpu_num;
	char intr[9000]; // total of all interrupts serviced(since boot)
	unsigned long long ctxt;
	unsigned long long btime;
	unsigned long long processes;
	unsigned long long procs_running;
	unsigned long long procs_blocked;
	char softirq[9000]; /* Number of softirq for all CPUs. */
};

/* Allocating memory and building a res_blk structure to return bulk
 * resource information.
 */
extern res_blk_t *res_build_blk(int *res_ids, int res_count);

/* Reading bulk resource information. Memory must be properly allocated and
 * all fields should be properly filled to return error free resource
 * information. res_build_blk call is suggested to allocate build res_blk_t
 * structure.
 */
extern int res_read_blk(res_blk_t *resblk, int pid, int flags);

/* Free allocated memory from res_build_blk */
extern void res_destroy_blk(res_blk_t *resblk);

/* Read a resource information. Memory for out should be properly allocated.
 * Also size of allocated memory should be passed in out_sz.
 */
extern int res_read(int res_id, void *out, size_t out_sz, void **hint, int pid, int flags);
extern int res_exist(int res_id, void *out, size_t out_sz, void *hint, int pid, int flags);

extern int res_read_kern(int res_id, void *out, size_t out_sz, void* in);

#endif /* RESOURCE_H */
