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
 *  Configuring network address sockaddr_in (Socket Address Internet).
 */
static sockaddr_in configAddr(sockaddr_in &addr) {
    addr.sin_family = AF_INET;  // Internet address family: IPv4
    addr.sin_port = ntohs(1234); // Assign port num ntohs -> network to host short

    // socket IP address set to localhost, INADDR_LOOPBACK = (127.0.0.1)
    // ntohl (Network to Host Long)
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
}

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

const size_t k_max_msg = 4096;

/**
 *
 *
 * @param fd
 * @param text
 * @return
 */
static int32_t query(int fd, const char *text) {
    // Retrieve the 4 byte length int.
    uint32_t len = (uint32_t)strlen(text);
    if (len > k_max_msg) {
        return -1;
    }

    // assign 4 byte int + string size to wbuf
    char wbuf[4 + k_max_msg];
    // copy length int and text to write buffer.
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);
    // write the write buffer to the server.
    if (int32_t err = write_all(fd, wbuf, 4 + len)) {
        return err;
    }

    // 4 bytes header
    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    memcpy(&len, rbuf, 4);  // assume little endian
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    printf("server says: %.*s\n", len, &rbuf[4]);
    return 0;
}

int main() {
    int fd;
    createSocket(fd);

    struct sockaddr_in addr = {};
    configAddr(addr);
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) die("connect");

    // send multiple requests
    int32_t err = query(fd, "hello1");
    if (err) {
        goto L_DONE;
    }
    err = query(fd, "hello2");
    if (err) {
        goto L_DONE;
    }
    err = query(fd, "hello3");
    if (err) {
        goto L_DONE;
    }

L_DONE:
    close(fd);
    return 0;
}