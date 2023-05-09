#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <event.h>

//当事件满足条件的时候，会触发callback 函数
void time_cb(evutil_socket_t fd, short events, void *arg)
{
    time_t t;
	struct tm *timeinfo;  
	time(&t);
	timeinfo = localtime(&t);
	std::cout << timeinfo->tm_sec << std::endl;
}

int main(int argc , char *argv[])
{
    struct event ev;
    struct timeval tv;
    //初始化事件集合
    struct event_base *base =  event_init();

    tv.tv_sec = 1; 
    tv.tv_usec = 0;
    evtimer_set(&ev , time_cb , NULL);

    event_base_set(base, &ev);
    
    event_add(&ev, &tv);


    //开始监听事件
    event_base_dispatch(base);
    return 0;
}