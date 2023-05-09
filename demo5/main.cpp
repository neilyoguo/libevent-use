#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <event.h>
#include <signal.h>

void signal_cb(evutil_socket_t fd, short events, void *arg)
{
    std::cout << "signal is" << fd << std::endl;
}

int main(int argc , char *argv[])
{
    struct event ev;
    //初始化事件集合
    struct event_base *base =  event_init();

    event_assign(&ev, base, SIGINT, EV_SIGNAL | EV_PERSIST, signal_cb, NULL);
    
    event_add(&ev, NULL);

    //开始监听事件
    event_base_dispatch(base);
    return 0;
}