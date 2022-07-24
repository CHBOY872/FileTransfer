#ifndef FILETRANSFERHANDLER_HPP_SENTRY
#define FILETRANSFERHANDLER_HPP_SENTRY

enum
{
    max_file_name = 1024,
    send_size_buf = 512
};

class FileSendHandler
{
    int fd_from, fd_to;
    char *file_name;
    char buffer[send_size_buf];
    FileSendHandler(int _fd_from, int _fd_to, char *_file_name);

public:
    ~FileSendHandler();
    static FileSendHandler *Start(int fd_to, char *_file_name);
    bool Send();
};

#endif