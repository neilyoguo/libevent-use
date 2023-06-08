# libevent-use

## 编译libevent
从官网下载安装包  https://libevent.org/
然后解压编译安装，这里要注意可能会报这个错误
```
checking for openssl/ssl.h... no
configure: error: openssl is a must but can not be found. You should add the directory containing `openssl.pc' to the `PKG_CONFIG_PATH' environment variable, or set `CFLAGS' and `LDFLAGS' directly for openssl, or use `--disable-openssl' to disable support for openssl encryption
```
它是因为 libevent 与 openssl 版本不兼容导致
|  libevent   | openssl  |
|  ----  | ----  |
| 2.1.x  | 1.1以上 |
| 2.0.x  | 1.0 |

如下，都是可以编译成功的
|  libevent   | openssl  |
|  ----  | ----  |
| libevent-2.0.22-stable.tar.gz  | OpenSSL 1.0.2k-fips  26 Jan 2017 |
| libevent-2.1.12-stable.tar.gz  | OpenSSL 3.0.0 7 sep 2021 (Library: OpenSSL 3.0.0 7 sep 2021) |

安装
```
tar zxvf libevent-2.0.22-stable.tar.gz
cd libevent-2.0.22-stable
#设置安装路径 不加--prefix=/usr，默认安装目录在/usr/local/lib
./configure --prefix=/usr
make && make install
```
## 使用
```
初始化事件处理器集合 -> 
创建一个事件 -> 
将事件注册到事件集合 -> 
添加事件到事件处理器的事件队列中 ->  
让事件集合进入事件循环处理事件

```
event 结构体原型
```
struct event {
	TAILQ_ENTRY(event) ev_active_next;
	TAILQ_ENTRY(event) ev_next;
	/* for managing timeouts */
	union {
		TAILQ_ENTRY(event) ev_next_with_common_timeout;
		int min_heap_idx;
	} ev_timeout_pos;
	evutil_socket_t ev_fd; //事件关联的文件描述符

	struct event_base *ev_base;  

	union {
		/* used for io events */
		struct {
			TAILQ_ENTRY(event) ev_io_next;
			struct timeval ev_timeout;
		} ev_io;

		/* used by signal events */
		struct {
			TAILQ_ENTRY(event) ev_signal_next;
			short ev_ncalls; //回调函数的调用次数
			/* Allows deletes in callback */
			short *ev_pncalls; //指向该事件的回调函数已经被调用的次数的指针
		} ev_signal;
	} _ev;

	short ev_events; //要监听的事件类型。
	short ev_res;		/* result passed to event callback */
	short ev_flags;//标识该事件的状态，包括 EVLIST_INSERTED、EVLIST_ACTIVE、EVLIST_TIMEOUT 等
	ev_uint8_t ev_pri;	/* smaller numbers are higher priority */
	ev_uint8_t ev_closure;
	struct timeval ev_timeout;//指定该事件的超时时间，用于在一定时间内等待该事件发生

	/* allows us to adopt for different types of events */
	void (*ev_callback)(evutil_socket_t, short, void *arg);//事件发生时要调用的回调函数
	void *ev_arg;//传递给回调函数的参数
};
```
event对象初始化有多种方式，但是本质上都是通过event_assign进行初始化的，如下：
```
直接通过event_assign 初始化
int event_assign(struct event *ev, struct event_base *base, evutil_socket_t fd, short events, void (*callback)(evutil_socket_t, short, void *), void *arg)

```
```
通过event_set初始化
void event_set(struct event *ev, evutil_socket_t fd, short events,
	  void (*callback)(evutil_socket_t, short, void *), void *arg)
{
	int r;
	r = event_assign(ev, current_base, fd, events, callback, arg);
	EVUTIL_ASSERT(r == 0);
}
```
```
通过event_new , 可以看到event_new是在堆上创建的，需要通过event_free进行释放
struct event *
event_new(struct event_base *base, evutil_socket_t fd, short events, void (*cb)(evutil_socket_t, short, void *), void *arg)
{
	struct event *ev;
	ev = mm_malloc(sizeof(struct event));
	if (ev == NULL)
		return (NULL);
	if (event_assign(ev, base, fd, events, cb, arg) < 0) {
		mm_free(ev);
		return (NULL);
	}

	return (ev);
}
```
```
event_init()
该函数会创建一个全局的事件处理器（event_base）对象，这个对象是事件处理的中心，所有的事件都将注册到这个event_base上
```
```
int event_set(struct event *ev, evutil_socket_t fd, short events, void (*callback)(evutil_socket_t, short, void *), void *arg)
ev:指向事件结构体的指针
fd:是事件关联的文件描述符
events:事件类型，是一个宏定义，可以使用位运算符（｜）将它们组合起来如下：
        #define EV_TIMEOUT     0x01    定时器事件
        #define EV_READ        0x02    可读事件
        #define EV_WRITE       0x04    可写事件 
        #define EV_SIGNAL      0x08    信号事件
        #define EV_PERSIST     0x10    持续事件，当事件触发执行后，不会从事件队列删除，依然存在
        #define EV_ET          0x20    指定事件采用边缘出发模式，默认采用水平触发模式

