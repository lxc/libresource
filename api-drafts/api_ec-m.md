I would suggest to first focus on all the files and directories directly under
/proc/ and later discuss classes concerning /proc/pid. Thus, I am proposing the
following architecture for the api:

*Scheme: "classname: important files/directories for the class*"
- **kernelinfo**: cgroups, cmdline, config.gz, driver/, execdomains, kallsyms,
kcore, kmsg, kpagecount, kpagemasks, loadavg, modules, slabinfo, stat(\*),
uptime, version
    -   (*) stat varies with architecture, so we should discuss at a later 
    point how to handle this
- **memory:** buddyinfo, diskstat(\*), dma, iomem(\*), ioports(\*), locks(\*),
meminfo(\**),  mtrr, swaps, vmalloc, vmstat, zoneinfo                  
    - (\*) maybe it would be a good idea to have a (sub-)class called "io"
    that contains information for these 4 files?                       
    - (\**) meminfo already contains a lot of compact information, so my 
    suggestion is to have our main focus on this file in the beginning.
- **devices:** bus/, consoles(\*), devices, fb, mdstats, misc, tty/(\*)
    - (\*) maybe we should have a subclass for these 2 files?
- **sys:** sys/, sysrc-trigger
- **sysvripc:** sysvricp/
- **scsi:** scsci/
- **fs:** fs/, filesystems, mounts(\*), partitions
    - (\*) Or do you think that "mounts" belongs to devices?
- **irq:** irq/, interrups, softirqs
- **crypto:** crypto, keys, key-users
- **net:** net/

We should also discuss if it might be of use to write a **generic parser** for the
files under proc/. We should consider the following:
-   Almost all files are formatted as tables of some sort: Would it be possible to
    write a parser that can read out the information of any table into some generic
    format which we can then use for further processing?

