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
#include <fcntl.h>

#define READ_BUF 1024

static int port = 8888;

int main(int argc, const char **argv)
{
    if (argc != 3)
    {
        printf("Usage: <ip> <file>\n");
        return 1;
    }
    char buffer[BUFSIZ];
    memset(buffer, 0, BUFSIZ);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (-1 == fd)
    {
        perror("socket");
        return 2;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    int stat = connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == stat)
    {
        perror("connect");
        return 3;
    }
    write(fd, argv[2], strlen(argv[2]));
    int rc;
    int to_fd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if (-1 == to_fd)
    {
        perror("file");
        return 4;
    }

    do
    {
        rc = read(fd, buffer, BUFSIZ);
        write(fd, "1", 2);
        if (rc == 0)
            break;
        write(to_fd, buffer, rc);
        memset(buffer, 0, rc);
    } while (rc == READ_BUF);

    close(fd);
    close(to_fd);
    return 0;
}