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

/**
 * Append given number of bytes to the specified buffer.
 *
 * @param buf buffer to write bytes to.
 * @param data bytes to write.
 * @param len length of data.
 */
static void buf_append(std::vector<uint8_t> &buf, const uint8_t *data, size_t len) {
    buf.insert(buf.end(), data, data + len);
}


const size_t k_max_msg = 32 << 20;


/**
 * Construct message to send to the server.
 *
 * @param fd socket handle for the socket.
 * @param text memory address for the payload.
 * @param len length of the text in bytes.
 *
 * @return return -1 if message too long, result of write_all function.
 */
static int32_t send_req(int fd, const uint8_t *text, size_t len) {
    if (len > k_max_msg) {
        return -1;
    }

    std::vector<uint8_t> wbuf;
    buf_append(wbuf, (const uint8_t *) &len, 4);
    buf_append(wbuf, text, len);
    return write_all(fd, wbuf.data(), wbuf.size());
}


/**
 * Constructs message received from the server.
 * Extracts message length then payload.
 *
 * @param fd socket handle for socket.
 * @return err from read_full or 0 on successful read.
 */
static int32_t read_res(int fd) {
    std::vector<uint8_t> rbuf;
    rbuf.resize(4);
    errno = 0;

    int32_t err = read_full(fd, &rbuf[0], 4);
    if (err) {
        if (errno == 0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf.data(), 4);
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }

    rbuf.resize(4 + len);
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    printf("len:%u data:%.*s\n", len, len < 100 ? len : 100, &rbuf[4]);
    return 0;
}


/**
 * connect to and send/receive messages to the server.
 *
 * @return
 */
int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
    int rv = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
    if (rv) {
        die("connect");
    }

    std::vector<std::string> query_list = {
        "hello1", "hello2", "hello3",

        std::string(k_max_msg, 'z'),
        "hello5",
    };
    for (const std::string &s: query_list) {
        int32_t err = send_req(fd, (uint8_t *) s.data(), s.size());
        if (err) {
            goto L_DONE;
        }
    }
    for (size_t i = 0; i < query_list.size(); ++i) {
        int32_t err = read_res(fd);
        if (err) {
            goto L_DONE;
        }
    }

    L_DONE:
        close(fd);
    return 0;
}