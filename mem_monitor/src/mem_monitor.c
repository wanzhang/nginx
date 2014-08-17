#include "mem_monitor.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int get_sys_free_mem(char *buf, int size) {
	FILE *fd = NULL;
	int	pos = 0;
	time_t now = time(NULL);
	char *time_str = ctime(&now);
	pos = strlen(time_str)-1;
	buf[0] = '[';
	memcpy(buf+1, time_str, pos);
	buf[pos+1] = ']';
	pos += 2;

	fd = fopen("/proc/meminfo", "r"); 
	if (NULL == fd)
		return -1;
	if (NULL == fgets(buf+pos, size-pos, fd))
		return -1;
	if (NULL == fgets(buf+pos, size-pos, fd))
		return -1;
	return 0;
}


