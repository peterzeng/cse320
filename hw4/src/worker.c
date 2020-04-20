#include <stdlib.h>

#include <unistd.h>

#include "debug.h"
#include "polya.h"


volatile sig_atomic_t got_sighup = 0;

void sigterm_handler(int sig){
    exit(EXIT_SUCCESS);
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

        if (kill(getpid(), SIGSTOP) < 0){
            debug("kill error\n");
        }

        size_t size_of_header = sizeof(struct problem);

        debug("size of header: %zu\n",size_of_header);
        // printf("size of header: %zu\n",size_of_header);

        struct problem *problem = malloc(size_of_header);

        if (read(STDIN_FILENO, problem, size_of_header) < 0){
            debug("read error\n");
        }

        debug("size of problem: %zu\n", problem->size);

        // Malloc enough space for the entire problem
        problem = realloc(problem, problem->size);

        read(STDIN_FILENO, problem->data, problem->size - size_of_header);

        fflush(STDIN_FILENO);


        // Get the problem type from problem
        short problem_type = problem->type;

        // Solve the problem
        struct result *result;
        if (((result = solvers[problem_type].solve(problem, &got_sighup)) == NULL) || got_sighup != 0){
            debug("solution failed somehwo\n");
            struct result *result = malloc(sizeof(struct result));
            result->size = sizeof(struct result);
            result->id = problem->id;
            result->failed = 1;
            write(STDOUT_FILENO, result, result->size);
        } else {
            debug("writing a solution\n");
            write(STDOUT_FILENO, result, result->size);
        }

        free(result);
        free(problem);


        if (got_sighup){
            got_sighup = 0;
            debug("Resetting sighup\n");
        }

    }

    // debug("size of header: %zu\n",size_of_header);
    // problem new_problem = read
    debug("reeeeeeeeeeeeeeee\n");
    // pid_t pid;

    // int olderrno = errno;
    // TO BE IMPLEMENTED
    return EXIT_FAILURE;
}
