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