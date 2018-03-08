#ifndef _RESOURCE_IMPL_H
#define _RESOURCE_IMPL_H

#include <fcntl.h>
#include <errno.h>
#define SIZE_OF_TEMP_BUF	2048
#define eprintf(msg, ...)	fprintf(stderr,\
					"Err at line %d in file %s: "msg"\n",\
					__LINE__, __FILE__, ##__VA_ARGS__)\


static inline int file_to_buf(char *fname, char *buf)
{
	int fd = 0;
	size_t rdsz = 0;
	int err = 0;

	fd = open(fname, O_RDONLY);
	if (fd == -1) {
		err = errno;
		eprintf("in opening File %s with errno: %d", fname, errno);
		errno = err;
		return -1;
	}

	if (lseek(fd, 0L, SEEK_SET) == -1) {
		err = errno;
		eprintf("in lseek for File %s with errno: %d", fname, errno);
		close(fd);
		errno = err;
		return -1;
	}

	rdsz = read(fd, buf, SIZE_OF_TEMP_BUF - 1);
	if (rdsz < 0) {
		err = errno;
		eprintf("in read from File %s with errno: %d", fname, errno);
		close(fd);
		errno = err;
		return -1;
	}
	buf[rdsz] = '\0';
	close(fd);
	return rdsz;
}

#endif /* _RESOURCE_IMPL_H */
