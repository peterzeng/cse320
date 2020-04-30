#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <getopt.h>

#include "pbx.h"
#include "server.h"
#include "debug.h"

#include "csapp.h"

static void terminate(int status);

volatile sig_atomic_t got_sighup = 0;

void sighup_handler(int sig){
    got_sighup = 1;
    pbx_shutdown(pbx);
}
/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    int option;
    int port;
    int index;

    while((option = getopt(argc, argv, "p:")) != EOF) {
        switch(option) {
            case 'p':
                for (int i = 0; i < argc; i++){
                    if (!(strcmp(argv[i], "-p"))){
                        index = i + 1;
                    }
                    // debug("%s", argv[i]);
                }


                if ((port = atoi(optarg++)) <= 0) {
                    fprintf(stderr, "Usage: bin/pbx -p <port>");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: bin/pbx -p <port>");
                exit(EXIT_FAILURE);

        }
    }

    debug("Port %d", port);
    struct sigaction new_action, old_action;

    new_action.sa_handler = sighup_handler;
    sigemptyset(&new_action.sa_mask);
    sigaddset(&new_action.sa_mask, SIGHUP);
    new_action.sa_flags = 0;

    if (sigaction(SIGHUP, &new_action, &old_action) == -1){
        terminate(EXIT_FAILURE);
    }

    // Signal(SIGHUP, sighup_handler);

    // Perform required initialization of the PBX module.

    debug("Initializing PBX...");
    if ((pbx = pbx_init()) == NULL){
        terminate(EXIT_FAILURE);
    }

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = Open_listenfd(argv[index]);

    while (1){
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, pbx_client_service, connfdp);
    }


    fprintf(stderr, "You have to finish implementing main() "
	    "before the PBX server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}
