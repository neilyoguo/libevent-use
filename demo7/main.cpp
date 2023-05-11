#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

void read_cb(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *input = bufferevent_get_input(bev);
    std::cout <<evbuffer_get_length(input) <<std::endl; 
    while(evbuffer_get_length(input))
    {
        char buf[4]={0};
        int ret = bufferevent_read(bev, buf, sizeof(buf));
        if(ret < 0)
        {
            std::cout <<"read error"<< std::endl;
        }
        else
        {
           std::cout << "msg is :" << buf << std::endl;
        }
    }  
}

void event_cb(struct bufferevent *bev, short events, void *ctx)
{
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) 
    {
        std::cout << "client exit" << std::endl;
    }
    else
    {
       std::cout << "other error" << std::endl; 
    }
    bufferevent_free(bev);
}
void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *user_data)
{

    struct sockaddr_in * addr = (sockaddr_in * )sa;
    char ip[16] = "";
    int port = ntohs(addr->sin_port);
    inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));
    std::cout << "client ip:"<< ip <<" port:" << port << std::endl;
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);

    bufferevent_setwatermark(bev, EV_READ, 20, 0);

    bufferevent_enable(bev, EV_READ);


}

int main(int argc , char *argv[])
{
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000) ;
    server_addr.sin_addr.s_addr =  htonl(INADDR_ANY);
    //初始化事件集合
    struct event_base *base =  event_init();

    struct evconnlistener *listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
	    LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
	    (struct sockaddr*)&server_addr,
	    sizeof(server_addr));

    //开始监听事件
    event_base_dispatch(base);

    evconnlistener_free(listener);

    return 0;
}