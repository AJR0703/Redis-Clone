#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

/**
 * client.cpp from Section 3.
 *
 */


static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

/**
 * Creates an IPv4 TCP Socket.
 */
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
 *  Configuring network address sockaddr_in (Socket Address Internet).
 */
static sockaddr_in configAddr(sockaddr_in &addr) {
    addr.sin_family = AF_INET;  // Internet address family: IPv4
    addr.sin_port = ntohs(1234); // Assign port num ntohs -> network to host short

    // socket IP address set to localhost, INADDR_LOOPBACK = (127.0.0.1)
    // ntohl (Network to Host Long)
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
}


int main() {

    int fd;
    createSocket(fd);

    // create struct of sockaddr_in (Socket Address Internet).
    struct sockaddr_in addr = {};
    configAddr(addr);

    // connect initiates tcp handshake.
    // need to cast addr to sockaddr * pointer type for connect function.
    // sizeof(addr), telling system how much memory to read in bytes.
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("connect");
    }

    // write char array msg to server.
    // tell the system how many bytes to send.
    char msg[] = "hello";
    write(fd, msg, strlen(msg));


    // init a read buffer of 64 bytes, set all bytes to null chars (0).
    char rbuf[64] = {};
    // sizeof(rbuf) -1 (63 bytes) used to leave last byte as null terminator, allows for safe reading and printing.
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        die("read");
    }

    printf("server says: %s\n", rbuf);
    close(fd);
    return 0;

}