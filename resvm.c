/* Copyright (C) 2023, Oracle and/or its affiliates. All rights reserved
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
 */

#ifndef _RESOURCE_H
#include "resource.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "resvm.h"
#include "resource_impl.h"
#include <errno.h>
#include <libgen.h>

static char buffer[4096];

struct vm_tb {
	char *vm_name;
	unsigned long *vm_value;
};

static int compare_vm_tb_entry(const void *entry1, const void *entry2)
{
	return strcmp(((struct vm_tb*)entry1)->vm_name,((struct vm_tb*)entry2)->vm_name);
}

int find_vm_elem(char *search_str, void *out)
{
	char *start, *found = NULL, *value;

	start = buffer;
	found = strstr(start, search_str);
	if (found) {
		value = strchr(found, ' ');
		value++;
		*(unsigned long *)out = strtoul(value, NULL, 10);
		//printf("Element %s value %lu\n",search_str, *(unsigned long *)out);
		return 0;
	} else {
		printf("Element %s not found\n",search_str);
		return -1;
	}
	return 0;
}

int populate_vminfo(void *out, int ex)
{
	char *end, *start;
	char search_name[64];
	struct vm_tb search_entry = {search_name, NULL};
	struct vm_tb *result;
	int vm_tb_count;
	struct vmstat vm;

	struct vm_tb vm_tb_array[] = {
		{"nr_free_pages", &vm.nr_free_pages},
		{"nr_zone_inactive_anon", &vm.nr_zone_inactive_anon},
		{"nr_zone_active_anon", &vm.nr_zone_active_anon},
		{"nr_zone_inactive_file", &vm.nr_zone_inactive_file},
		{"nr_zone_active_file", &vm.nr_zone_active_file},
		{"nr_zone_unevictable", &vm.nr_zone_unevictable},
		{"nr_zone_write_pending", &vm.nr_zone_write_pending},
		{"nr_mlock", &vm.nr_mlock},
		{"nr_bounce", &vm.nr_bounce},
		{"nr_zspages", &vm.nr_zspages},
		{"nr_free_cma", &vm.nr_free_cma},
		{"numa_hit", &vm.numa_hit},
		{"numa_miss", &vm.numa_miss},
		{"numa_foreign", &vm.numa_foreign},
		{"numa_interleave", &vm.numa_interleave},
		{"numa_local", &vm.numa_local},
		{"numa_other", &vm.numa_other},
		{"nr_inactive_anon", &vm.nr_inactive_anon},
		{"nr_active_anon", &vm.nr_active_anon},
		{"nr_inactive_file", &vm.nr_inactive_file},
		{"nr_active_file", &vm.nr_active_file},
		{"nr_unevictable", &vm.nr_unevictable},
		{"nr_slab_reclaimable", &vm.nr_slab_reclaimable},
		{"nr_slab_unreclaimable", &vm.nr_slab_unreclaimable},
		{"nr_isolated_anon", &vm.nr_isolated_anon},
		{"nr_isolated_file", &vm.nr_isolated_file},
		{"workingset_nodes", &vm.workingset_nodes},
		{"workingset_refault_anon", &vm.workingset_refault_anon},
		{"workingset_refault_file", &vm.workingset_refault_file},
		{"workingset_activate_anon", &vm.workingset_activate_anon},
		{"workingset_activate_file", &vm.workingset_activate_file},
		{"workingset_restore_anon", &vm.workingset_restore_anon},
		{"workingset_restore_file", &vm.workingset_restore_file},
		{"workingset_nodereclaim", &vm.workingset_nodereclaim},
		{"nr_anon_pages", &vm.nr_anon_pages},
		{"nr_mapped", &vm.nr_mapped},
		{"nr_file_pages", &vm.nr_file_pages},
		{"nr_dirty", &vm.nr_dirty},
		{"nr_writeback", &vm.nr_writeback},
		{"nr_writeback_temp", &vm.nr_writeback_temp},
		{"nr_shmem", &vm.nr_shmem},
		{"nr_shmem_hugepages", &vm.nr_shmem_hugepages},
		{"nr_shmem_pmdmapped", &vm.nr_shmem_pmdmapped},
		{"nr_file_hugepages", &vm.nr_file_hugepages},
		{"nr_file_pmdmapped", &vm.nr_file_pmdmapped},
		{"nr_anon_transparent_hugepages", &vm.nr_anon_transparent_hugepages},
		{"nr_vmscan_write", &vm.nr_vmscan_write},
		{"nr_vmscan_immediate_reclaim", &vm.nr_vmscan_immediate_reclaim},
		{"nr_dirtied", &vm.nr_dirtied},
		{"nr_written", &vm.nr_written},
		{"nr_throttled_written", &vm.nr_throttled_written},
		{"nr_kernel_misc_reclaimable", &vm.nr_kernel_misc_reclaimable},
		{"nr_foll_pin_acquired", &vm.nr_foll_pin_acquired},
		{"nr_foll_pin_released", &vm.nr_foll_pin_released},
		{"nr_kernel_stack", &vm.nr_kernel_stack},
		{"nr_page_table_pages", &vm.nr_page_table_pages},
		{"nr_sec_page_table_pages", &vm.nr_sec_page_table_pages},
		{"nr_swapcached", &vm.nr_swapcached},
		{"pgpromote_success", &vm.pgpromote_success},
		{"pgpromote_candidate", &vm.pgpromote_candidate},
		{"nr_dirty_threshold", &vm.nr_dirty_threshold},
		{"nr_dirty_background_threshold", &vm.nr_dirty_background_threshold},
		{"pgpgin", &vm.pgpgin},
		{"pgpgout", &vm.pgpgout},
		{"pswpin", &vm.pswpin},
		{"pswpout", &vm.pswpout},
		{"pgalloc_dma", &vm.pgalloc_dma},
		{"pgalloc_dma32", &vm.pgalloc_dma32},
		{"pgalloc_normal", &vm.pgalloc_normal},
		{"pgalloc_movable", &vm.pgalloc_movable},
		{"pgalloc_device", &vm.pgalloc_device},
		{"allocstall_dma", &vm.allocstall_dma},
		{"allocstall_dma32", &vm.allocstall_dma32},
		{"allocstall_normal", &vm.allocstall_normal},
		{"allocstall_movable", &vm.allocstall_movable},
		{"allocstall_device", &vm.allocstall_device},
		{"pgskip_dma", &vm.pgskip_dma},
		{"pgskip_dma32", &vm.pgskip_dma32},
		{"pgskip_normal", &vm.pgskip_normal},
		{"pgskip_movable", &vm.pgskip_movable},
		{"pgskip_device", &vm.pgskip_device},
		{"pgfree", &vm.pgfree},
		{"pgactivate", &vm.pgactivate},
		{"pgdeactivate", &vm.pgdeactivate},
		{"pglazyfree", &vm.pglazyfree},
		{"pgfault", &vm.pgfault},
		{"pgmajfault", &vm.pgmajfault},
		{"pglazyfreed", &vm.pglazyfreed},
		{"pgrefill", &vm.pgrefill},
		{"pgreuse", &vm.pgreuse},
		{"pgsteal_kswapd", &vm.pgsteal_kswapd},
		{"pgsteal_direct", &vm.pgsteal_direct},
		{"pgsteal_khugepaged", &vm.pgsteal_khugepaged},
		{"pgdemote_kswapd", &vm.pgdemote_kswapd},
		{"pgdemote_direct", &vm.pgdemote_direct},
		{"pgdemote_khugepaged", &vm.pgdemote_khugepaged},
		{"pgscan_kswapd", &vm.pgscan_kswapd},
		{"pgscan_direct", &vm.pgscan_direct},
		{"pgscan_khugepaged", &vm.pgscan_khugepaged},
		{"pgscan_direct_throttle", &vm.pgscan_direct_throttle}, 
		{"pgscan_anon", &vm.pgscan_anon},
		{"pgscan_file", &vm.pgscan_file},
		{"pgsteal_anon", &vm.pgsteal_anon},
		{"pgsteal_file", &vm.pgsteal_file},
		{"zone_reclaim_failed", &vm.zone_reclaim_failed},
		{"pginodesteal", &vm.pginodesteal},
		{"slabs_scanned", &vm.slabs_scanned},
		{"kswapd_inodesteal", &vm.kswapd_inodesteal},
		{"kswapd_low_wmark_hit_quickly", &vm.kswapd_low_wmark_hit_quickly},
		{"kswapd_high_wmark_hit_quickly", &vm.kswapd_high_wmark_hit_quickly},
		{"pageoutrun", &vm.pageoutrun},
		{"pgrotated", &vm.pgrotated},
		{"drop_pagecache", &vm.drop_pagecache},
		{"drop_slab", &vm.drop_slab},
		{"oom_kill", &vm.oom_kill},
		{"numa_pte_updates", &vm.numa_pte_updates},
		{"numa_huge_pte_updates", &vm.numa_huge_pte_updates},
		{"numa_hint_faults", &vm.numa_hint_faults},
		{"numa_hint_faults_local", &vm.numa_hint_faults_local},
		{"numa_pages_migrated", &vm.numa_pages_migrated},
		{"pgmigrate_success", &vm.pgmigrate_success},
		{"pgmigrate_fail", &vm.pgmigrate_fail},
		{"thp_migration_success", &vm.thp_migration_success},
		{"thp_migration_fail", &vm.thp_migration_fail},
		{"thp_migration_split", &vm.thp_migration_split},
		{"compact_migrate_scanned", &vm.compact_migrate_scanned},
		{"compact_free_scanned", &vm.compact_free_scanned},
		{"compact_isolated", &vm.compact_isolated},
		{"compact_stall", &vm.compact_stall},
		{"compact_fail", &vm.compact_fail},
		{"compact_success", &vm.compact_success},
		{"compact_daemon_wake", &vm.compact_daemon_wake},
		{"compact_daemon_migrate_scanned", &vm.compact_daemon_migrate_scanned},
		{"compact_daemon_free_scanned", &vm.compact_daemon_free_scanned},
		{"htlb_buddy_alloc_success", &vm.htlb_buddy_alloc_success},
		{"htlb_buddy_alloc_fail", &vm.htlb_buddy_alloc_fail},
		{"cma_alloc_success", &vm.cma_alloc_success},
		{"cma_alloc_fail", &vm.cma_alloc_fail},
		{"unevictable_pgs_culled", &vm.unevictable_pgs_culled},
		{"unevictable_pgs_scanned", &vm.unevictable_pgs_scanned},
		{"unevictable_pgs_rescued", &vm.unevictable_pgs_rescued},
		{"unevictable_pgs_mlocked", &vm.unevictable_pgs_mlocked},
		{"unevictable_pgs_munlocked", &vm.unevictable_pgs_munlocked},
		{"unevictable_pgs_cleared", &vm.unevictable_pgs_cleared},
		{"unevictable_pgs_stranded", &vm.unevictable_pgs_stranded},
		{"thp_fault_alloc", &vm.thp_fault_alloc},
		{"thp_fault_fallback", &vm.thp_fault_fallback},
		{"thp_fault_fallback_charge", &vm.thp_fault_fallback_charge},
		{"thp_collapse_alloc", &vm.thp_collapse_alloc},
		{"thp_collapse_alloc_failed", &vm.thp_collapse_alloc_failed},
		{"thp_file_alloc", &vm.thp_file_alloc},
		{"thp_file_fallback", &vm.thp_file_fallback},
		{"thp_file_fallback_charge", &vm.thp_file_fallback_charge},
		{"thp_file_mapped", &vm.thp_file_mapped},
		{"thp_split_page", &vm.thp_split_page},
		{"thp_split_page_failed", &vm.thp_split_page_failed},
		{"thp_deferred_split_page", &vm.thp_deferred_split_page},
		{"thp_split_pmd", &vm.thp_split_pmd},
		{"thp_scan_exceed_none_pte", &vm.thp_scan_exceed_none_pte},
		{"thp_scan_exceed_swap_pte", &vm.thp_scan_exceed_swap_pte},
		{"thp_scan_exceed_share_pte", &vm.thp_scan_exceed_share_pte},
		{"thp_split_pud", &vm.thp_split_pud},
		{"thp_zero_page_alloc", &vm.thp_zero_page_alloc},
		{"thp_zero_page_alloc_failed", &vm.thp_zero_page_alloc_failed},
		{"thp_swpout", &vm.thp_swpout},
		{"thp_swpout_fallback", &vm.thp_swpout_fallback},
		{"balloon_inflate", &vm.balloon_inflate},
		{"balloon_deflate", &vm.balloon_deflate},
		{"balloon_migrate", &vm.balloon_migrate},
		{"swap_ra", &vm.swap_ra},
		{"swap_ra_hit", &vm.swap_ra_hit},
		{"ksm_swpin_copy", &vm.ksm_swpin_copy},
		{"cow_ksm", &vm.cow_ksm},
		{"zswpin", &vm.zswpin},
		{"zswpout", &vm.zswpout},
		{"direct_map_level2_splits", &vm.direct_map_level2_splits},
		{"direct_map_level3_splits", &vm.direct_map_level3_splits},
		{"nr_unstable", &vm.nr_unstable},
	};

	memset(&vm, 0, sizeof(vm));
	vm_tb_count = sizeof(vm_tb_array)/sizeof(struct vm_tb);

	qsort(vm_tb_array, vm_tb_count, sizeof(struct vm_tb),
	      compare_vm_tb_entry);

	start = buffer;
	while(1) {
		end = strchr(start, ' ');
		if (!end)
			break;
		end[0] = '\0';
		strcpy(search_entry.vm_name, start);
		result = bsearch(&search_entry, vm_tb_array, vm_tb_count,
				 sizeof(struct vm_tb),
				 compare_vm_tb_entry);
		if (result) {
			end++;
			if (ex)
				*(result->vm_value) = 1;
			else
				*(result->vm_value) = strtoul(end, NULL, 10);
		} else {
			printf("Entry %s not found\n",search_entry.vm_name);
		}
		start = strchr(end, '\n');
		if (!start)
			break;
		start++;
	}
	memcpy(out, &vm, sizeof(vm));
	return 0;
}

