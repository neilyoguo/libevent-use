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