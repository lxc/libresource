#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
//#define BPF_FILT
#ifdef BPF_FILT
#include <stdbool.h>
#include <linux/filter.h>
#endif

#define RUSAGE_TIME

#ifdef RUSAGE_TIME
#include <sys/resource.h>
#endif
#include <signal.h>

//#define FILTER

#ifdef FILTER 
#define NL_MESSAGE_SIZE (sizeof(struct nlmsghdr) + sizeof(struct cn_msg) + \
                     sizeof(struct proc_input))
#else
#define NL_MESSAGE_SIZE (sizeof(struct nlmsghdr) + sizeof(struct cn_msg) + \
                     sizeof(int))
#endif

#define MAX_EVENTS 1

volatile static int interrupted;
static int exit_count;
static int fork_count;
static int exec_count;
static int tcount;

static int nl_sock, ret_errno;
static struct epoll_event evn;

//#define ENABLE_PRINTS

#ifdef ENABLE_PRINTS
#define Printf printf
#else
#define Printf
#endif

#ifdef BPF_FILT
struct sock_filter BPF_bad_exit[] = {
	{ 0x20,  0,  0, 0x00000024 },
	{ 0x15,  0,  3, 0x00000080 },
	{ 0x20,  0,  0, 0x0000003c },
	{ 0x15,  1,  0, 0000000000 },
	{ 0x06,  0,  0, 0xffffffff },
	{ 0x06,  0,  0, 0000000000 },
};

struct sock_filter BPF_exit[] = {
	{ 0x20,  0,  0, 0x00000024 },
	{ 0x15,  0,  1, 0x00000080 },
	{ 0x06,  0,  0, 0xffffffff },
	{ 0x06,  0,  0, 0000000000 },
};

struct sock_fprog Filter;
#endif

#ifdef RUSAGE_TIME
float time_diff(struct timeval *start, struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) +
		1e-6*(end->tv_usec - start->tv_usec);
}
#endif

#ifdef FILTER
int send_message(struct proc_input *pinp) 
#else
int send_message(enum proc_cn_mcast_op mcast_op)
#endif
{
	char buff[NL_MESSAGE_SIZE];
	struct nlmsghdr *hdr;
	struct cn_msg *msg;

	hdr = (struct nlmsghdr *)buff;
	hdr->nlmsg_len = NL_MESSAGE_SIZE;
	hdr->nlmsg_type = NLMSG_DONE;
	hdr->nlmsg_flags = 0;
	hdr->nlmsg_seq = 0;
	hdr->nlmsg_pid = getpid();

	msg = (struct cn_msg *)NLMSG_DATA(hdr);
	msg->id.idx = CN_IDX_PROC;
	msg->id.val = CN_VAL_PROC;
	msg->seq = 0;
	msg->ack = 0;
	msg->flags = 0;

#ifdef FILTER
	msg->len = sizeof(struct proc_input);
	((struct proc_input*)msg->data)->mcast_op = pinp->mcast_op;
	((struct proc_input*)msg->data)->event_type = pinp->event_type;
#else
	msg->len = sizeof(int);
	*(int*)msg->data = mcast_op;
#endif

	if (send(nl_sock, hdr, hdr->nlmsg_len, 0) == -1) {
		ret_errno = errno;
		perror("send failed");
		return -3;
	}
	return 0;
}

#ifdef FILTER
int register_proc_netlink(int *efd, struct proc_input *input)
#else
int register_proc_netlink(int *efd, enum proc_cn_mcast_op mcast_op)
#endif
{
	struct sockaddr_nl sa_nl;
	int err = 0, epoll_fd;

	nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);

	if (nl_sock == -1) {
		ret_errno = errno;
		perror("socket failed");
		return -1;
	}

	bzero(&sa_nl, sizeof(sa_nl));
	sa_nl.nl_family = AF_NETLINK;
	sa_nl.nl_groups = CN_IDX_PROC;
	sa_nl.nl_pid    = getpid();

	if (bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl)) == -1) {
		ret_errno = errno;
		perror("bind failed");
		return -2;
	}

#ifdef BPF_FILT
	/*Filter.len = sizeof(BPF_bad_exit)/sizeof(BPF_bad_exit[0]);
	Filter.filter = BPF_bad_exit; */

	Filter.len = sizeof(BPF_exit)/sizeof(BPF_exit[0]);
	Filter.filter = BPF_exit;

	if (setsockopt(nl_sock, SOL_SOCKET, SO_ATTACH_FILTER,
		       &Filter, sizeof(Filter)) < 0) {
		ret_errno = errno;
		perror("setsockopt attach filter failed");
		return -2;
	}
#endif

	if ((epoll_fd = epoll_create1(EPOLL_CLOEXEC)) < 0) {
		ret_errno = errno;
		perror("epoll_create1 failed");
		return -2;
	}

#ifdef FILTER
	if ((err = send_message(input)) < 0) {
#else
	if ((err = send_message(mcast_op)) < 0) {
#endif
		return err;
	}

	evn.events = EPOLLIN;
	evn.data.fd = nl_sock;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, nl_sock, &evn) < 0) {
		ret_errno = errno;
		perror("epoll_ctl failed");
		return -3;
	}
	*efd = epoll_fd;
	return 0;
}

static void sigint(__attribute__((unused)) int sig) {
	interrupted = 1;
}

