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

void http_cb(struct evhttp_request* req, void* arg)
{

    //获取客户端请求的URI(使用evhttp_request_uri或直接req->uri)
    const char *uri = evhttp_request_uri(req);
    printf("uri = %s\n", uri);
    const char *cmdtype;

    //获取请求类型
    switch (evhttp_request_get_command(req)) 
    {
      case EVHTTP_REQ_GET: cmdtype = "GET"; break;
      case EVHTTP_REQ_POST: cmdtype = "POST"; break;
      case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
      case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
      case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
      case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
      case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
      case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
      case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
      default: cmdtype = "unknown"; break;
    }
    printf("request type  = %s \n",cmdtype);

    //遍历头部信息
    struct evkeyvalq *headers;
    struct evkeyval *header;
    headers = evhttp_request_get_input_headers(req);
    for (header = headers->tqh_first; header != NULL; header = header->next.tqe_next) 
    {
      printf("%s: %s\n", header->key, header->value);
    }
    //查找头部信息
    const char *content_type = evhttp_find_header(headers, "Content-Type");
    if (content_type != NULL) 
    {
      printf("Content-Type: %s\n", content_type);
    } 
    else 
    {
        printf("Content-Type not found\n");
    }
    //获取消息
    evbuffer* inbuf = evhttp_request_get_input_buffer(req);
    while (evbuffer_get_length(inbuf))
    {
      char buf[1024];
      int n = evbuffer_remove(inbuf, buf, sizeof(buf) - 1);
      if (n > 0) 
      {
        buf[n] = '\0';
        printf("%s" , buf);
      }
    }

    // 发送HTTP响应
    struct evbuffer *evbuf = evbuffer_new();
    if (evbuf == NULL) 
    {
      return;
    }
    // 设置HTTP响应头
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/html; charset=UTF-8");
    // 构造HTTP响应内容
    const char *content = "<html><body><h1>Hello,neilyoguo</h1></body></html>";
    evbuffer_add(evbuf, content, strlen(content));
    // 发送HTTP响应
    evhttp_send_reply(req, HTTP_OK, "OK", evbuf);
    evbuffer_free(evbuf);
}

void http_jpg_cb(struct evhttp_request* req, void* arg)
{
    // 发送HTTP响应
    struct evbuffer *evbuf = evbuffer_new();
    if (evbuf == NULL) 
    {
      return;
    }
    // 设置HTTP响应头
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",  "image/jpeg");
    //读取图片
    FILE *fp = fopen("qop.jpg", "rb");

    if(fp == NULL)
    {
        evhttp_send_error(req, HTTP_NOTFOUND, "File Not Found");
        evbuffer_free(evbuf);
        return ;
    }
    fseek(fp, 0, SEEK_END);//定位文件指针到文件尾。
    int size = ftell(fp);//获取文件指针偏移量，即文件大小

    fseek(fp, 0, SEEK_SET);
    char *buffer = (char *)malloc(size);
    size_t n = fread(buffer, 1, size, fp);

    evbuffer_add(evbuf, buffer, size);
    evhttp_send_reply(req, HTTP_OK, "OK", evbuf);
    free(buffer);
    evbuffer_free(evbuf);
    fclose(fp);//关闭文件。

}


int main(int argc , char *argv[])
{

    //初始化事件集合
    struct event_base *base =  event_init();
     //http 服务器
    //1. 创建evhttp上下文
    evhttp* evh = evhttp_new(base);

    //2. 绑定端口与ip
    if (evhttp_bind_socket(evh, "172.17.127.201", 80) != 0) {
        std::cout << "evhttp_bind_socket failed!" << std::endl;
    }

    //3. 设定通用回调函数
    evhttp_set_gencb(evh, http_cb, NULL);
    //设置其他uri回掉函数
    evhttp_set_cb(evh,"/jpg",http_jpg_cb,NULL);
    //开始监听事件
    event_base_dispatch(base);

    evhttp_free(evh);

    return 0;
}