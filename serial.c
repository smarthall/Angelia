#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

int openserial() {
    int fd;
    struct termios local_term;

    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

    tcgetattr(fd, &local_term);
    // Change the settings to match the local XBee
    local_term.c_iflag = IGNBRK;
    local_term.c_oflag = 0;
    local_term.c_lflag = 0;
    local_term.c_cflag = (CS8 | CREAD | CLOCAL);
    local_term.c_cc[VMIN]  = 1;
    local_term.c_cc[VTIME] = 0;
    cfsetospeed(&local_term, B115200);
    cfsetispeed(&local_term, B115200);

    tcsetattr(fd, TCSANOW, &local_term);
    fcntl(fd, F_SETFD, O_NONBLOCK);
}

int closeserial(int fd) {
    close(fd);
}

