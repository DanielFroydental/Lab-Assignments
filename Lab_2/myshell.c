#include "LineParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

void execute(cmdLine *pCmdLine) {
    if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
        fprintf(stderr, "execv() error");
        freeCmdLines(pCmdLine);
        exit(1);
    }
}

// Handles the "cd" command - changes the current working directory:
void handleCd(cmdLine *cmd) {
    if (cmd->argCount < 2)
        fprintf(stderr, "cd: missing argument\n");
    else if (chdir(cmd->arguments[1]) != 0)
        fprintf(stderr, "chdir() error");
}

// Handles the "alarm" command - sends a SIGCONT signal to a process:
void handleAlarm(cmdLine *cmd) {
    if (cmd->argCount < 2)
        fprintf(stderr, "alarm: missing argument\n");
    else {
        int pid = atoi(cmd->arguments[1]);
        if (kill(pid, SIGCONT) != 0)
            perror("kill() error");
    }
}

// Handles the "blast" command - sends a SIGKILL signal to a process:
void handleBlast(cmdLine *cmd) {
    if (cmd->argCount < 2)
        fprintf(stderr, "blast: missing argument\n");
    else {
        int pid = atoi(cmd->arguments[1]);
        if (kill(pid, SIGKILL) != 0)
            fprintf(stderr, "kill() error");
    }
}

int main(int argc, char **argv) {
    int debug_mode = 0;
    // Check for the debug flag (-d):
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
        debug_mode = 1;


    while(1) {
        // Display a prompt - the current working directory:
        char cwd[PATH_MAX];
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        }
        else {
            fprintf(stderr, "getcwd() error");
            continue;
        }

        // Read a line of input from stdin (no more than 2048 bytes):
        char input[2048];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            fprintf(stderr, "fgets() error");
            continue;
        }

        // Remove '\n' from the end of input:
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }
        
        // End the infinite loop of the shell if the command "quit" is entered in the shell, and exit the shell "normally":
        if (strcmp(input, "quit\n") == 0) {
            break;
        }

        // Parse the input using parseCmdLines():
        cmdLine *cmd = parseCmdLines(input);
        if (cmd == NULL)
            continue;

        // Handle the "cd" command:
        if (strcmp(cmd->arguments[0], "cd") == 0) {
            handleCd(cmd);
            freeCmdLines(cmd);
            continue;
        }

        // Handle the "alarm" command:
        if (strcmp(cmd->arguments[0], "alarm") == 0) { 
            handleAlarm(cmd);
            freeCmdLines(cmd);
            continue;
        }

        // Handle the "blast" command:
        if (strcmp(cmd->arguments[0], "blast") == 0) {
            handleBlast(cmd);
            freeCmdLines(cmd);
            continue;
        }

        pid_t pid = fork(); // Fork a child process

        // Check if the fork() was unsuccessful:
        if (pid == -1) {
            fprintf(stderr, "fork() error");
            freeCmdLines(cmd);
            exit(1);
        }
        else if (pid == 0) {
            // Redirect input if needed:
            if (cmd->inputRedirect) {
                int fd = open(cmd->inputRedirect, O_RDONLY);
                if (fd == -1) {
                    printf("open() error");
                    freeCmdLines(cmd);
                    exit(1);
                }
                close(STDIN_FILENO);
                dup(fd);
                close(fd);
            }
            // Redirect output if needed:
            if (cmd->outputRedirect) {
                int fd = open(cmd->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    fprintf(stderr, "open() error");
                    freeCmdLines(cmd);
                    exit(1);
                }
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
            }
            // Execute the command:
            execute(cmd);
        }
        else {
            if (debug_mode) {
                fprintf(stderr, "PID: %d\n", pid);
                fprintf(stderr, "Executing command: %s\n", cmd->arguments[0]);    
            }
            // Wait for the child process to finish if the command is blocking:
            if (cmd->blocking)
                waitpid(pid, NULL, 0);
        }
        // Release the cmd line:
        freeCmdLines(cmd);
    }
    return 0;
}
