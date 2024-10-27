#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <resource.h>

int main(int argc, char **argv)
{
        unsigned long value;
        int ret;
	int pid = 43044;

	/* Non-zero value of pid field (second last variable) to res_read
	 * indicates to read RES_MEM_FREE value from cgroups
	 */
        ret = res_read(RES_CPU_STAT_USAGE, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_CPU_STAT_USAGE returned error %d\n",ret);
                exit(1);
        }
	printf("RES_CPU_STAT_USAGE for pid %d is %lu\n", pid, value);

        ret = res_read(RES_CPU_STAT_USER, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_CPU_STAT_USER returned error %d\n",ret);
                exit(1);
        }
	printf("RES_CPU_STAT_USER for pid %d is %lu\n", pid, value);

        ret = res_read(RES_CPU_STAT_SYSTEM, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_CPU_STAT_SYSTEM returned error %d\n",ret);
                exit(1);
        }
	printf("RES_CPU_STAT_SYSTEM for pid %d is %lu\n", pid, value);

        ret = res_read(RES_CPU_WEIGHT, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_CPU_WEIGHT returned error %d\n",ret);
                exit(1);
        }
	printf("RES_CPU_WEIGHT for pid %d is %lu\n", pid, value);

        ret = res_read(RES_CPU_WEIGHT_NICE, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_CPU_WEIGHT_NICE returned error %d\n",ret);
                exit(1);
        }
	printf("RES_CPU_WEIGHT_NICE for pid %d is %lu\n", pid, value);

        ret = res_read(RES_CPU_MAX, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_CPU_MAX returned error %d\n",ret);
                exit(1);
        }
	printf("RES_CPU_MAX for pid %d is %lu\n", pid, value);
}
