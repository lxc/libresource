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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <resource.h>

int main(int argc, char **argv)
{
        struct vmstat data, exist;
	FILE *fp;
	int ret;

	ret = res_read(RES_VMSTAT_INFO, &data, sizeof(data), NULL, 0, 0);
	if (ret != 0) {
		printf("res_read returned error %d\n",ret);
		exit(1);
	}
	ret = res_exist(RES_VMSTAT_INFO, &exist, sizeof(exist), NULL, 0, 0);
	if (ret != 0) {
		printf("res_exist returned error %d\n",ret);
		exit(1);
	}

	fp = fopen ("./vm_info.txt", "w");
	if (fp == NULL) {
		printf("fopen on ./vm_info.txt failed\n");
		exit(1);
	}

	if (exist.nr_free_pages)
        	fprintf(fp, "nr_free_pages %lu\n", data.nr_free_pages);
	if (exist.nr_zone_inactive_anon)
        	fprintf(fp, "nr_zone_inactive_anon %lu\n", data.nr_zone_inactive_anon);
	if (exist.nr_zone_active_anon)
        	fprintf(fp, "nr_zone_active_anon %lu\n", data.nr_zone_active_anon);
	if (exist.nr_zone_inactive_file)
        	fprintf(fp, "nr_zone_inactive_file %lu\n", data.nr_zone_inactive_file);
	if (exist.nr_zone_active_file)
        	fprintf(fp, "nr_zone_active_file %lu\n", data.nr_zone_active_file);
	if (exist.nr_zone_unevictable)
        	fprintf(fp, "nr_zone_unevictable %lu\n", data.nr_zone_unevictable);
	if (exist.nr_zone_write_pending)
        	fprintf(fp, "nr_zone_write_pending %lu\n", data.nr_zone_write_pending);
	if (exist.nr_mlock)
		fprintf(fp, "nr_mlock %lu\n", data.nr_mlock);
	if (exist.nr_bounce)
		fprintf(fp, "nr_bounce %lu\n", data.nr_bounce);
	if (exist.nr_zspages)
		fprintf(fp, "nr_zspages %lu\n", data.nr_zspages);
	if (exist.nr_free_cma)
		fprintf(fp, "nr_free_cma %lu\n", data.nr_free_cma);
	if (exist.numa_hit)
		fprintf(fp, "numa_hit %lu\n", data.numa_hit);
	if (exist.numa_miss)
		fprintf(fp, "numa_miss %lu\n", data.numa_miss);
	if (exist.numa_foreign)
		fprintf(fp, "numa_foreign %lu\n", data.numa_foreign);
	if (exist.numa_interleave)
		fprintf(fp, "numa_interleave %lu\n", data.numa_interleave);
	if (exist.numa_local)
		fprintf(fp, "numa_local %lu\n", data.numa_local);
	if (exist.numa_other)
		fprintf(fp, "numa_other %lu\n", data.numa_other);
	if (exist.nr_inactive_anon)
		fprintf(fp, "nr_inactive_anon %lu\n", data.nr_inactive_anon);
	if (exist.nr_active_anon)
		fprintf(fp, "nr_active_anon %lu\n", data.nr_active_anon);
	if (exist.nr_inactive_file)
		fprintf(fp, "nr_inactive_file %lu\n", data.nr_inactive_file);
	if (exist.nr_active_file)
		fprintf(fp, "nr_active_file %lu\n", data.nr_active_file);
	if (exist.nr_unevictable)
		fprintf(fp, "nr_unevictable %lu\n", data.nr_unevictable);
	if (exist.nr_slab_reclaimable)
		fprintf(fp, "nr_slab_reclaimable %lu\n", data.nr_slab_reclaimable);
	if (exist.nr_slab_unreclaimable)
		fprintf(fp, "nr_slab_unreclaimable %lu\n", data.nr_slab_unreclaimable);
	if (exist.nr_isolated_anon)
		fprintf(fp, "nr_isolated_anon %lu\n", data.nr_isolated_anon);
	if (exist.nr_isolated_file)
		fprintf(fp, "nr_isolated_file %lu\n", data.nr_isolated_file);
	if (exist.workingset_nodes)
		fprintf(fp, "workingset_nodes %lu\n", data.workingset_nodes);
	if (exist.workingset_refault_anon)
		fprintf(fp, "workingset_refault_anon %lu\n", data.workingset_refault_anon);
	if (exist.workingset_refault_file)
		fprintf(fp, "workingset_refault_file %lu\n", data.workingset_refault_file);
	if (exist.workingset_activate_anon)
		fprintf(fp, "workingset_activate_anon %lu\n", data.workingset_activate_anon);
	if (exist.workingset_activate_file)
		fprintf(fp, "workingset_activate_file %lu\n", data.workingset_activate_file);
	if (exist.workingset_restore_anon)
		fprintf(fp, "workingset_restore_anon %lu\n", data.workingset_restore_anon);
	if (exist.workingset_restore_file)
		fprintf(fp, "workingset_restore_file %lu\n", data.workingset_restore_file);
	if (exist.workingset_nodereclaim)
		fprintf(fp, "workingset_nodereclaim %lu\n", data.workingset_nodereclaim);
	if (exist.nr_anon_pages)
		fprintf(fp, "nr_anon_pages %lu\n", data.nr_anon_pages);
	if (exist.nr_mapped)
		fprintf(fp, "nr_mapped %lu\n", data.nr_mapped);
	if (exist.nr_file_pages)
		fprintf(fp, "nr_file_pages %lu\n", data.nr_file_pages);
	if (exist.nr_dirty)
		fprintf(fp, "nr_dirty %lu\n", data.nr_dirty);
	if (exist.nr_writeback)
		fprintf(fp, "nr_writeback %lu\n", data.nr_writeback);
	if (exist.nr_writeback_temp)
		fprintf(fp, "nr_writeback_temp %lu\n", data.nr_writeback_temp);
	if (exist.nr_shmem)
		fprintf(fp, "nr_shmem %lu\n", data.nr_shmem);
	if (exist.nr_shmem_hugepages)
		fprintf(fp, "nr_shmem_hugepages %lu\n", data.nr_shmem_hugepages);
	if (exist.nr_shmem_pmdmapped)
		fprintf(fp, "nr_shmem_pmdmapped %lu\n", data.nr_shmem_pmdmapped);
	if (exist.nr_file_hugepages)
		fprintf(fp, "nr_file_hugepages %lu\n", data.nr_file_hugepages);
	if (exist.nr_file_pmdmapped)
		fprintf(fp, "nr_file_pmdmapped %lu\n", data.nr_file_pmdmapped);
	if (exist.nr_anon_transparent_hugepages)
		fprintf(fp, "nr_anon_transparent_hugepages %lu\n", data.nr_anon_transparent_hugepages);
	if (exist.nr_vmscan_write)
		fprintf(fp, "nr_vmscan_write %lu\n", data.nr_vmscan_write);
	if (exist.nr_vmscan_immediate_reclaim)
		fprintf(fp, "nr_vmscan_immediate_reclaim %lu\n", data.nr_vmscan_immediate_reclaim);
	if (exist.nr_dirtied)
		fprintf(fp, "nr_dirtied %lu\n", data.nr_dirtied);
	if (exist.nr_written)
		fprintf(fp, "nr_written %lu\n", data.nr_written);
	if (exist.nr_throttled_written)
		fprintf(fp, "nr_throttled_written %lu\n", data.nr_throttled_written);
	if (exist.nr_kernel_misc_reclaimable)
		fprintf(fp, "nr_kernel_misc_reclaimable %lu\n", data.nr_kernel_misc_reclaimable);
	if (exist.nr_foll_pin_acquired)
		fprintf(fp, "nr_foll_pin_acquired %lu\n", data.nr_foll_pin_acquired);
	if (exist.nr_foll_pin_released)
		fprintf(fp, "nr_foll_pin_released %lu\n", data.nr_foll_pin_released);
	if (exist.nr_kernel_stack)
		fprintf(fp, "nr_kernel_stack %lu\n", data.nr_kernel_stack);
	if (exist.nr_page_table_pages)
		fprintf(fp, "nr_page_table_pages %lu\n", data.nr_page_table_pages);
	if (exist.nr_sec_page_table_pages)
		fprintf(fp, "nr_sec_page_table_pages %lu\n", data.nr_sec_page_table_pages);
	if (exist.nr_swapcached)
		fprintf(fp, "nr_swapcached %lu\n", data.nr_swapcached);
	if (exist.pgpromote_success)
		fprintf(fp, "pgpromote_success %lu\n", data.pgpromote_success);
	if (exist.pgpromote_candidate)
		fprintf(fp, "pgpromote_candidate %lu\n", data.pgpromote_candidate);
	if (exist.nr_dirty_threshold)
		fprintf(fp, "nr_dirty_threshold %lu\n", data.nr_dirty_threshold);
	if (exist.nr_dirty_background_threshold)
		fprintf(fp, "nr_dirty_background_threshold %lu\n", data.nr_dirty_background_threshold);
	if (exist.pgpgin)
		fprintf(fp, "pgpgin %lu\n", data.pgpgin);
	if (exist.pgpgout)
		fprintf(fp, "pgpgout %lu\n", data.pgpgout);
	if (exist.pswpin)
		fprintf(fp, "pswpin %lu\n", data.pswpin);
	if (exist.pswpout)
		fprintf(fp, "pswpout %lu\n", data.pswpout);
	if (exist.pgalloc_dma)
		fprintf(fp, "pgalloc_dma %lu\n", data.pgalloc_dma);
	if (exist.pgalloc_dma32)
		fprintf(fp, "pgalloc_dma32 %lu\n", data.pgalloc_dma32);
	if (exist.pgalloc_normal)
		fprintf(fp, "pgalloc_normal %lu\n", data.pgalloc_normal);
	if (exist.pgalloc_movable)
		fprintf(fp, "pgalloc_movable %lu\n", data.pgalloc_movable);
	if (exist.pgalloc_device)
		fprintf(fp, "pgalloc_device %lu\n", data.pgalloc_device);
	if (exist.allocstall_dma)
		fprintf(fp, "allocstall_dma %lu\n", data.allocstall_dma);
	if (exist.allocstall_dma32)
		fprintf(fp, "allocstall_dma32 %lu\n", data.allocstall_dma32);
	if (exist.allocstall_normal)
		fprintf(fp, "allocstall_normal %lu\n", data.allocstall_normal);
	if (exist.allocstall_movable)
		fprintf(fp, "allocstall_movable %lu\n", data.allocstall_movable);
	if (exist.allocstall_device)
		fprintf(fp, "allocstall_device %lu\n", data.allocstall_device);
	if (exist.pgskip_dma)
		fprintf(fp, "pgskip_dma %lu\n", data.pgskip_dma);
	if (exist.pgskip_dma32)
		fprintf(fp, "pgskip_dma32 %lu\n", data.pgskip_dma32);
	if (exist.pgskip_normal)
		fprintf(fp, "pgskip_normal %lu\n", data.pgskip_normal);
	if (exist.pgskip_movable)
		fprintf(fp, "pgskip_movable %lu\n", data.pgskip_movable);
	if (exist.pgskip_device)
		fprintf(fp, "pgskip_device %lu\n", data.pgskip_device);
	if (exist.pgfree)
		fprintf(fp, "pgfree %lu\n", data.pgfree);
	if (exist.pgactivate)
		fprintf(fp, "pgactivate %lu\n", data.pgactivate);
	if (exist.pgdeactivate)
		fprintf(fp, "pgdeactivate %lu\n", data.pgdeactivate);
	if (exist.pglazyfree)
		fprintf(fp, "pglazyfree %lu\n", data.pglazyfree);
	if (exist.pgfault)
		fprintf(fp, "pgfault %lu\n", data.pgfault);
	if (exist.pgmajfault)
		fprintf(fp, "pgmajfault %lu\n", data.pgmajfault);
	if (exist.pglazyfreed)
		fprintf(fp, "pglazyfreed %lu\n", data.pglazyfreed);
	if (exist.pgrefill)
		fprintf(fp, "pgrefill %lu\n", data.pgrefill);
	if (exist.pgreuse)
		fprintf(fp, "pgreuse %lu\n", data.pgreuse);
	if (exist.pgsteal_kswapd)
		fprintf(fp, "pgsteal_kswapd %lu\n", data.pgsteal_kswapd);
	if (exist.pgsteal_direct)
		fprintf(fp, "pgsteal_direct %lu\n", data.pgsteal_direct);
	if (exist.pgsteal_khugepaged)
		fprintf(fp, "pgsteal_khugepaged %lu\n", data.pgsteal_khugepaged);
	if (exist.pgdemote_kswapd)
		fprintf(fp, "pgdemote_kswapd %lu\n", data.pgdemote_kswapd);
	if (exist.pgdemote_direct)
		fprintf(fp, "pgdemote_direct %lu\n", data.pgdemote_direct);
	if (exist.pgdemote_khugepaged)
		fprintf(fp, "pgdemote_khugepaged %lu\n", data.pgdemote_khugepaged);
	if (exist.pgscan_kswapd)
		fprintf(fp, "pgscan_kswapd %lu\n", data.pgscan_kswapd);
	if (exist.pgscan_direct)
		fprintf(fp, "pgscan_direct %lu\n", data.pgscan_direct);
	if (exist.pgscan_khugepaged)
		fprintf(fp, "pgscan_khugepaged %lu\n", data.pgscan_khugepaged);
	if (exist.pgscan_direct_throttle)
		fprintf(fp, "pgscan_direct_throttle %lu\n", data.pgscan_direct_throttle);
	if (exist.pgscan_anon)
		fprintf(fp, "pgscan_anon %lu\n", data.pgscan_anon);
	if (exist.pgscan_file)
		fprintf(fp, "pgscan_file %lu\n", data.pgscan_file);
	if (exist.pgsteal_anon)
		fprintf(fp, "pgsteal_anon %lu\n", data.pgsteal_anon);
	if (exist.pgsteal_file)
		fprintf(fp, "pgsteal_file %lu\n", data.pgsteal_file);
	if (exist.zone_reclaim_failed)
		fprintf(fp, "zone_reclaim_failed %lu\n", data.zone_reclaim_failed);
	if (exist.pginodesteal)
		fprintf(fp, "pginodesteal %lu\n", data.pginodesteal);
	if (exist.slabs_scanned)
		fprintf(fp, "slabs_scanned %lu\n", data.slabs_scanned);
	if (exist.kswapd_inodesteal)
		fprintf(fp, "kswapd_inodesteal %lu\n", data.kswapd_inodesteal);
	if (exist.kswapd_low_wmark_hit_quickly)
		fprintf(fp, "kswapd_low_wmark_hit_quickly %lu\n", data.kswapd_low_wmark_hit_quickly);
	if (exist.kswapd_high_wmark_hit_quickly)
		fprintf(fp, "kswapd_high_wmark_hit_quickly %lu\n", data.kswapd_high_wmark_hit_quickly);
	if (exist.pageoutrun)
		fprintf(fp, "pageoutrun %lu\n", data.pageoutrun);
	if (exist.pgrotated)
		fprintf(fp, "pgrotated %lu\n", data.pgrotated);
	if (exist.drop_pagecache)
		fprintf(fp, "drop_pagecache %lu\n", data.drop_pagecache);
	if (exist.drop_slab)
		fprintf(fp, "drop_slab %lu\n", data.drop_slab);
	if (exist.oom_kill)
		fprintf(fp, "oom_kill %lu\n", data.oom_kill);
	if (exist.numa_pte_updates)
		fprintf(fp, "numa_pte_updates %lu\n", data.numa_pte_updates);
	if (exist.numa_huge_pte_updates)
		fprintf(fp, "numa_huge_pte_updates %lu\n", data.numa_huge_pte_updates);
	if (exist.numa_hint_faults)
		fprintf(fp, "numa_hint_faults %lu\n", data.numa_hint_faults);
	if (exist.numa_hint_faults_local)
		fprintf(fp, "numa_hint_faults_local %lu\n", data.numa_hint_faults_local);
	if (exist.numa_pages_migrated)
		fprintf(fp, "numa_pages_migrated %lu\n", data.numa_pages_migrated);
	if (exist.pgmigrate_success)
		fprintf(fp, "pgmigrate_success %lu\n", data.pgmigrate_success);
	if (exist.pgmigrate_fail)
		fprintf(fp, "pgmigrate_fail %lu\n", data.pgmigrate_fail);
	if (exist.thp_migration_success)
		fprintf(fp, "thp_migration_success %lu\n", data.thp_migration_success);
	if (exist.thp_migration_fail)
		fprintf(fp, "thp_migration_fail %lu\n", data.thp_migration_fail);
	if (exist.thp_migration_split)
		fprintf(fp, "thp_migration_split %lu\n", data.thp_migration_split);
	if (exist.compact_migrate_scanned)
		fprintf(fp, "compact_migrate_scanned %lu\n", data.compact_migrate_scanned);
	if (exist.compact_free_scanned)
		fprintf(fp, "compact_free_scanned %lu\n", data.compact_free_scanned);
	if (exist.compact_isolated)
		fprintf(fp, "compact_isolated %lu\n", data.compact_isolated);
	if (exist.compact_stall)
		fprintf(fp, "compact_stall %lu\n", data.compact_stall);
	if (exist.compact_fail)
		fprintf(fp, "compact_fail %lu\n", data.compact_fail);
	if (exist.compact_success)
		fprintf(fp, "compact_success %lu\n", data.compact_success);
	if (exist.compact_daemon_wake)
		fprintf(fp, "compact_daemon_wake %lu\n", data.compact_daemon_wake);
	if (exist.compact_daemon_migrate_scanned)
		fprintf(fp, "compact_daemon_migrate_scanned %lu\n", data.compact_daemon_migrate_scanned);
	if (exist.compact_daemon_free_scanned)
		fprintf(fp, "compact_daemon_free_scanned %lu\n", data.compact_daemon_free_scanned);
	if (exist.htlb_buddy_alloc_success)
		fprintf(fp, "htlb_buddy_alloc_success %lu\n", data.htlb_buddy_alloc_success);
	if (exist.htlb_buddy_alloc_fail)
		fprintf(fp, "htlb_buddy_alloc_fail %lu\n", data.htlb_buddy_alloc_fail);
	if (exist.cma_alloc_success)
		fprintf(fp, "cma_alloc_success %lu\n", data.cma_alloc_success);
	if (exist.cma_alloc_fail)
		fprintf(fp, "cma_alloc_fail %lu\n", data.cma_alloc_fail);
	if (exist.unevictable_pgs_culled)
		fprintf(fp, "unevictable_pgs_culled %lu\n", data.unevictable_pgs_culled);
	if (exist.unevictable_pgs_scanned)
		fprintf(fp, "unevictable_pgs_scanned %lu\n", data.unevictable_pgs_scanned);
	if (exist.unevictable_pgs_rescued)
		fprintf(fp, "unevictable_pgs_rescued %lu\n", data.unevictable_pgs_rescued);
	if (exist.unevictable_pgs_mlocked)
		fprintf(fp, "unevictable_pgs_mlocked %lu\n", data.unevictable_pgs_mlocked);
	if (exist.unevictable_pgs_munlocked)
		fprintf(fp, "unevictable_pgs_munlocked %lu\n", data.unevictable_pgs_munlocked);
	if (exist.unevictable_pgs_cleared)
		fprintf(fp, "unevictable_pgs_cleared %lu\n", data.unevictable_pgs_cleared);
	if (exist.unevictable_pgs_stranded)
		fprintf(fp, "unevictable_pgs_stranded %lu\n", data.unevictable_pgs_stranded);
	if (exist.thp_fault_alloc)
		fprintf(fp, "thp_fault_alloc %lu\n", data.thp_fault_alloc);
	if (exist.thp_fault_fallback)
		fprintf(fp, "thp_fault_fallback %lu\n", data.thp_fault_fallback);
	if (exist.thp_fault_fallback_charge)
		fprintf(fp, "thp_fault_fallback_charge %lu\n", data.thp_fault_fallback_charge);
	if (exist.thp_collapse_alloc)
		fprintf(fp, "thp_collapse_alloc %lu\n", data.thp_collapse_alloc);
	if (exist.thp_collapse_alloc_failed)
		fprintf(fp, "thp_collapse_alloc_failed %lu\n", data.thp_collapse_alloc_failed);
	if (exist.thp_file_alloc)
		fprintf(fp, "thp_file_alloc %lu\n", data.thp_file_alloc);
	if (exist.thp_file_fallback)
		fprintf(fp, "thp_file_fallback %lu\n", data.thp_file_fallback);
	if (exist.thp_file_fallback_charge)
		fprintf(fp, "thp_file_fallback_charge %lu\n", data.thp_file_fallback_charge);
	if (exist.thp_file_mapped)
		fprintf(fp, "thp_file_mapped %lu\n", data.thp_file_mapped);
	if (exist.thp_split_page)
		fprintf(fp, "thp_split_page %lu\n", data.thp_split_page);
	if (exist.thp_split_page_failed)
		fprintf(fp, "thp_split_page_failed %lu\n", data.thp_split_page_failed);
	if (exist.thp_deferred_split_page)
		fprintf(fp, "thp_deferred_split_page %lu\n", data.thp_deferred_split_page);
	if (exist.thp_split_pmd)
		fprintf(fp, "thp_split_pmd %lu\n", data.thp_split_pmd);
	if (exist.thp_scan_exceed_none_pte)
		fprintf(fp, "thp_scan_exceed_none_pte %lu\n", data.thp_scan_exceed_none_pte);
	if (exist.thp_scan_exceed_swap_pte)
		fprintf(fp, "thp_scan_exceed_swap_pte %lu\n", data.thp_scan_exceed_swap_pte);
	if (exist.thp_scan_exceed_share_pte)
		fprintf(fp, "thp_scan_exceed_share_pte %lu\n", data.thp_scan_exceed_share_pte);
	if (exist.thp_split_pud)
		fprintf(fp, "thp_split_pud %lu\n", data.thp_split_pud);
	if (exist.thp_zero_page_alloc)
		fprintf(fp, "thp_zero_page_alloc %lu\n", data.thp_zero_page_alloc);
	if (exist.thp_zero_page_alloc_failed)
		fprintf(fp, "thp_zero_page_alloc_failed %lu\n", data.thp_zero_page_alloc_failed);
	if (exist.thp_swpout)
		fprintf(fp, "thp_swpout %lu\n", data.thp_swpout);
	if (exist.thp_swpout_fallback)
		fprintf(fp, "thp_swpout_fallback %lu\n", data.thp_swpout_fallback);
	if (exist.balloon_inflate)
		fprintf(fp, "balloon_inflate %lu\n", data.balloon_inflate);
	if (exist.balloon_deflate)
		fprintf(fp, "balloon_deflate %lu\n", data.balloon_deflate);
	if (exist.balloon_migrate)
		fprintf(fp, "balloon_migrate %lu\n", data.balloon_migrate);
	if (exist.swap_ra)
		fprintf(fp, "swap_ra %lu\n", data.swap_ra);
	if (exist.swap_ra_hit)
		fprintf(fp, "swap_ra_hit %lu\n", data.swap_ra_hit);
	if (exist.ksm_swpin_copy)
		fprintf(fp, "ksm_swpin_copy %lu\n", data.ksm_swpin_copy);
	if (exist.cow_ksm)
		fprintf(fp, "cow_ksm %lu\n", data.cow_ksm);
	if (exist.zswpin)
		fprintf(fp, "zswpin %lu\n", data.zswpin);
	if (exist.zswpout)
		fprintf(fp, "zswpout %lu\n", data.zswpout);
	if (exist.direct_map_level2_splits)
		fprintf(fp, "direct_map_level2_splits %lu\n", data.direct_map_level2_splits);
	if (exist.direct_map_level3_splits)
		fprintf(fp, "direct_map_level3_splits %lu\n", data.direct_map_level3_splits);
	if (exist.nr_unstable)
		fprintf(fp, "nr_unstable %lu\n", data.nr_unstable);
	fclose(fp);
}
