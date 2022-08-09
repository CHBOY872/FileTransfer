#ifndef SERVER_HPP_SENTRY
#define SERVER_HPP_SENTRY

#include "FileHandler.hpp"

enum
{
    buf_len = 100
};

class FdHandler
{
    int fd;
    bool want_write;
    bool want_read;

public:
    FdHandler(int _fd, bool _want_write = false, bool _want_read = true)
        : fd(_fd), want_write(_want_write), want_read(_want_read) {}
    ~FdHandler();

    int GetFd() { return fd; }
    bool WantWrite() { return want_write; }
    bool WantRead() { return want_read; }
    void SetRead(bool r) { want_read = r; }
    void SetWrite(bool w) { want_write = w; }
    virtual void Handle(bool r, bool w) = 0;
};

class EventSelector
{
    FdHandler **fd_array;
    int max_fd;
    int fd_array_len;

public:
    EventSelector() : fd_array(0), max_fd(-1), fd_array_len(0) {}
    ~EventSelector();

    void Add(FdHandler *h);
    void Remove(FdHandler *h);

    void Run();
};

////////////////////////

class Client;
class Server : public FdHandler
{
    struct item
    {
        Client *cl;
        item *next;
        item(Client *_cl, item *_next = 0) : cl(_cl), next(_next) {}
    };
    EventSelector *the_selector;
    item *first;
    Server(int _fd, EventSelector *_the_selector);

public:
    ~Server();
    static Server *Start(int port, EventSelector *_the_selector);
    void RemoveClient(Client *h);

private:
    virtual void Handle(bool r, bool w);
};

class Client : public FdHandler
{
    friend class Server;
    enum stat {
        not_started = 1,
        started
    };
    stat st;
    Server *the_master;
    FileSender *f;
    char buffer[buf_len];
    Client(int _fd, Server *_the_master);

private:
    virtual void Handle(bool r, bool w);
};

#endif