int getvmexist(int res_id, void *exist, size_t sz, void *hint, int flags)
{
	int ret;
	memset(exist, 0, sz);
	ret = file_to_buf("./vm_info.orig", buffer, sizeof(buffer));
	if (ret < 0)
		return ret;
	ret = populate_vminfo(exist, 1);
	return ret;
}

int getvmstatinfo(int res_id, void *out, size_t sz, void **hint, int flags)
{
	int ret;
	struct vmstat out1;
	memset(&out1, 0, sizeof(out1));
	memset(out, 0, sz);

#ifdef TESTING
	ret = file_to_buf("./vm_info.orig", buffer, sizeof(buffer));
#else	
	ret = file_to_buf(VMINFO_FILE, buffer, sizeof(buffer));
#endif
	if (ret < 0)
		return ret;

	switch (res_id) {
	case RES_VMSTAT_INFO:
		CHECK_SIZE(sz, sizeof(struct vmstat));
		ret = populate_vminfo(out, 0);
		break;
	case RES_VMSTAT_PGPGIN:
		CHECK_SIZE(sz, sizeof(unsigned long));
		ret = find_vm_elem("pgpgin", out);
		break;
	case RES_VMSTAT_PGPGOUT:
		CHECK_SIZE(sz, sizeof(unsigned long));
		ret = find_vm_elem("pgpgout", out);
		break;
	case RES_VMSTAT_SWAPIN:
		CHECK_SIZE(sz, sizeof(unsigned long));
		ret = find_vm_elem("pswpin", out);
		break;
	case RES_VMSTAT_SWAPOUT:
		CHECK_SIZE(sz, sizeof(unsigned long));
		ret = find_vm_elem("pswpout", out);
		break;
	case RES_VMSTAT_PGALLOC:
		CHECK_SIZE(sz, sizeof(unsigned long));
		ret = populate_vminfo(&out1, 0);
		*(unsigned long *)out = out1.pgalloc_dma +
					out1.pgalloc_dma32 +
					out1.pgalloc_normal +
					out1.pgalloc_movable;
		break;
	case RES_VMSTAT_PGREFILL:
		CHECK_SIZE(sz, sizeof(unsigned long));
		ret = find_vm_elem("pgrefill", out);
		break;
	case RES_VMSTAT_PGSCAN:
		CHECK_SIZE(sz, sizeof(unsigned long));
		ret = populate_vminfo(&out1, 0);
		*(unsigned long *)out = out1.pgscan_kswapd +
					out1.pgscan_direct +
					out1.pgscan_direct_throttle +
					out1.pgscan_anon + out1.pgscan_file;
		break;
	case RES_VMSTAT_PGSTEAL:
		CHECK_SIZE(sz, sizeof(unsigned long));
		ret = populate_vminfo(&out1, 0);
		*(unsigned long *)out = out1.pgsteal_kswapd +
					out1.pgsteal_direct +
					out1.pgsteal_anon +
					out1.pgsteal_file;
		break;
        default:
                eprintf("Resource Id is invalid");
                errno = EINVAL;
                return -1;

	}
	return ret;
}
