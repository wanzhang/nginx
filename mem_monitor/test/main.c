#include <stdio.h>
#include "mem_monitor.h"

int main(int argc , char* argv[]) {
	int ret = 0;
	char buf[256] = {0};
	ret = get_sys_free_mem(buf, 256);
	if (ret) {
		printf("get_sys_free_mem fail, ret:%d\n", ret);
		return -1;
	}
	printf("%s", buf);
	return 0;
}
