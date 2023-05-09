#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <event.h>

//当事件满足条件的时候，会触发callback 函数
void fifo_read(evutil_socket_t fd, short events, void *arg)
{
    char buf[10] = {0};
    int ret = read(fd, buf, sizeof(buf));
    if(ret > 0)
    {
        std::cout << buf << std::endl;
    }

}
int main(int argc , char *argv[])
{
    struct event ev;
    const char *pathName = "fifo.pipe"; 
    int ret = mkfifo(pathName, 0666);
    if(-1 == ret)
    {
        std::cout << "create pipe err" << std::endl;
    }
    int fd = open(pathName,O_RDONLY);
    if(-1 == fd)
    {
        std::cout << "open pipe err" << std::endl;
    }

    //初始化事件集合
    struct event_base *base =  event_init();

    //把fd 和 读事件绑定
    event_set(&ev, fd, EV_READ | EV_PERSIST, fifo_read, NULL);

    event_base_set(base, &ev);
    event_add(&ev, NULL);

    //开始监听事件
    event_base_dispatch(base);
    return 0;
}