#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <resource.h>

int main(int argc, char **argv)
{
	int intfs;
	struct ifstats *ifs = NULL, *ifsn;

	intfs = res_read(RES_NET_DEV_ALL, NULL, 0, (void **)&ifs, 0, 0);
	if (intfs < 0) {
		printf("res_read() returned %d\n", intfs);
		exit(1);
	}
	ifsn = ifs;
	printf("No. of interfaces: %d\n",intfs);
	for (int i = 0; i < intfs; i++) {
		printf("Interface %s\n", ifs->ifname);
		printf("rx_packets %llu\n",ifs->st64.rx_packets);
		printf("tx_packets %llu\n",ifs->st64.tx_packets);
		printf("rx_bytes %llu\n",ifs->st64.rx_bytes);
		printf("tx_bytes %llu\n",ifs->st64.tx_bytes);
		ifs++;
	}
	if (ifsn)
		free(ifsn);
}
