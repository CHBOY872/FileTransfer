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
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

FdHandler::FdHandler(int _fd)
    : fd(_fd), want_read(true), want_write(false) {}
FdHandler::~FdHandler()
{
    close(fd);
}

int FdHandler::GetFd() { return fd; }
bool FdHandler::WantRead() { return want_read; }
bool FdHandler::WantWrite() { return want_write; }
void FdHandler::SetRead(bool op) { want_read = op; }
void FdHandler::SetWrite(bool op) { want_write = op; }

////////////////////////////////////////////////////////////

EventSelector::EventSelector()
    : fd_array(0), fd_array_len(0), quit_flag(false) {}

EventSelector::~EventSelector()
{
    delete[] fd_array;
}

void EventSelector::Add(FdHandler *h)
{
    int i;
    int fd = h->GetFd();
    if (!fd_array)
    {
        fd_array_len =
            fd > listen_count - 1 ? fd + 1 : listen_count;
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
    if (fd > fd_array_len || h != fd_array[fd])
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
    int i, stat;
    do
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
        stat = select(max_fd + 1, &rds, &wrs, 0, 0);
        if (stat <= 0)
        {
            quit_flag = true;
            return;
        }
        for (i = 0; i <= max_fd; i++)
        {
            if (fd_array[i])
            {
                bool r = FD_ISSET(fd_array[i]->GetFd(), &rds);
                bool w = FD_ISSET(fd_array[i]->GetFd(), &wrs);
                if (r || w)
                    fd_array[i]->Handle(r, w);
            }
        }

    } while (!quit_flag);
}

////////////////////////////////////////////////////////////

Server::Server(int _fd, EventSelector *_the_selector)
    : FdHandler(_fd), the_selector(_the_selector), first(0)
{
    the_selector->Add(this);
}

Server::~Server()
{
    item *tmp;
    while (first)
    {
        tmp = first->next;
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
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) == -1)
        return 0;
    if (listen(fd, listen_count) == -1)
        return 0;
    return new Server(fd, _the_selector);
}

void Server::RemoveSession(Session *s)
{
    the_selector->Remove(s);
    item **p;
    for (p = &first; *p; p = &((*p)->next))
    {
        if ((*p)->sess == s)
        {
            item *tmp = *p;
            *p = (*p)->next;
            delete tmp->sess;
            delete tmp;
            return;
        }
    }
}

void Server::Handle(bool r, bool w)
{
    sockaddr_in addr_client;
    socklen_t len = 0;
    int client_fd = accept(GetFd(), (sockaddr *)&addr_client, &len);
    if (client_fd == -1)
        return;
    item *tmp = new item;
    tmp->sess = new Session(client_fd, this);
    tmp->next = first;
    first = tmp;
    the_selector->Add(first->sess);
    // int flags = fcntl(client_fd, F_GETFL);
    // fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
}

////////////////////////////////////////////////////////////

Session::Session(int _fd, Server *_the_master)
    : FdHandler(_fd), file_name_used(0),
      f(0), the_master(_the_master)
{
    memset(file_name, 0, max_file_name);
}

Session::~Session()
{
    if (f)
        delete f;
}

void Session::Handle(bool r, bool w)
{
    if (r)
    {
        int rc =
            read(GetFd(), file_name, max_file_name - file_name_used);
        if (rc <= 0 || file_name_used + rc > max_file_name)
        {
            the_master->RemoveSession(this);
            return;
        }
        int i;
        for (i = 0; i < rc; i++)
        {
            if (file_name[i] == '\r')
            {
                file_name[i] = 0;
                break;
            }
        }
        f = FileSendHandler::Start(GetFd(), file_name);
        if (!f)
        {
            Send("No such file or something went wrong...\n");
            the_master->RemoveSession(this);
            return;
        }

        SetWrite(true);
    }
    if (w)
    {
        if (!f->Send())
        {
            SetWrite(false);
            return;
        }
    }
}

void Session::Send(const char *msg)
{
    write(GetFd(), msg, strlen(msg));
}