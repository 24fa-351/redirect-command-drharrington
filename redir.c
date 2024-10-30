#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Parse the command into an array of arguments
void parse_cmd(char *cmd, char **args) {
    char *token = strtok(cmd, " ");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

// Get the absolute path of the command from the PATH environment variable
char *get_absolute_path(char *cmd) {
    char *path_env = getenv("PATH");
    if (!path_env) {
        fprintf(stderr, "PATH environment variable not found.\n");
        return cmd;  // Return original cmd if PATH is not set
    }
    char *path_copy = strdup(path_env);
    char *path_token = strtok(path_copy, ":");
    static char abs_path[256];

    // Search for the command in each directory listed in PATH
    while (path_token != NULL) {
        snprintf(abs_path, sizeof(abs_path), "%s/%s", path_token, cmd);
        if (access(abs_path, X_OK) == 0) {
            free(path_copy);
            return abs_path;
        }
        path_token = strtok(NULL, ":");
    }
    free(path_copy);
    return cmd;  // Return original cmd if absolute path not found
}

int main(int argc, char *argv[]) {
    // Check for the correct number of arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: redir <inp> <cmd> <out>\n");
        exit(EXIT_FAILURE);
    }

    char *inp = argv[1];
    char *cmd = argv[2];
    char *out = argv[3];

    char *args[256];
    parse_cmd(cmd, args);

    args[0] = get_absolute_path(args[0]);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {  // Child process
        // Redirect input if necessary
        if (strcmp(inp, "-") != 0) {
            int fd_in = open(inp, O_RDONLY);
            if (fd_in < 0) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        // Redirect output if necessary
        if (strcmp(out, "-") != 0) {
            int fd_out = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        // Execute the command
        execv(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else {  // Parent process
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
