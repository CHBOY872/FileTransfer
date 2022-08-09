#include "Server.hpp"
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
#include <netdb.h>

enum
{
    max_array_len = 15
};

FdHandler::~FdHandler() {}

/////////////////////////////

EventSelector::~EventSelector()
{
    if (fd_array)
        delete fd_array;
}

void EventSelector::Add(FdHandler *h)
{
    int fd = h->GetFd();
    int i;
    if (!fd_array)
    {
        fd_array_len =
            fd > max_array_len - 1 ? fd + 1 : max_array_len;
        fd_array = new FdHandler *[fd_array_len];
        for (i = 0; i < fd_array_len; i++)
            fd_array[i] = 0;
        max_fd = -1;
    }
    if (fd_array_len <= fd)
    {
        FdHandler **temp = new FdHandler *[fd + 1];
        for (i = 0; i <= fd; i++)
            temp[i] = i < fd_array_len ? fd_array[i] : 0;
        fd_array_len = fd + 1;
        delete[] fd_array;
        fd_array = temp;
    }
    if (fd > max_fd)
        max_fd = fd;
    fd_array[fd] = h;
}

void EventSelector::Remove(FdHandler *h)
{
    int fd = h->GetFd();
    if (fd_array[fd] || fd_array[fd] != h)
        return;
    fd_array[fd] = 0;
    if (fd == max_fd)
    {
        while (max_fd >= 0 && !fd_array[max_fd])
            max_fd--;
    }
}

void EventSelector::Run()
{
    fd_set rds, wrs;
    int i;
    for (;;)
    {
        FD_ZERO(&rds);
        FD_ZERO(&wrs);
        for (i = 0; i <= max_fd; i++)
        {
            if (fd_array[i])
            {
                if (fd_array[i]->WantRead())
                    FD_SET(i, &rds);
                if (fd_array[i]->WantWrite())
                    FD_SET(i, &wrs);
            }
        }
        int stat = select(max_fd + 1, &rds, &wrs, 0, 0);
        if (stat <= 0)
            break;
        for (i = 0; i <= max_fd; i++)
        {
            if (fd_array[i])
            {
                bool r = FD_ISSET(i, &rds);
                bool w = FD_ISSET(i, &wrs);
                fd_array[i]->Handle(r, w);
            }
        }
    }
}

////////////////////////////

Server::Server(int _fd, EventSelector *_the_selector)
    : FdHandler(_fd), the_selector(_the_selector), first(0)
{
    _the_selector->Add(this);
}

Server::~Server()
{
    delete the_selector;
    while (first)
    {
        item *tmp = first->next;
        delete first;
        first = tmp;
    }
}

Server *Server::Start(int port, EventSelector *_the_selector)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
        return 0;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (-1 == bind(fd, (sockaddr *)&addr, sizeof(addr)))
        return 0;
    if (-1 == listen(fd, max_array_len))
        return 0;
    return new Server(fd, _the_selector);
}

void Server::RemoveClient(Client *h)
{
    item **p;
    for (p = &first; *p; p = &(first->next))
    {
        if ((*p)->cl == h)
        {
            item *tmp = *p;
            *p = tmp->next;
            the_selector->Remove(tmp->cl);
            delete tmp;
            return;
        }
    }
}

void Server::Handle(bool r, bool w)
{
    if (r)
    {
        sockaddr_in addr_client;
        socklen_t len = 0;
        int _fd = accept(GetFd(), (sockaddr *)&addr_client, &len);
        if (-1 == _fd)
            return;
        item *tmp = new item(new Client(_fd, this), first);
        first = tmp;
        the_selector->Add(tmp->cl);
    }
}

///////////////////////

Client::Client(int _fd, Server *_the_master)
    : FdHandler(_fd), st(not_started),
      the_master(_the_master), f(0)
{
    memset(buffer, 0, buf_len);
}

void Client::Handle(bool r, bool w)
{
    if (r)
    {
        int i;
        int rc = read(GetFd(), buffer, buf_len);
        if (rc <= 0)
        {
            the_master->RemoveClient(this);
            return;
        }

        switch (st)
        {
        case not_started:
            for (i = 0; i < rc; i++)
            {
                if (buffer[i] == '\r')
                {
                    buffer[i] = 0;
                    break;
                }
            }
            f = FileSender::Make(buffer, GetFd());
            if (!f)
            {
                the_master->RemoveClient(this);
                return;
            }
            st = started;

        case started:
            if (!f->Send())
                the_master->RemoveClient(this);
            break;
        }
        memset(buffer, 0, rc);
    }
}