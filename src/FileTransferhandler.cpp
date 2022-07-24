#include "FileTransferhandler.hpp"
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

FileSendHandler::FileSendHandler(int _fd_from, int _fd_to, char *_file_name)
    : fd_from(_fd_from), fd_to(_fd_to), file_name(_file_name) {}

FileSendHandler::~FileSendHandler()
{
    close(fd_from);
}

FileSendHandler *FileSendHandler::Start(int fd_to, char *_file_name)
{
    int _fd_from = open(_file_name, O_RDONLY, 0777);
    // int _fd_from = open(_file_name, O_RDONLY | O_NONBLOCK, 0777);
    if (-1 == _fd_from)
    {
        perror("file:");
        return 0;
    }

    return new FileSendHandler(_fd_from, fd_to, _file_name);
}

bool FileSendHandler::Send()
{
    bzero(buffer, send_size_buf);
    int stat = read(fd_from, buffer, send_size_buf);
    if (stat == -1)
    {
        perror("read:");
        return false;
    }

    write(fd_to, buffer, stat);
    if (stat > 0)
        return true;
    return false;
}