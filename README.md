# libresource
library of interfaces through which we can get system resource information
like memory, CPU, stat, networking, device etc.
Currently most of such information is read from /proc and /sys.

## Compile
To compile: 
make all

Compile a test program as follows:
cc -I $LD_LIBRARY_PATH -std=gnu99 -o test  test.c -L $LD_LIBRARY_PATH -lresource

where, $LD_LIBRARY_PATH is the location of your top level libresource code.

To run, set LD_LIBRARY_PATH:
export LD_LIBRARY_PATH=<libresource-directory>
eg. LD_LIBRARY_PATH=/home/user/libresource

Then run your test program:
./test

## Use case:

**1: Ease of use -**
Currently applications and tools need to read this info mostly from
/proc and /sys file-systems. In most of the cases complex string
parsing is involved which is needed to be done in application
code. With the library interfaces application can get the information
directly and all the string parsing, if any, will be done by library.

**2: Stability -**
If the format in which the information is provided in /proc or /sys
file-system is changed then the application code is changed to align
with those changes. Also if a better way to get information comes in
future, like through a syscall or a sysconf then again application code
needs to be changed to get the benefit of it. Library will take care of
such changes.

**3: Virtualization -**
In cases where DB is running in a virtualized environment using
cgroup or namespaces, reading from /proc and /sys file-systems
might not give correct information as these are not cgroup
aware. Library API will take care of this e.g. if a process
is running in a cgroup then library should provide information
which is local to that cgroup.

## Interfaces:

**1: Resource id**

Each resource is identified by a resource id. User land application should
provide the id while calling the interfaces to fetch the information.
Following are resource IDs which are already implemented.

	RES_MEM_ACTIVE
	RES_MEM_INACTIVE
	RES_MEM_AVAILABLE
	RES_MEM_FREE
	RES_MEM_TOTAL
	RES_MEM_PAGESIZE
	RES_MEM_SWAPFREE
	RES_MEM_SWAPTOTAL
	RES_KERN_COMPILE_TIME
	RES_KERN_RELEASE
	RES_NET_ALLIFSTAT
	RES_NET_IFSTAT
	RES_MEM_INFOALL

**2: Read single resource info**

`int res_read(int res_id, void *out, void *hint, int pid, int flags);`

This is to read a resource information. A valid resource id should be provided
in res_id, out should be properly allocated on the basis of size of resource
information, hint should be given where needed. Currently pid and flags are
not used, they are for future extensions.

**3: Read multiple resource info**

If an application wants to read multiple resource information in one call, it
can call res_*_blk APIs to do so.

**3.1 structs**

Following struct holds information about one resource in such case.

    typedef struct res_unit {
        int status;
        int res_id;
        void *hint;
        union r_data data;
    } res_unit_t;

An array of these form res_blk_t structure as below.

    typedef struct res_blk {
        int res_count;
        res_unit_t *res_unit[0];
    } res_blk_t;

res_blk_t strcut is used in all res_*_blk interfaces.

**3.2 functions**

`res_blk_t *res_build_blk(int *res_ids, int res_count);`

It allocates memory for resources and initiates them properly. res_ids
holds an array of valid resource ids and res_count holds number of
resource ids. It also initializes struct fields properly.

`int res_read_blk(res_blk_t *resblk, int pid, int flags);`

Reading bulk resource information. Memory must be properly allocated and
all fields should be properly filled to return error free resource
information. res_build_blk call is suggested to allocate build res_blk_t
structure.

`void res_destroy_blk(res_blk_t *resblk); `

Free allocated memory from res_build_blk.

## Examples

**1: Reading total memory**

    size_t stemp = 0;
    res_read(RES_MEM_TOTAL,&stemp,NULL, 0, 0);
    printf("MEMTOTAL is: %zu\n", stemp);


**2: Reading network interface related statistics for interface named "lo"**

	res_net_ifstat_t ifstat;
	res_read(RES_NET_IFSTAT,&ifstat, (void *)"lo",0, 0);
	printf("status for %s: %llu %llu\n", ifstat.ifname,
		ifstat.rx_bytes,
		ifstat.rx_packets
	);

**3: Reading multiple resource information in one call.**

    res_blk_t *b = NULL;
    int a[NUM] = {RES_MEM_PAGESIZE,
                RES_MEM_TOTAL,
                RES_MEM_AVAILABLE,
                RES_MEM_INFOALL,
                RES_KERN_RELEASE,
                RES_NET_IFSTAT,
                RES_NET_ALLIFSTAT,
                RES_KERN_COMPILE_TIME
                };
    b = res_build_blk(a, NUM);
    b->res_unit[5]->hint = (void *)"lo";

    res_read_blk(b, 0, 0);

    printf("pagesize %ld bytes,\n memtotal %ld kb,\n memavailable %ld kb,\n"
            " memfree %ld kb,\n release %s,\n compile time %s\n",
            b->res_unit[0]->data.sz,
            b->res_unit[1]->data.sz,
            b->res_unit[2]->data.sz,
            ((res_mem_infoall_t *)(b->res_unit[3]->data.ptr))->memfree,
            b->res_unit[4]->data.str,
            b->res_unit[7]->data.str
    );

    res_net_ifstat_t *ip = (res_net_ifstat_t *)b->res_unit[5]->data.ptr;
    printf("stat for interface %s: %llu %llu\n", ip->ifname,
        ip->rx_bytes,
        ip->rx_packets
        );

    int k = (int)(long long)b->res_unit[6]->hint;
    res_net_ifstat_t *ipp = (res_net_ifstat_t *)b->res_unit[6]->data.ptr;
    for (int j=0; j< k; j++) {
        printf("stat for interface %s: %llu %llu\n", ipp[j].ifname,
            ipp[j].rx_bytes,
            ipp[j].rx_packets
        );
    }

    free(ipp);
    res_destroy_blk(b);
