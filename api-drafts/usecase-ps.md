1. Functions needed for **ps**

```
// returns terminal associated with given pid
char *get_tty(pid_t)

// returns amount of cpu time used by process with given pid
// (should we return an int?)
int get_cputime(pid_t)

// returns name of process with given pid
char *get_cmdline(pid_t)
```

```
void ps
{
        print "PID TTY TIME CMD"
        pid = getpid()
        ppid = getppid()
        print pid get_tty(pid) get_cputime(pid) get_cmdline(pid)
        print ppid get_tty(ppid) get_cputime(ppid) get_cmdline(ppid)
}
```

- We probably also need a function that recursively gives us the topmost
  calling process's pid.
- Maybe all these functions should take structs as arguments and return
  structs so that they can be used on a lot of pids?

2. Additional functions needed for **ps a**

```
// returns process status code as string
char *get_stat(pid_t)

// returns all pids that have a terminal associated with it
// function should sort by pid
struct pid *get_tty_all(struct pid)
```

```
void ps a
{
        print "PID TTY STAT TIME COMMAND"
        struct pid pidtty = get_tty_all(&pidtty)
        for (all pid in pidtty) {
                print pid get_tty(pid) get_stat_(pid) get_cputime(pid) get_cmdline(pid)
        }
}
```

3. Additional functions needed for **ps aux**

```
// returns all pids
struct pid *get_all_pid()

// returns name of user who owns process for given pid
char *get_user(pid_t)

// returns amount of cpu used by process with given pid
double get_cpushare(pid_t)

// returns amount of memory used by process with given pid
double get_memshare(pid_t)

// returns virtual memory sized used by process with given pid
int get_vsz(pid_t)

// returns amount of real memory used by process with given pid
int get_rss(pid_t)

// returns time when process of given pid started
// as string separated by colon
char *get_starttime(pid_t)
```

```
void ps aux
{
        print "USER PID %CPU %MEM VSZ RSS TTY STAT START TIME COMMAND"
        struct pid allpids = get_all_pid(&allpids)
        for (all pid in allpids) {
                print get_user(pid) pid get_cpushare(pid) get_memshare(pid)
                        get_vsz(pid) get_rss(pid) get_tty(pid) get_stat(pid)
                        get_starttime(pid) get_cputime(pid) get_cmdline()
        }
}
```
