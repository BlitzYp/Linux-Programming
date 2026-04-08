#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main(void)
{
    int sock[2];
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sock);
    if (ret) {
    	perror("socketpair");
    	return 1;
    }

    // fork
    int f=fork();
    if (f<0) {
        perror("Fork failed");
        return 1;
    }
    // in parent code: write to socket, wait for reply, read it, and output it
    if (f>0) {
        close(sock[1]);
        char* req="40 456\n";
        char reply[64];

        if (write(sock[0],req,strlen(req)) < 0) {
            perror("write");
            close(sock[0]);
            wait(NULL);
            return 1;
        }

        shutdown(sock[0], SHUT_WR);
        ssize_t bytes=read(sock[0],reply,sizeof(reply)-1);
        if (bytes<0) {
            perror("read");
            close(sock[0]);
            wait(NULL);
            return 1;
        }

        reply[bytes]='\0';
        printf("%s\n", reply);

        close(sock[0]);
        wait(NULL);
    }
    // in child code: read from socket, process arguments, write reply, exit
    else {
        close(sock[0]);
        char buffer[64],reply[64];
        int a,b,sum;
        ssize_t bytes=read(sock[1],buffer,sizeof(buffer)-1);
        if (bytes<0) {
            perror("read");
            close(sock[1]);
            return 1;
        }

        buffer[bytes]='\0';
        if (sscanf(buffer,"%d %d",&a,&b)!=2) {
            fprintf(stderr,"Invalid input: %s\n",buffer);
            close(sock[1]);
            return 1;
        }

        int s=a+b;
        snprintf(reply,sizeof(reply),"%d",s);
        if (write(sock[1],reply,strlen(reply))<0) {
            perror("write");
            close(sock[1]);
            return 1;
        }
        close(sock[1]);
    }
    return 0;
}
