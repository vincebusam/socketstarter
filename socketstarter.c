#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <netinet/in.h>

#define BUFLEN 1024

int recvsend(int from_fd, int to_fd) {
    int cnt;
    char buf[BUFLEN];
    ioctl(from_fd, FIONREAD, &cnt);
    if (cnt > 0) {
        cnt = MIN(cnt, BUFLEN);
        recv(from_fd, &buf, cnt, 0);
        send(to_fd, &buf, cnt, MSG_DONTWAIT);
    }
    return cnt;
}

int main(int argc, char *argv[]) {
    int server_fd, sock, client, reuseaddr=1, rv, timeout, shutdown;
    time_t start_time;
    struct sockaddr_in addr;
    fd_set rfds;
    struct timeval tv;

    timeout = atoi(getenv("TIMEOUT"));
    shutdown = atoi(getenv("SHUTDOWN"));

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(getenv("PORT")));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) {
        perror("socket()");
        exit(1);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0) {
        perror("SO_REUSEADDR");
        exit(1);
    }
    if (bind(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind()");
        exit(1);
    }
    if (listen(server_fd, 1) < 0) {
        perror("listen()");
        exit(1);
    }
    sock = accept(server_fd, NULL, NULL);
    if (sock < 0) {
        perror("accept()");
        exit(1);
    }
    close(server_fd);
    start_time = time(NULL);
    system(getenv("START_COMMAND"));

    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client < 0) {
        perror("socket()");
        exit(1);
    }
    while (connect(client, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        sleep(1);
        if ((time(NULL) - start_time) >= timeout)
            exit(1);
    }    

    while(1) {
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);
        FD_SET(client, &rfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        rv = select(MAX(sock, client)+1, &rfds, NULL, NULL, &tv);
        if (rv == -1) {
            perror("select()");
            break;
        }
        if (!rv) {
            continue;
        }
        if (FD_ISSET(sock, &rfds))
            if (recvsend(sock, client) < 1)
                break;
        if (FD_ISSET(client, &rfds))
            if (recvsend(client, sock) < 1)
                break;
    }

    if ((time(NULL) - start_time) < shutdown) {
        printf("Sleep %li secs until shutdown\n", shutdown - (time(NULL) - start_time));
        sleep(shutdown - (time(NULL) - start_time));
        
    }

    close(sock);
    close(client);

    system(getenv("STOP_COMMAND"));
    return 0;
}
