#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void do_something(int connfd) {
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        msg("read() error");
        return;
    }
    fprintf(stderr, "client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

static void createSocket(int &fd) {
    // File descriptor = socket handle.
    // AF_INET (Address Family: Internet) = IPv4.
    // SOCK_STREAM (TCP)
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }
}

/**
 *  Configures network address sockaddr_in (Socket Address Internet).
 *  Binds the address to the socket.
 */
static int bindAddr(sockaddr_in &addr, int &fd) {
    addr.sin_family = AF_INET;  // Internet address family: IPv4
    addr.sin_port = ntohs(1234); // Assign port num ntohs -> network to host short

    // socket IP address set to localhost, INADDR_LOOPBACK = (127.0.0.1)
    // ntohl (Network to Host Long)
    addr.sin_addr.s_addr = ntohl(0);

    // rv = 0 on success, -1 on error.
    int rv = bind(fd,(const struct sockaddr *)&addr, sizeof(addr));

    return rv;
}

int main() {
    int fd;
    createSocket(fd);

    int val = 1;
    // SOL_SOCKET: Generic socket option
    // SO_REUSEADDR: Allows for address to be reused on restart.
    // val used to turn SO_REUSEADDR on.
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr;
    int rv = bindAddr(addr, fd);
    if (rv < 0) die("bind");

    // SOMAXCONN = Max num of connections waiting for TCP handshake, (Max num that the OS allows).
    rv = listen(fd, SOMAXCONN);
    if (rv) die("listen");

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        // pass empty struct to be filled with the client address.
        // pass reference to address length for modification in accept function.
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue;
        }

        do_something(connfd);
        close(connfd);
    }

    return 0;
}

