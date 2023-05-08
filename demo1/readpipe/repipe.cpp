#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <event.h>

//当事件满足条件的时候，会触发callback 函数
void fifo_read(evutil_socket_t fd, short events, void *arg)
{
    char buf[32] = {0};
    int ret = read(fd, buf, sizeof(buf));
    if(ret == -1)
    {
        std::cout << "read err" << std::endl;
    }
    std::cout << buf << std::endl;
}
int main(int argc , char *argv[])
{
    struct event evfifo;
    const char *pathName = "../fifo.pipe"; 
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
    event_init();

    //把fd 和 事件绑定
    event_set(&evfifo, fd, EV_READ, fifo_read, NULL);

    //把事件添加到集合中
    event_add(&evfifo, NULL);

    //开始监听事件
    event_dispatch();
    return 0;
}