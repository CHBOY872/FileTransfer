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

enum
{
    max_file_name = 1024,
    send_size_buf = 512
};

static int port = 8808;

int socket_connect(int port, const char *ip)
{
    int socket_client = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    int socket_server;
    struct sockaddr_in server_addr, addr_client;

    if (-1 == socket_client)
        return -1;
    setsockopt(socket_client, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    addr_client.sin_addr.s_addr = htons(INADDR_ANY);
    addr_client.sin_port = htons(port);
    addr_client.sin_family = AF_INET;
    bind(socket_client, (struct sockaddr *)&addr_client, sizeof(addr_client));
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    socket_server = connect(socket_client, (struct sockaddr *)&server_addr,
                            sizeof(server_addr));
    if (-1 == socket_server)
        return -1;
    return socket_client;
}

void send_request(int fd, const char *file)
{
    write(fd, file, strlen(file));
}

void write_file(int fd, const char *file_name)
{
    int fd_file = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (-1 == fd_file)
    {
        perror("handle");
        return;
    }
    char buf[send_size_buf];
    int stat = read(fd, buf, send_size_buf);

    do
    {
        memset(buf, 0, send_size_buf);
        stat = read(fd, buf, send_size_buf);
        write(fd_file, buf, stat);
    } while (stat == send_size_buf);
    close(fd_file);
    close(fd);
}

int main(int argc, const char **argv)
{
    if (argc != 3)
    {
        printf("usage: <to ip connect> <file to receive>\n");
        return 1;
    }
    int socket_client = socket_connect(port, argv[1]);
    if (-1 == socket_client)
    {
        perror("socket");
        return 1;
    }
    send_request(socket_client, argv[2]);
    write_file(socket_client, argv[2]);
    return 0;
}