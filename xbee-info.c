#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "serial.h"

int main(int argc, char **argv) {
    int sfd;

    sfd = openserial();

    closeserial(sfd);

    return EXIT_SUCCESS;
}

