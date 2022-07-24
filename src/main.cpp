#include "Server.hpp"
#include <stdio.h>

static int port = 8808;

int main()
{
    EventSelector *sel = new EventSelector();
    Server *serv = Server::Start(port, sel);
    if (!serv)
    {
        perror("error");
        return 1;
    }
    sel->Run();
    return 0;
}