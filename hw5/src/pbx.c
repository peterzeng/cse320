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

sem_t mutex;

char* state_to_string(TU_STATE state){
    if (state == TU_ON_HOOK){
        return "ON HOOK";
    } else if (state == TU_RINGING){
        return "RINGING";
    } else if (state == TU_DIAL_TONE){
        return "DIAL TONE";
    } else if (state == TU_RING_BACK){
        return "RING BACK";
    } else if (state == TU_BUSY_SIGNAL){
        return "BUSY SIGNAL";
    } else if (state == TU_CONNECTED){
        return "CONNECTED";
    } else if (state == TU_ERROR){
        return "ERROR";
    }
    return NULL;
}

struct tu {
    volatile int fd;
    volatile TU_STATE state;
    volatile int caller;
    volatile int called;
    volatile int connected;
    // volatile int state;
};

void reset_tu(TU* tu){
    tu->caller = 0;
    tu->called = 0;
    tu->connected = 0;
}

struct pbx {
    struct tu* tunits[PBX_MAX_EXTENSIONS];
};

PBX *pbx_init(){
    pbx = malloc(sizeof(struct pbx));
    for (int i = 0; i < PBX_MAX_EXTENSIONS; i++){
        pbx->tunits[i] = NULL;
    }

    if (sem_init(&mutex, 0, 1) < 0){
        debug("error in mutex init");
    }

    return pbx;
}

void pbx_shutdown(PBX *pbx){
    // free(pbx);
    // shutdown registered threads
    for (int q = 0; q < PBX_MAX_EXTENSIONS; q++){
        if (pbx->tunits[q] != NULL){
            int fd = pbx->tunits[q]->fd;
            if (shutdown(fd, SHUT_RDWR) == -1){
                debug("error in shutdown");
            }
        }
    }

    int check = 1;
    while (1){
        for (int i = 0; i < PBX_MAX_EXTENSIONS; i++){
            if (pbx->tunits[i] != NULL)
                check = 0;
        }
        if (check)
            break;
    }
    debug("shutdown successful");
    free(pbx);
}

TU *pbx_register(PBX *pbx, int fd){
    // debug("register fd: %d",fd);
    P(&mutex);
    TU *tu = malloc(sizeof(struct tu));
    if (tu == NULL){
        return NULL;
    }
    tu->fd = fd;
    tu->state = TU_ON_HOOK;
    pbx->tunits[fd] = tu;
    V(&mutex);

    char *c = state_to_string(tu->state);
    dprintf(fd, "%s %d\r\n", c, fd);
    return pbx->tunits[fd];
}

int pbx_unregister(PBX* pbx, TU *tu){
    // debug("unregister");
    if (pbx->tunits[tu->fd] == NULL){
        return -1;
    }
    if (close(tu->fd) == -1)
        debug("close didn't work properly");

    pbx->tunits[tu->fd] = NULL;
    free(tu);
    return 0;
}

int tu_fileno(TU *tu){
    if (tu != NULL){
        return tu->fd;
    } else {
        return -1;
    }
}

int tu_extensions(TU *tu){
    if (tu != NULL){
        return tu->fd;
    } else {
        return -1;
    }
}

int tu_pickup(TU *tu){
    // debug("pickup tu->fd: %d", tu->fd);
    P(&mutex);
    if (tu->state == TU_ON_HOOK){
        // debug("pickup, state->ON_HOOK");
        tu->state = TU_DIAL_TONE;
        char *c = state_to_string(tu->state);
        dprintf(tu->fd, "%s\r\n", c);
    } else if (tu->state == TU_RINGING){
        // debug("pickup ringing");
        // PEER TU TRANSITIONS TO CONNECTED
        // SET BOTH TO CONNECTED
        if (tu->caller){
            tu->connected = tu->caller;
            pbx->tunits[tu->caller]->connected = tu->fd;

            tu->state = TU_CONNECTED;
            char *c = state_to_string(tu->state);
            dprintf(tu->fd, "%s %d\r\n", c, tu->caller);

            pbx->tunits[tu->caller]->state = TU_CONNECTED;
            dprintf(tu->caller, "%s %d\r\n", c, tu->fd);
        } else {
            // debug("no caller but ringing");
            return -1;
        }

    } else {
        // debug("pickup not ringing or on hook");
        char *c = state_to_string(tu->state);
        dprintf(tu->fd, "%s\r\n", c);
    }
    V(&mutex);

    return 0;
}

