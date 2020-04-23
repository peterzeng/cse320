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

// Handle result/answers from worker in sigchild_handler

void sigchild_handler(int sig){
    int status;
    int child_pid;
    // local_index = 0;
    while((child_pid = (waitpid(-1, &status, WCONTINUED | WNOHANG | WUNTRACED))) > 0){

        // local copy of index in while loop
        int local_index;
        // int stop;
        // debug("Child Pid after waitpid called is: %ld", (long)child_pid);
        for (int i = 0; i < global_workers; i++){

            if (pids[i] == child_pid){
                local_index = i;
            }
        }
        // debug("global_workers %d\n", global_workers);

        // debug("check local_index %d\n", local_index);
        // debug("status %d\n", states[local_index]);

        if (WIFCONTINUED(status)){
            if (states[local_index] == WORKER_CONTINUED){
                debug("Attempting to read");
                sf_change_state(pids[local_index], states[local_index], WORKER_RUNNING);
                states[local_index] = WORKER_RUNNING;
            }

        } else if (WIFEXITED(status)){
            debug("child terminated normally");
            sf_change_state(pids[local_index], states[local_index], WORKER_EXITED);
            states[local_index] = WORKER_EXITED;

        } else if (WIFSTOPPED(status)){

            if (states[local_index] == WORKER_STARTED){
                // debug("Worker made idle\n");
                sf_change_state(pids[local_index], states[local_index], WORKER_IDLE);
                states[local_index] = WORKER_IDLE;

            } else if (states[local_index] == WORKER_RUNNING){
                debug("Worker Stopped\n");
                sf_change_state(pids[local_index], states[local_index], WORKER_STOPPED);
                states[local_index] = WORKER_STOPPED;

            } else if (states[local_index] == WORKER_STOPPED){
                debug("Resetting worker %d to IDLE\n", local_index);

                sf_change_state(pids[local_index], states[local_index], WORKER_IDLE);
                states[local_index] = WORKER_IDLE;
            }
            // sf_change_state(pids[local_index], states[local_index], WORKER_STOPPED);
            // states[local_index] = WORKER_STOPPED;

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

    struct problem *problem_assigned[workers];

    if (workers < 1){
        workers = 1;
    }

    global_workers = workers;

    // int states[workers];
    // int pids[workers];
    pids = malloc(sizeof(*pids) * workers);
    states = malloc(sizeof(*states) * workers);

    int master_pipes[workers][2];
    int worker_pipes[workers][2];

    // int fd_master[2];

    // int fd_worker[2];
    // FILE *in, *out;

    // if (pipe(fd_master) < 0){
    //     perror("Can't Create Pipe\n");
    //     exit(1);
    // }

    // if (pipe(fd_worker) < 0){
    //     perror("Can't Create Pipe\n");
    //     exit(1);
    // }

    if (signal(SIGCHLD, sigchild_handler) == SIG_ERR){
        debug("error in child\n");
    }
    if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR){
        debug("error in sigpipe\n");
    }


    sigset_t child_mask;
    // sigset_t child_mask, prev_mask;
    sigemptyset(&child_mask);
    sigaddset(&child_mask, SIGCHLD);


    // Initialization of master: Pipes, redirection, exec

    for (int i = 0; i < workers; i++){

        pipe(master_pipes[i]);
        pipe(worker_pipes[i]);

        if ((pid = fork()) == 0){
            // debug("In child, pid is %ld\n", (long)getpid());
            dup2(worker_pipes[i][1], STDOUT_FILENO);
            dup2(master_pipes[i][0], STDIN_FILENO);



            // sf_change_state(pids[i], 0, WORKER_STARTED);
            // states[i] = WORKER_STARTED;

            if ((execl("bin/polya_worker", "bin/polya_worker", NULL)) < 0){
                debug("error in exec\n");
            }

        } else {

            // sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);
            debug("Starting worker %d", i);
            pids[i] = pid;
            sf_change_state(pids[i], 0, WORKER_STARTED);
            states[i] = WORKER_STARTED;

            // sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            debug("Workers alive: %d",i+1);

            // debug("In parent, pid Wis %ld\n", (long)getpid());

            // SET PIDS/STATUSES OF CHILDREN IN ARRAYS OF LENGTH WORKERS


        }
    }


    while(1){
        if (got_sigpipe){
            got_sigpipe = 0;
        }

        // CHECK FOR IDLE WORKER

        for (int i = 0; i < workers; i++){
            // debug("states[i]: %d\n", states[i]);
            if (states[i] == WORKER_IDLE){

                struct problem *new_problem = get_problem_variant(workers, i);

                if (new_problem == NULL){
                    debug("new _prob = NULL");
                    int all_idle = 1;
                    for (int i = 0; i < workers; i++){
                        if (states[i] != WORKER_IDLE){
                            all_idle = 0;
                        }
                    }
                    if (all_idle){
                        debug("All problems solved, all idle");
                        for (int i = 0; i < workers; i++){

                            kill(pids[i], SIGCONT);

                            kill(pids[i], SIGTERM);
                            sf_change_state(pids[i], states[i], WORKER_EXITED);

                            // sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);
                            states[i] = WORKER_EXITED;
                            // sigprocmask(SIG_SETMASK, &prev_mask, NULL);

                        }
                        exit(EXIT_SUCCESS);
                    }
                    // NO NEW PROBLEMS
                    // debug("no new problems\n");

                    goto check_stop;
                }

                problem_assigned[i] = new_problem;

                debug("new_problem: %p\n", new_problem);

                debug("Sending problem to worker %d",i);

                // write(master_pipes[i][1])

                kill(pids[i], SIGCONT);
                sf_change_state(pids[i], states[i], WORKER_CONTINUED);
                states[i] = WORKER_CONTINUED;

                // sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);


                // sigprocmask(SIG_SETMASK, &prev_mask, NULL);

                write(master_pipes[i][1], new_problem, new_problem->size);
                // write(master_pipes[i][1], new_problem, new_problem->size - sizeof(struct problem));

                // NEED TO CHANGE IDLE TO RUNNING OF THIS WORKER


            }
        }

        // CHECK FOR STOPPED WORKER
        check_stop:for (int i = 0; i < workers; i++){
            // READ FROM STOPPED WORKER PIPE
            if (states[i] == WORKER_STOPPED){

                int check_solution;
                struct result *new_result = malloc(sizeof(struct result));
                read(worker_pipes[i][0], new_result, new_result->size);

                sf_recv_result(pids[i], new_result);

                debug("solution received, reading solution\n");

                // if (new_result->failed){
                //     sf_change_state(pids[i], states[i], WORKER_IDLE);
                //     states[i] = WORKER_IDLE;
                // } else {

                // }

                if ((check_solution = (post_result(new_result, problem_assigned[i]))) == 0){
                    free(new_result);
                    sf_change_state(pids[i], states[i], WORKER_IDLE);

                    // sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);

                    states[i] = WORKER_IDLE;

                    // sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                    // Solution works, we need to cancel workers working on this same problem
                    // Sends terminate
                    for (int j = 0; i < workers; i++){
                        if ((problem_assigned[j] == problem_assigned[i]) && (i != j)){
                            sf_cancel(pids[j]);
                            sf_change_state(pids[j], states[j], WORKER_STOPPED);

                            // sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);

                            states[j] = WORKER_IDLE;

                            // sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                            kill(pids[j], SIGHUP);
                        }
                    }

                } else {
                    free(new_result);
                    debug("solution failed");
                    sf_change_state(pids[i], states[i], WORKER_IDLE);

                    // sigprocmask(SIG_BLOCK, &child_mask, &prev_mask);

                    states[i] = WORKER_IDLE;

                    // sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                }
            }
        }


    }



    sf_end();

    return EXIT_FAILURE;

}
