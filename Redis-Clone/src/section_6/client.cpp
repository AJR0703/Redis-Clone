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
#include <string>
#include <vector>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

/**
 * Read the entire buffer from the socket.
 *
 * @param fd socket handle for socket.
 * @param buf buffer to read from.
 * @param n number of bytes ot read.
 *
 * @return -1 for error, 0 if successful full read from buffer.
 */
static int32_t read_full(int fd, uint8_t *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t) rv;
        buf += rv;
    }
    return 0;
}

/**
 * Write all bytes from the memory address to the buffer to the server.
 *
 * @param fd socket handle for socket.
 * @param buf location in memory for the buffer.
 * @param n number of bytes to write.
 *
 * @return -1 for error, 0 if successful full write.
 */
static int32_t write_all(int fd, const uint8_t *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t) rv;
        buf += rv;
    }
    return 0;
}
