#include <iostream>
#include <stdlib.h>
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
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
using namespace std;

#define PORT 80

void udp_read_cb(evutil_socket_t fd, short events, void *arg) 
{
	char buffer[1024];
	struct sockaddr_in their_addr;
	socklen_t addr_size = sizeof their_addr;
	int nbytes = recvfrom(fd, buffer, sizeof buffer, 0, (struct sockaddr *)&their_addr, &addr_size);
	if (nbytes < 0) 
	{
		std::cerr << "recvfrom failed" << std::endl;
		return;
	}
}


int main()
{

	struct event_base *base = event_init();
	// 创建 UDP 套接字
    int fd = socket(AF_INET , SOCK_STREAM, 0 );
    if(fd < 0 )
    {
        cout<< "create sock err" <<endl;
        return 0;
    }
	// 绑定 IP 地址和端口号
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		cout << "Failed to bind socket" << endl;
		return 1;
	}

	struct event *udp_event = event_new(base, fd, EV_READ | EV_PERSIST, udp_read_cb, NULL);

	event_add(udp_event, NULL);
	//开始监听事件
	int t = event_base_dispatch(base);
	event_base_free(base);


	return 0;
}