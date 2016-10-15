#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>


double GetTickCount(void)
{
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now))
    return 0;
  return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}


int main( ){
printf("Hello world\n");

unsigned short v1 =GetTickCount();
int fd;
int ret_val;


fd = open("/sys/kernel/cnodes/node_0/in_reg_1",  O_WRONLY);
if (fd<0)
{
printf("Can't open\n");
}else{
	printf("Open\n");
}
char *buf = (char*)&v1;
int len = write(fd, buf, sizeof(unsigned short));

printf("Write %d , %d %d %d %d\n",v1,  buf[0], buf[1], buf[2], buf[3]);
printf("\n");

printf("\n END \n");

close (fd);


return 0;
}
