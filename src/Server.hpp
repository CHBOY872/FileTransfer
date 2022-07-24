#ifndef SERVER_HPP_SENTRY
#define SERVER_HPP_SENTRY
#include "FileTransferhandler.hpp"

enum
{
    listen_count = 16
};

class FdHandler
{
    int fd;
    bool want_read;
    bool want_write;

public:
    FdHandler(int _fd);
    virtual ~FdHandler();

    int GetFd();
    bool WantRead();
    bool WantWrite();
    void SetRead(bool op);
    void SetWrite(bool op);

    virtual void Handle(bool r, bool w) = 0;
};

class EventSelector
{
    FdHandler **fd_array;
    int fd_array_len;
    int max_fd;
    bool quit_flag;

public:
    EventSelector();
    ~EventSelector();

    void Add(FdHandler *h);
    void Remove(FdHandler *h);

    void Run();
};

////////////////////////////////

class Session;

class Server : FdHandler
{
    EventSelector *the_selector;
    struct item
    {
        Session *sess;
        item *next;
    };
    item *first;
    Server(int _fd, EventSelector *_the_selector);

public:
    virtual ~Server();
    static Server *Start(int port, EventSelector *_the_selector);
    void RemoveSession(Session *s);

private:
    virtual void Handle(bool r, bool w);
};

class Session : FdHandler
{
    friend class Server;
    char file_name[max_file_name];
    int file_name_used;

    FileSendHandler *f;
    Server *the_master;

public:
    Session(int _fd, Server *_the_master);
    virtual ~Session();

private:
    void Send(const char *msg);
    virtual void Handle(bool r, bool w);
};

#endif