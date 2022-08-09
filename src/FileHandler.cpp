#include "FileHandler.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netdb.h>

FileSender::FileSender(int _fd_from, int _fd_to)
    : fd_from(_fd_from), fd_to(_fd_to)
{
    memset(buf, 0, read_buf);
}

FileSender *FileSender::Make(const char *name, int _fd_to)
{
    int fd_from = open(name, O_RDONLY);
    if (-1 == fd_from)
        return 0;
    return new FileSender(fd_from, _fd_to);
}

bool FileSender::Send()
{
    int rc = read(fd_from, buf, read_buf);
    if (rc == -1)
    {
        close(fd_from);
        return false;
    }
    write(fd_to, buf, rc);
    if (rc != read_buf)
    {
        close(fd_from);
        return false;
    }
    return true;
}