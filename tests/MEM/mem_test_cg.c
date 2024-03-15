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
        ret = res_read(RES_MEM_AVAILABLE, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_MEM_AVAILABLE returned error %d\n",ret);
                exit(1);
        }
	printf("RES_MEM_AVAILABLE for pid %d is %lu\n", pid, value);

        ret = res_read(RES_MEM_TOTAL, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_MEM_TOTAL returned error %d\n",ret);
                exit(1);
        }
	printf("RES_MEM_TOTAL for pid %d is %lu\n", pid, value);

        ret = res_read(RES_MEM_FREE, &value, sizeof(value), NULL,
			pid, 0);
        if (ret != 0) {
                printf("RES_MEM_FREE returned error %d\n",ret);
                exit(1);
        }
	printf("RES_MEM_FREE for pid %d is %lu\n", pid, value);
}
