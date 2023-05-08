#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc , char *argv[])
{
    const char *pathName = "../fifo.pipe"; 
    int fd = open(pathName,O_WRONLY);
    if(-1 == fd)
    {
        std::cout << "open pipe err" << std::endl;
        return 0;
    }
    while(1)
    {
        char buf[32] = {0};
        scanf("%s", buf);
        int ret = write(fd, buf, sizeof(buf));
        if(ret > 0)
        {
            std::cout << "write data....." <<std::endl;
        }
    }
    return 0;
}