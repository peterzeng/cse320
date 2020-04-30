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

/*
 * Thread function for the thread that handles a particular client.
 *
 * @param  Pointer to a variable that holds the file descriptor for
 * the client connection.  This variable must be freed once the file
 * descriptor has been retrieved.
 * @return  NULL
 *
 * This function executes a "service loop" that receives messages from
 * the client and dispatches to appropriate functions to carry out
 * the client's requests.  The service loop ends when the network connection
 * shuts down and EOF is seen.  This could occur either as a result of the
 * client explicitly closing the connection, a timeout in the network causing
 * the connection to be closed, or the main thread of the server shutting
 * down the connection as part of graceful termination.
 */

void *pbx_client_service(void *arg){

    int client_fd = *(int*)arg;
    free(arg);
    debug("client fd: %d", client_fd);

    // detach so it doesn't have to be explicity reaped

    pthread_detach(pthread_self());

    // register the client file descriptor with pbx module

    TU *client = pbx_register(pbx, client_fd);

    debug("client pointer: %p", client);
    printf("client pointer %p", client);




    return NULL;
}