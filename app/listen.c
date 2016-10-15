#include <stropts.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


//int main(void)
//{
//	printf(">>>>>>>  Poll example 3 start \n");
//
//	char dummybuf[4];
//
//	struct pollfd fds;
//	        fds.fd = open("/sys/kernel/kobject_example/foo", O_RDONLY);
//	        read(fds.fd, &dummybuf, 4);
//	        fds.events = POLLPRI;
//
//	        do{
//	                int ret = poll(&fds, 1, -1);
//	                if(ret > 0){
//	                        FILE *fp = fopen("/sys/kernel/kobject_example/foo", "r");
//	                        printf("   change \n");
//
//	                        lseek(fds.fd, 0, SEEK_SET);
//	                        fclose(fp);
//	                        read(fds.fd, &dummybuf, 4);
//	                }
//	        }while(1);
//
//	        close(fds.fd);
//}

int main(void){
	printf(">>>>>>>  Poll example 2 start \n");

	struct pollfd fds[2];
	int timeout_msecs = 10000;
	int ret;
    int i;

    char buf[10];
    	int len;
    	int val=0;
    	int *pval;

    /*
     * Open STREAMS device. */
    fds[0].fd = open("/sys/kernel/cnodes/node_0/in_reg_1", O_RDONLY);
    fds[1].fd = open("/sys/kernel/cnodes/node_1/in_reg_1", O_RDONLY);

    fds[0].events = POLLPRI ;
    fds[1].events = POLLPRI ;

    if (fds[0].fd <0)
    	{
    		printf("Can't open\n");
    	}else{
    		printf("Open \n");
    	}
    if (fds[1].fd <0)
        	{
        		printf("Can't open /dev/a/2\n");
        	}else{
        		printf("Open /dev/a/2\n");
        	}

   while (1){
	   printf("Poll start \n");



	ret = poll(fds, 2, -1);
	   printf("Poll end \n");
	if (ret > 0) {

		for (i=0; i<2; i++) {
			printf("check %d\n", i);
		        if (fds[i].revents & POLLPRI) {

		        	printf("File change = %d\n", i);
		        	read(fds[i].fd, buf, 4  );
		        	lseek(fds[i].fd, 0, SEEK_SET);
		        }
		    }

//		FILE *fp = fopen("/sys/kernel/kobject_example/foo", "r");
//		printf("Poll in ret = %d \n", ret);
//		len = read(fds[0].fd, buf, 4  );
//		pval=(int *)buf;
//		val =*pval;
//		printf("File  change - val: %d,  buf %d %d %d %d\n", val, buf[0], buf[1], buf[2], buf[3] );



	}


   }
	printf("\n END \n");
}
