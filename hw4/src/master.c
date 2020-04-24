#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"
#include "polya.h"

volatile sig_atomic_t got_sigchild = 0;
volatile sig_atomic_t got_sigpipe = 0;

sig_atomic_t *pids;
sig_atomic_t *states;

int global_workers;

void sigchild_handler(int sig){
    int status;
    int child_pid;

    while((child_pid = (waitpid(-1, &status, WCONTINUED | WNOHANG | WUNTRACED))) > 0){
        int local_index;

        for (int i = 0; i < global_workers; i++){

            if (pids[i] == child_pid){
                local_index = i;
            }
        }

        if (WIFCONTINUED(status)){
            debug("Setting worker [%ld] to RUNNING", (long)child_pid);
            sf_change_state(pids[local_index], states[local_index], WORKER_RUNNING);
            states[local_index] = WORKER_RUNNING;

        } else if (WIFEXITED(status)){
            // debug("child terminated normally");
            sf_change_state(pids[local_index], states[local_index], WORKER_EXITED);
            states[local_index] = WORKER_EXITED;

        } else if (WIFSTOPPED(status)){

            if (states[local_index] == WORKER_STARTED){
                // debug("Worker made idle\n");
                sf_change_state(pids[local_index], states[local_index], WORKER_IDLE);
                states[local_index] = WORKER_IDLE;

            } else if (states[local_index] == WORKER_RUNNING){
                debug("Worker Stopped");
                sf_change_state(pids[local_index], states[local_index], WORKER_STOPPED);
                states[local_index] = WORKER_STOPPED;
            }
            // } else if (states[local_index] == WORKER_STOPPED){
            //     debug("Resetting worker %d to IDLE\n", local_index);

            //     sf_change_state(pids[local_index], states[local_index], WORKER_IDLE);
            //     states[local_index] = WORKER_IDLE;
            // }
        }

    got_sigchild = 1;
    // exit(EXIT_SUCCESS);
    }
}

void sigpipe_handler(int sig){
    got_sigpipe = 1;
}

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers) {

    sf_start();



    pid_t pid;

    // struct problem *problem_assigned[workers];
    struct problem *problem_assigned;

    if (workers < 1){
        workers = 1;
    }

    global_workers = workers;

    pids = malloc(sizeof(*pids) * workers);
    states = malloc(sizeof(*states) * workers);

    int master_pipes[workers][2];
    int worker_pipes[workers][2];

    if (signal(SIGCHLD, sigchild_handler) == SIG_ERR){
        debug("error in child\n");
    }
    if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR){
        debug("error in sigpipe\n");
    }

    sigset_t child_mask, prev_mask;
    sigemptyset(&child_mask);
    sigaddset(&child_mask, SIGCHLD);

    // Initialization of master: Pipes, redirection, exec
    for (int i = 0; i < workers; i++){

        pipe(master_pipes[i]);
        pipe(worker_pipes[i]);

        if ((pid = fork()) == 0){

            dup2(worker_pipes[i][1], STDOUT_FILENO);
            dup2(master_pipes[i][0], STDIN_FILENO);


            if ((execl("bin/polya_worker", "bin/polya_worker", NULL)) < 0){
                debug("error in exec\n");
            }

        } else {

            sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);
            debug("Starting worker %d", i);
            pids[i] = pid;
            sf_change_state(pids[i], 0, WORKER_STARTED);
            states[i] = WORKER_STARTED;

            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            debug("Workers alive: %d",i+1);

        }
    }
    for (int i = 0; i < workers; i++){
        if (states[i] == WORKER_STARTED){
            pause();
        }
    }

    while(1){
        if (got_sigpipe){
            got_sigpipe = 0;
        }

        // CHECK FOR IDLE WORKER
        for (int i = 0; i < workers; i++){
            if (states[i] == WORKER_IDLE){

                struct problem *new_problem = get_problem_variant(workers, i);

                if (new_problem == NULL){
                  int check_idle = 1;
                    // Check if all workers idle
                    for (int j = 0; j < workers; j++){
                        if (states[j] != WORKER_IDLE)
                            check_idle = 0;
                    }

                    if (check_idle){
                        for (int k = 0; k < workers; k++){
                            kill(pids[k],SIGTERM);

                            kill(pids[k], SIGCONT);
                            debug("Sending SIGCONT to worker %ld",(long)pids[k]);
                            if (states[k] == WORKER_RUNNING)
                                pause();
                            while(1){
                                // pause();
                                if (states[k] == WORKER_EXITED)
                                    break;
                            }
                        }
                        sf_end();
                        exit(EXIT_SUCCESS);
                    } else {
                        goto check_stop;
                    }

                }

                problem_assigned = new_problem;

                sf_send_problem(pids[i], new_problem);
                sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);
                sf_change_state(pids[i], states[i], WORKER_CONTINUED);
                states[i] = WORKER_CONTINUED;
                sigprocmask(SIG_SETMASK, &prev_mask, NULL);

                // write(master_pipes[i][1], new_problem, new_problem->size);
                kill(pids[i], SIGCONT);
                if (states[i] == WORKER_CONTINUED)
                    pause();
                debug("Sending SIGCONT to worker %d",i);
                debug("Writing problem to worker %d",i);
                // debug("size %ld", new_problem->size);
                write(master_pipes[i][1], new_problem, new_problem->size);
            }
        }

        // CHECK FOR STOPPED WORKER
        check_stop:for (int i = 0; i < workers; i++){
            if(states[i] == WORKER_STOPPED){
                // debug("stopped worker");
                int check_solution;
                struct result *new_result = malloc(sizeof(struct result));
                read(worker_pipes[i][0], new_result, sizeof(struct result));
                new_result = realloc(new_result, new_result->size);
                read(worker_pipes[i][0], new_result->data, new_result->size - sizeof(struct result));

                sf_recv_result(pids[i], new_result);

                if ((check_solution = (post_result(new_result, problem_assigned))) == 0){
                    // Solution works
                    free(new_result);
                    sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);
                    sf_change_state(pids[i], states[i], WORKER_IDLE);
                    states[i] = WORKER_IDLE;
                    sigprocmask(SIG_SETMASK, &prev_mask, NULL);

                    // Cancels other workers working on the same problem
                    for (int j = 0; j < workers; j++){
                        if (states[j] != WORKER_IDLE){
                            sf_cancel(pids[j]);
                            kill(pids[j], SIGHUP);
                            if (states[j] == WORKER_RUNNING)
                                pause();

                            // sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);
                            sf_change_state(pids[j], states[j], WORKER_IDLE);
                            states[j] = WORKER_IDLE;
                            // sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                        }
                    }
                } else {
                    // Solution failed, still need to IDLE current worker
                    free(new_result);
                    debug("Solution failed");
                    sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);
                    sf_change_state(pids[i], states[i], WORKER_IDLE);
                    states[i] = WORKER_IDLE;
                    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                }
            }

        } // End of check_stop
    } // End of while

    return EXIT_FAILURE;
}

