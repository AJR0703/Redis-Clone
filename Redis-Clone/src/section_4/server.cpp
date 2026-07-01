#include <assert.h>
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

const size_t k_max_msg = 4096;

/**
 * Fixes issue with partial reading of messages.
 * Retrieves n bytes and stores them in the buffer.
 *
 * @param fd socket handle
 * @param buf location to read bytes from.
 * @param n number of bytes to read.
 * @return
 */
static int32_t read_full(int fd, char *buf, size_t n) {
    // continues to call read until n = 0;
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        // decrement n by the number of bytes read.
        n -= (size_t)rv;
        // point the buffer to the number of bytes read to prevent overwriting.
        buf += rv;
    }
    return 0;
}

/**
 * Fixes issue with partial writing of messages.
 * Writes n bytes to the write buffer.
 *
 * @param fd socket handle
 * @param buf location to write bytes to.
 * @param n number of bytes to read.
 * @return
 */
static int32_t write_all(int fd, const char *buf, size_t n) {
    // continues to call write until n = 0;
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        // decrement n by the number of bytes read.
        n -= (size_t)rv;
        // point the buffer to the number of bytes read to prevent overwriting.
        buf += rv;
    }
    return 0;
}

/**
* Process each client connection, reading incoming messages and sending a reply.
*
* @param connfd socket handle
* @return
*/
static int32_t one_request(int connfd) {
    // 4 byte int for length plus length of message.
    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return -1;
    }

    // extract 4 byte int length from read buffer.
    uint32_t len = 0;
    memcpy(&len, rbuf, 4);
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }

    // print contents
    fprintf(stderr, "client says: %.*s\n", len, &rbuf[4]);

    // write the length of the reply string and the reply to the write buffer and write to client.
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)sizeof(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);
    return write_all(connfd, wbuf, 4 + len);
}

int main() {
    int fd;
    createSocket(fd);

    // this is needed for most server applications
    int val = 1;
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

        while (true) {
            // serve one client connection at a time.
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }
        close(connfd);
    }

    return 0;
}