该函数创建一个事件并设置事件的相关参数
```
```
int event_base_set(struct event_base *base, struct event *ev)
该函数用于将一个事件注册到指定的事件处理器上，从而使该事件能够被该事件处理器处理，
该函数将事件关联到事件处理器后，就可以调用`event_base_loop`函数开始事件循环，等待事件发生，
并通过注册的回调函数来处理事件

```
```
int event_add(struct event *ev, const struct timeval *timeout)
该函数用于将一个事件添加到事件处理器的事件队列中，从而使该事件能够被该事件处理器处理，
事件处理器会在事件超时时间到达时开始处理该事件，如果事件超时时间为NULL，则该事件会立即被处理
```
```
event_base_dispatch()
该函数用于启动事件循环并开始处理事件。它会一直阻塞当前线程，直到事件循环中没有任何事件需要处理为止

```
## 定时器事件
### 单个事件循环 [demo1](https://github.com/neilyoguo/libevent-use/tree/main/demo1)
### 多个事件循环 [demo2](https://github.com/neilyoguo/libevent-use/tree/main/demo2)
### 也可以用 evtimer_set 创建一个事件[demo3](https://github.com/neilyoguo/libevent-use/tree/main/demo3)

```
evtimer_set是个宏定义，本质上还是event_set，只不过默认设置了fd（-1） 和 事件类型（0），在指定的时间后触发该事件，通过该函数设置的事件都不是持续性事件
#define evtimer_set(ev, cb, arg)	event_set((ev), -1, 0, (cb), (arg))

```
## IO事件

管道通信 [demo4](https://github.com/neilyoguo/libevent-use/tree/main/demo4)

## 信号事件
[demo5](https://github.com/neilyoguo/libevent-use/tree/main/demo5)

## TCP

### 使用
```
初始化事件处理器集合 -> 
创建一个监听器（listener）并绑定到指定的IP地址和端口上 -> 
将链接事件注册到事件集合 -> 
添加事件到事件处理器的事件队列中 ->  
让事件集合进入事件循环处理事件

```
这里socket的create,bind,listen包括将accept事件注册添加到base集合中都是由evconnlistener_new_bind函数完成的
调用链
```
evconnlistener_new_bind()
	->socket()
	->evutil_make_socket_nonblocking()->fcntl() 设置socket非阻塞
	->bind()
	->evconnlistener_new()->listen()
						  ->event_assign()
```
```
struct evconnlistener *evconnlistener_new_bind(struct event_base *base,
    evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
    const struct sockaddr *sa, int socklen)

base：事件处理器event_base对象，用于接收新连接事件；
cb：回调函数，当有新连接到达时，将被调用；
ptr：传递给回调函数的参数；
flags：监听器的选项，可以为LEV_OPT_LEAVE_SOCKETS_BLOCKING,LEV_OPT_CLOSE_ON_FREE等标志的组合；
backlog：监听队列的长度，表示等待处理的最大连接数；
		if (backlog > 0) {
			if (listen(fd, backlog) < 0)
				return NULL;
		} else if (backlog < 0) {
			if (listen(fd, 128) < 0)
				return NULL;
		}
sa：指向要绑定的sockaddr结构体；
socklen：sockaddr结构体的长度。

```
```
当有连接时触发的回调函数
void evconnlistener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)

listener：触发回调函数的evconnlistener对象；
fd：新连接的描述符；
sa：连接的sockaddr结构体；
socklen：sockaddr结构体的长度；
user_data：传递给evconnlistener_new_bind函数的ptr参数。

