#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <vector>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void msg_errno(const char *msg) {
    fprintf(stderr, "[errno:%d] %s\n", errno, msg);
}

static void die(const char *msg) {
    fprintf(stderr, "[%d] %s\n", errno, msg);
    abort();
}

/**
 * Set the socket to non-blocking mode.
 * @param fd socket handle.
 *
 */
static void fd_set_nb(int fd) {
    errno = 0;
    // fcntl is used to manipulate the properties of the file descriptor.
    // F_GETFL: Get file status flags (File_GetFlags)
    int flags = fcntl(fd, F_GETFL, 0);
    if (errno) {
        die("fcntl error: error retrieving flags.");
        return;
    }

    // |= is the bitwise OR, used instead of addition for the case that O_NONBLOCK might already be set (1000 + O_NONBLOCK would cause issues)
    // flags represent individual bits packed into an integer.
    // O_NONBLOCK = 0x0800 (1000 0000 0000), the 1 is important here, as it represents O_NONBLOCK.
    // The bitwise OR simply makes it so that any existing flags (individual bits) already set, will remain while this flag is also added.
    //
    flags |= O_NONBLOCK;

    errno = 0;
    // set the flags for the file descriptor.
    (void)fcntl(fd, F_SETFL, flags);
    if (errno) {
        die("fcntl error: error setting flags");
    }
}