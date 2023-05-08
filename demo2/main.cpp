#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <event.h>
#include <vector>


//当事件满足条件的时候，会触发callback 函数
void time_cb(evutil_socket_t fd, short events, void *arg)
{
    int a = * (int *) arg;
    time_t t;
	struct tm *timeinfo;  
	time(&t);
	timeinfo = localtime(&t);
	std::cout << a << "-------"<<timeinfo->tm_sec << std::endl;
}

int main(int argc , char *argv[])
{

    struct event_base *base =  event_init();
    struct event ev[5];
    int arg[5] = {0,1,2,3,4};
    struct timeval tv;
    tv.tv_sec = 1; 
    tv.tv_usec = 0;
    for(int t = 0 ; t< 5; t++)
    {

        event_set(&ev[t] , -1 , EV_PERSIST , time_cb , &arg[t]);
        event_base_set(base, &ev[t]);
        event_add(&ev[t], &tv);
    }

    //开始监听事件
    event_base_dispatch(base);

    return 0;
}