```
### socket数据读写

#### 创建bufferent
需要使用到基于socket的bufferevent，可以使用bufferevent_socket_new()函数创建一个基于套接字的bufferevent，函数原型如下

```
struct bufferevent *bufferevent_socket_new(struct event_base *base, evutil_socket_t fd, int options)
base:是一个指向 `event_base` 类型的指针，表示要将新创建的缓冲区事件对象添加到哪个事件集合中
fd：是一个类型为 `evutil_socket_t` 的整数，表示要关联的套接字文件描述符
options：是要应用于新创建的缓冲区事件对象的选项。可以使用以下选项之一或多个：
		`BEV_OPT_CLOSE_ON_FREE`：在销毁缓冲区事件对象时自动关闭关联的套接字。
		`BEV_OPT_THREADSAFE`：启用多线程支持。
		`BEV_OPT_DEFER_CALLBACKS`：延迟回调函数的执行。

```
```
struct bufferevent 
在这个事件对象中包含了各种数据结构和回调函数，用于管理缓冲区和事件
比如可能会经常使用：
    struct evbuffer *input;     	输入缓冲区 
    struct evbuffer *output;    	输出缓冲区 
    bufferevent_data_cb readcb; 	读回调函数 
    bufferevent_data_cb writecb;	写回调函数 
    bufferevent_event_cb errorcb;	错误回调函数 
    void *cbarg;                	回调函数参数 

```
#### 释放bufferevent
```
void bufferevent_free(struct bufferevent *bev)
```
#### 设置回调函数
bufferevent_setcb()函数修改bufferevent的一个或多个回调。当数据被读取时调用读回调，当数据被写入时调用写回调，当有error事件发生时调用error回调

#### 读写bufferevent
首先要获取各自的输入输出缓冲区
```
struct evbuffer *bufferevent_get_input(struct bufferevent *bufev)
struct evbuffer *bufferevent_get_output(struct bufferevent *bufev)
```
读数据
```
此处需要通过evbuffer_get_length获取缓冲区大小，将缓冲区数据全部读出，否则设置读取的长度太小会导致有的数据遗留在缓冲区中
size_t bufferevent_read(struct bufferevent *bufev, void *data, size_t size)
数据读出后，数据将从缓冲区移除

```
写数据
```
size_t bufferevent_write(struct bufferevent *bufev, void *data, size_t size)
```

具体实现[demo6](https://github.com/neilyoguo/libevent-use/tree/main/demo6)

#### 设置bufferevent水位线

```
int bufferevent_setwatermark(struct bufferevent *bev, short events, size_t lowmark, size_t highmark)
其中，`bev` 是一个 `bufferevent` 对象的指针，`events` 可以是 `EV_READ` 或 `EV_WRITE`，
用于指定是设置输入水位线还是输出水位线。`lowmark` 是设置的低水位线，`highmark` 是设置的高水位线 ，单位是字节，如果不需要设置参数是0
当缓冲区的数据量超出指定的高水位线时，Libevent 将停止读入或写出数据，数据会被阻塞在应用程序或操作系统的发送缓冲区中

```
 设置读最低水位线为20个字节，当缓冲区 >= 20个字节时，才会触发回调，把数据全部读出来, 见[demo7](https://github.com/neilyoguo/libevent-use/tree/main/demo7)

## http

```
初始化事件处理器集合 -> 
创建一个 HTTP 服务器 -> 
绑定端口与ip到该服务上 -> 
让事件集合进入事件循环处理事件
```
```
创建http 服务器
struct evhttp *evhttp_new(struct event_base *base);
```
```
绑定ip地址和端口到http服务上 ，0成功 -1失败
int evhttp_bind_socket(struct evhttp *http, const char *address, ev_uint16_t port);
```
```
设置 HTTP 服务器的通用回调函数
void evhttp_set_gencb(struct evhttp *http, evhttp_cb cb, void *arg);
```
```
为指定的 URI 设置回调
void evhttp_set_cb(struct evhttp *http, const char *path, void (*cb)(struct evhttp_request *, void *), void *cb_arg);
```
```
获取 HTTP 请求的 URI（Uniform Resource Identifier，统一资源标识符）
const char* evhttp_request_get_uri(const struct evhttp_request* req);
```
```
获取 HTTP 请求的类型（例如 GET、POST、HEAD 等）
enum evhttp_cmd_type evhttp_request_get_command(const struct evhttp_request* req);
返回值是一个 `enum evhttp_cmd_type` 类型的枚举值，表示 HTTP 请求的类型。常见的枚举值包括：
- `EVHTTP_REQ_GET`: GET 方法
- `EVHTTP_REQ_POST`: POST 方法
- `EVHTTP_REQ_HEAD`: HEAD 方法
- `EVHTTP_REQ_PUT`: PUT 方法
- `EVHTTP_REQ_DELETE`: DELETE 方法
- `EVHTTP_REQ_OPTIONS`: OPTIONS 方法

```
[demo8](https://github.com/neilyoguo/libevent-use/tree/main/demo8)