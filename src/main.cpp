#include "Server.hpp"
#include <stdio.h>

static int port = 8888;

int main() {
    EventSelector *sel = new EventSelector();
    if (sel == 0)
    {
        perror("selector");
        return 1;
    }
    
    Server *ser = Server::Start(port, sel);
    if (ser == 0)
    {
        perror("server");
        return 2;
    }
    
    sel->Run();
    return 0;
}