int tu_hangup(TU *tu){
    // debug("hangup tu->fd: %d", tu->fd);
    P(&mutex);
    if (tu->state == TU_CONNECTED){
        tu->state = TU_ON_HOOK;
        char *c = state_to_string(tu->state);
        dprintf(tu->fd, "%s %d\r\n", c, tu->fd);
        // TO BE IMPLEMENTED: PEER TU TRANSITIONS TO DIAL TONE

        if (tu->connected){
            // SET BOTH TO DISCONNECTED
            pbx->tunits[tu->connected]->state = TU_DIAL_TONE;

            char* c = state_to_string(pbx->tunits[tu->connected]->state);
            dprintf(tu->connected, "%s %d\r\n", c, tu->connected);
            reset_tu(tu);
            reset_tu(pbx->tunits[tu->connected]);
        } else {
            debug("tu->connected false");
            V(&mutex);
            return -1;
        }

    } else if (tu->state == TU_RING_BACK){
        tu->state = TU_ON_HOOK;
        char *c = state_to_string(tu->state);
        dprintf(tu->fd, "%s %d\r\n", c, tu->fd);
        // TO BE IMPLEMENTED: CALLER TU TRANSITIONS TO ON HOOK
        if (tu->called){
            pbx->tunits[tu->called]->state = TU_ON_HOOK;
            char* c = state_to_string(pbx->tunits[tu->called]->state);
            dprintf(tu->called, "%s %d\r\n", c, tu->called);
            reset_tu(tu);
            reset_tu(pbx->tunits[tu->connected]);
        } else {
            debug("tu->called false");
            V(&mutex);
            return -1;
        }
    } else if (tu->state == TU_RINGING){
        tu->state = TU_ON_HOOK;
        char *c = state_to_string(tu->state);
        dprintf(tu->fd, "%s %d\r\n", c, tu->fd);
        // TO BE IMPLEMENTED: CALLED TU TRANSITIONS TO DIAL TONE
        if (tu->caller){
            pbx->tunits[tu->caller]->state = TU_DIAL_TONE;;
            char* c = state_to_string(pbx->tunits[tu->caller]->state);
            dprintf(tu->caller, "%s\r\n", c);
            reset_tu(tu);
            reset_tu(pbx->tunits[tu->connected]);
        } else {
            debug("tu->caller false");
            V(&mutex);
            return -1;
        }
    } else if (tu->state == TU_DIAL_TONE || tu->state == TU_BUSY_SIGNAL || tu->state == TU_ERROR){
        // debug("test");
        tu->state = TU_ON_HOOK;
        char *c = state_to_string(tu->state);
        dprintf(tu->fd, "%s %d\r\n", c, tu->fd);
        reset_tu(tu);
    } else {
        char *c = state_to_string(tu->state);
        dprintf(tu->fd, "%s %d\r\n", c, tu->fd);
    }

    V(&mutex);
    return 0;
}

int tu_dial(TU *tu, int ext){
    // debug("dial tu->fd: %d, ext: %d", tu->fd, ext);
    P(&mutex);
    // CALLER IS tu->fd
    // CALLED is ext

    // check that extension refers to an actual extension


    if (tu->state == TU_DIAL_TONE){
        // CHECK STATE OF DIALED EXTENSION
        if (pbx->tunits[ext] == NULL){
        // debug("not a valid extension");
        tu->state = TU_ERROR;
        char* c = state_to_string(tu->state);
        dprintf(tu->fd, "%s\r\n", c);
        V(&mutex);
        return 0;
    }
        if (pbx->tunits[ext]->state == TU_ON_HOOK){
            // CALLER -> RING_BACK
            // CALLED -> RINGING
            // SET CALLED->CALLER, set CALLER->CALLED

            pbx->tunits[ext]->caller = tu->fd;
            tu->called = ext;

            tu->state = TU_RING_BACK;
            char* c = state_to_string(tu->state);
            dprintf(tu->fd, "%s\r\n", c);

            pbx->tunits[ext]->state = TU_RINGING;
            c = state_to_string(pbx->tunits[ext]->state);
            dprintf(ext, "%s\r\n", c);
        } else {
            tu->state = TU_BUSY_SIGNAL;
            char* c = state_to_string(tu->state);
            dprintf(tu->fd, "%s\r\n", c);
        }
    } else {
        if (tu->state == TU_CONNECTED){
            if (tu->connected){
                char* c = state_to_string(tu->state);
                dprintf(tu->fd, "%s %d\r\n", c, tu->connected);
            } else {
                debug("state is connected, but tu->connected = 0");
                return -1;
            }
        } else if (tu->state == TU_ON_HOOK) {
            char* c = state_to_string(tu->state);
            dprintf(tu->fd, "%s %d\r\n", c, tu->fd);
        } else {
            char* c = state_to_string(tu->state);
            dprintf(tu->fd, "%s\r\n", c);
        }
    }
    V(&mutex);
    return 0;
}

int tu_chat(TU *tu, char *msg){
    if (tu->state != TU_CONNECTED){
        if (tu->state == TU_ON_HOOK) {
            char* c = state_to_string(tu->state);
            dprintf(tu->fd, "%s %d\r\n", c, tu->fd);
        } else {
            char* c = state_to_string(tu->state);
            dprintf(tu->fd, "%s\r\n", c);
        }
        // debug("not connected");
        return -1;
    }
    if (tu->connected){
        dprintf(tu->connected, "CHAT%s\r\n", msg);
    } else {
        return -1;
    }
    return 0;
}