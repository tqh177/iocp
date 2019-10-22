#include "sock/iocp.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

void shut_down(int sig){
    exit(0);
}

int main(int argc, char const *argv[])
{
    config *con = getConfig();
    signal(SIGINT,shut_down);
    initWsa();
    printf("¿ªÊ¼¼àÌý%s:%d\n",con->ip,con->port);
    startServer(con->ip,con->port);
    return 0;
}
