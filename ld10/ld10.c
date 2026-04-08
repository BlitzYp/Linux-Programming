#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sock);
    if (ret) {
	perror("socketpair");
	return 1;
    }


    // fork

    // in parent code: write to socket, wait for reply, read it, and output it


    // in child code: read from socket, process arguments, write reply, exit

    return 0;
}
