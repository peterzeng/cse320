#include <stdlib.h>

#include <unistd.h>

#include "debug.h"
#include "polya.h"


volatile sig_atomic_t got_sighup = 0;
volatile sig_atomic_t got_sigterm = 0;


void sigterm_handler(int sig){
    got_sigterm = 1;
    // exit(EXIT_SUCCESS);
}

void sighup_handler(int sig){
    got_sighup = 1;
}
/*
 * worker
 * (See polya.h for specification.)
 */
int worker(void) {

    if (signal(SIGTERM, sigterm_handler) == SIG_ERR){
        debug("error in termination\n");
    }
    if (signal(SIGHUP, sighup_handler) == SIG_ERR){
        debug("error in interruption\n");
    }


    while(1){

        // debug("Test");
        if (kill(getpid(), SIGSTOP) < 0){
            debug("kill error\n");
        }

        if (got_sigterm){
            debug("Killing self");
            got_sigterm = 0;
            exit(EXIT_SUCCESS);
        }

        size_t size_of_header = sizeof(struct problem);

        // debug("size of header: %zu\n",size_of_header);
        // printf("size of header: %zu\n",size_of_header);

        struct problem *problem = malloc(size_of_header);

        // debug("problem pointer: %p", problem);

        if (read(STDIN_FILENO, problem, size_of_header) < 0){
            debug("read error\n");
        }

        // debug("size of problem: %zu\n", problem->size);

        // Malloc enough space for the entire problem
        problem = realloc(problem, problem->size);

        read(STDIN_FILENO, problem->data, problem->size - size_of_header);

        // fflush(STDIN_FILENO);


        // Get the problem type from problem
        short problem_type = problem->type;

        // Solve the problem
        struct result *result;
        if (((result = solvers[problem_type].solve(problem, &got_sighup)) == NULL) || got_sighup != 0){
            // debug("solution failed somehow\n");
            struct result *result = malloc(sizeof(struct result));
            result->size = sizeof(struct result);


            result->id = problem->id;
            result->failed = 1;
            debug("[%ld:Worker] Writing failed result to Master", (long)getpid());
            write(STDOUT_FILENO, result, result->size);
        } else {
            debug("[%ld:Worker] Writing result to Master", (long)getpid());
            // debug("result size in worker: %ld\n", result->size);

            write(STDOUT_FILENO, result, result->size);
        }

        free(result);
        free(problem);


    }

    // debug("size of header: %zu\n",size_of_header);
    // problem new_problem = read
    // pid_t pid;

    // int olderrno = errno;
    // TO BE IMPLEMENTED
    return EXIT_FAILURE;
}
