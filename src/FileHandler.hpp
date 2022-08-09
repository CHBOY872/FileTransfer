#ifndef FILEHANDLER_HPP_SENTRY
#define FILEHANDLER_HPP_SENTRY

enum
{
    read_buf = 1024
};

class FileSender
{
    int fd_from;
    int fd_to;
    unsigned char buf[read_buf];
    FileSender(int _fd_from, int _fd_to);

public:
    static FileSender *Make(const char *name, int _fd_to);
    bool Send();
};

#endif