int handle_packet(char *buff, int fd, struct proc_event *event) 
{
	struct nlmsghdr *hdr;

	hdr = (struct nlmsghdr *)buff;

	if (hdr->nlmsg_type == NLMSG_ERROR) {
		perror("NLMSG_ERROR error\n");
		return -3;
	} else if (hdr->nlmsg_type == NLMSG_DONE) {
		event = (struct proc_event *)
			((struct cn_msg *)NLMSG_DATA(hdr))->data;
		tcount++;
		switch(event->what) {
		case PROC_EVENT_EXIT:
			exit_count++;
			//send_message(fd, 1);
			Printf("Exit process %d (tgid %d) with code "
			       "%d, signal %d\n",
			       event->event_data.exit.process_pid,
			       event->event_data.exit.process_tgid,
			       event->event_data.exit.exit_code,
			       event->event_data.exit.exit_signal);
                 	break;
		case PROC_EVENT_FORK:
			fork_count++;
			Printf("Fork process %d (tgid %d), parent "
			       "%d (tgid %d)\n",
			       event->event_data.fork.child_pid,
			       event->event_data.fork.child_tgid,
			       event->event_data.fork.parent_pid,
			       event->event_data.fork.parent_tgid);
			break;
		case PROC_EVENT_EXEC:
			exec_count++;
			Printf("Exec process %d (tgid %d)\n",
			       event->event_data.exec.process_pid,
			       event->event_data.exec.process_tgid);
			break;
		case PROC_EVENT_UID:
			Printf("UID process %d (tgid %d) uid %d euid %d\n",
			       event->event_data.id.process_pid,
			       event->event_data.id.process_tgid,
			       event->event_data.id.r.ruid,
			       event->event_data.id.e.euid);
			break;
		case PROC_EVENT_GID:
			Printf("GID process %d (tgid %d) gid %d egid %d\n",
			       event->event_data.id.process_pid,
			       event->event_data.id.process_tgid,
			       event->event_data.id.r.rgid,
			       event->event_data.id.e.egid);
			break;
		case PROC_EVENT_SID:
			Printf("SID process %d (tgid %d)\n",
			       event->event_data.sid.process_pid,
			       event->event_data.sid.process_tgid);
			break;
		case PROC_EVENT_PTRACE:
			Printf("Ptrace process %d (tgid %d), Tracer "
			       "%d (tgid %d)\n",
			       event->event_data.ptrace.process_pid,
			       event->event_data.ptrace.process_tgid,
			       event->event_data.ptrace.tracer_pid,
			       event->event_data.ptrace.tracer_tgid);
			break;
		case PROC_EVENT_COMM:
			Printf("Comm process %d (tgid %d) comm %s\n",
			       event->event_data.comm.process_pid,
			       event->event_data.comm.process_tgid,
			       event->event_data.comm.comm);
			break;
		case PROC_EVENT_COREDUMP:
			Printf("Coredump process %d (tgid %d) parent "
			       "%d, (tgid %d)\n",
			       event->event_data.coredump.process_pid,
			       event->event_data.coredump.process_tgid,
			       event->event_data.coredump.parent_pid,
			       event->event_data.coredump.parent_tgid);
			break;
		default:
			break;
		}
	}
	return 0;
}

int handle_events(int epoll_fd, struct proc_event *pev)
{
	char buff[CONNECTOR_MAX_MSG_SIZE];
	struct epoll_event ev[MAX_EVENTS];
	int i, event_count = 0, err = 0;

	if ((event_count = epoll_wait(epoll_fd, ev, MAX_EVENTS, -1)) < 0) {
		ret_errno = errno;
		perror("epoll_wait failed");
		return -3;
	}
	for (i = 0; i < event_count; i++) {
		if (!(ev[i].events & EPOLLIN))
			continue;
	if (recv(ev[i].data.fd, buff, sizeof(buff), 0) == -1) {
		ret_errno = errno;
		perror("recv failed");
		return -3;
	}
	if ((err = handle_packet(buff, ev[i].data.fd, pev)) < 0)
		return err;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int epoll_fd, err;
	struct proc_event proc_ev;
#ifdef FILTER 
	struct proc_input input;
#endif
#ifdef RUSAGE_TIME
	struct rusage start, end;
	getrusage(RUSAGE_SELF, &start);
#endif

	signal(SIGINT, sigint);

	//input.ev_type = PROC_EVENT_BAD_EXIT | PROC_EVENT_FORK | PROC_EVENT_EXEC | PROC_EVENT_EXIT;
#ifdef FILTER 
	input.event_type = PROC_EVENT_EXIT;
	input.mcast_op = PROC_CN_MCAST_LISTEN;
#endif

#ifdef FILTER 
	if ((err = register_proc_netlink(&epoll_fd, &input)) < 0) {
#else
	if ((err = register_proc_netlink(&epoll_fd,
					 PROC_CN_MCAST_LISTEN)) < 0) {
#endif
		if (err == -2)
			close(nl_sock);
		if (err == -3) {
			close(nl_sock);
			close(epoll_fd);
		}
		exit (1);
	}

	while (!interrupted) {
		if ((err = handle_events(epoll_fd, &proc_ev)) < 0) {
			if (ret_errno == EINTR)
				continue;
			if (err == -2)
				close(nl_sock);
			if (err == -3) {
				close(nl_sock);
				close(epoll_fd);
			}
			exit (1);
		}
		if (exit_count >= 8000)
			break;
	}

#ifdef FILTER
	input.mcast_op = PROC_CN_MCAST_IGNORE;
	send_message(&input);
#else
	send_message(PROC_CN_MCAST_IGNORE);
#endif

	close(epoll_fd);
	close(nl_sock);

#ifdef RUSAGE_TIME
	getrusage(RUSAGE_SELF, &end);
	printf("Done exit_count: %d fork_count: %d tcount: %d exec_count %d\n",exit_count, fork_count, tcount, exec_count);
	printf("user time spent: %0.8f usec\n",(1000000*time_diff(&start.ru_utime, &end.ru_utime)));
	printf("sys time spent: %0.8f usec\n",(1000000*time_diff(&start.ru_stime, &end.ru_stime)));
#endif

	exit (0);
}
