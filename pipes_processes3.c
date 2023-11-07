#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <grep_argument>\n", argv[0]);
        return 1;
    }

    int pipefd1[2]; // Pipe to connect cat and grep
    int pipefd2[2]; // Pipe to connect grep and sort

    if (pipe(pipefd1) == -1) {
        perror("pipe");
        return 1;
    }

    if (pipe(pipefd2) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        return 1;
    }

    if (pid1 == 0) { // cat process
        close(pipefd1[0]); // Close unused read end
        dup2(pipefd1[1], STDOUT_FILENO);
        close(pipefd1[1]);
        
        close(pipefd2[0]); // Close unused read end
        close(pipefd2[1]); // Close unused write end

        char *cat_args[] = {"cat", "scores", NULL};
        execvp(cat_args[0], cat_args);
        perror("execvp (cat)");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        return 1;
    }

    if (pid2 == 0) { // grep process
        close(pipefd1[1]); // Close write end of the first pipe
        dup2(pipefd1[0], STDIN_FILENO);
        close(pipefd1[0]);

        close(pipefd2[0]); // Close unused read end
        dup2(pipefd2[1], STDOUT_FILENO);
        close(pipefd2[1]);

        char *grep_args[] = {"grep", argv[1], NULL};
        execvp(grep_args[0], grep_args);
        perror("execvp (grep)");
        exit(EXIT_FAILURE);
    }

    pid_t pid3 = fork();
    if (pid3 == -1) {
        perror("fork");
        return 1;
    }

    if (pid3 == 0) { // sort process
        close(pipefd2[1]); // Close write end of the second pipe
        dup2(pipefd2[0], STDIN_FILENO);
        close(pipefd2[0]);

        close(pipefd1[0]); // Close unused read end
        close(pipefd1[1]); // Close unused write end

        char *sort_args[] = {"sort", NULL};
        execvp(sort_args[0], sort_args);
        perror("execvp (sort)");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(pipefd1[0]); // Close unused read end
    close(pipefd1[1]); // Close unused write end
    close(pipefd2[0]); // Close unused read end
    close(pipefd2[1]); // Close unused write end

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;
}
