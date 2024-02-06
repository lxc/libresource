enum {
        NET_ROUTE,
        NET_ARP,
        NET_IF,
        NET_MAX
};

//extern char net_file[3][20];

extern int connect_netlink(int groups);
extern int get_net_len(int net_sock);
extern int parse_rt_attr(struct rtattr *at[], struct rtattr *rta, int len);
extern void print_rt(struct rtmsg *rt);
extern int handle_net_resp(int net_type, int net_sock, void **out);
extern void print_rt_info(struct rt_info *rt, FILE *fp);
extern int get_rt_attr(struct rtattr *at[], struct rt_info *rt, struct rtmsg *m);
extern int parse_arp_attr(struct rtattr *at[], struct rtattr *rta, int len);
#ifdef TESTING
extern int get_arp_attr(struct rtattr *at[], struct arp_info *arp,
                struct ndmsg *n,
                FILE *fp);
#else
extern int get_arp_attr(struct rtattr *at[], struct arp_info *arp,
                struct ndmsg *n);
#endif
extern int parse_if_attr(struct rtattr *at[], struct rtattr *rta, int len);
#ifdef TESTING
extern void print_if_info(struct ifstats *ifs, FILE *fp);
#endif
extern void print_if(struct ifstats *ifs);
