/**
 * CS2106 AY22/23 Semester 2 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define ERROR_RETURN(err_msg) \
        fprintf(stderr, "%s\n", err_msg); \
        return; \

#define MAX_PROCESS 50

// PCB Status Codes
#define EXITED 1
#define RUNNING 2
#define TERMINATING 3
#define STOPPED 4

typedef struct Command {
    char *command;
    int argc;
    char **argv;
    bool background;
    char *stdinFile;
    char *stdoutFile;
    char *stderrFile;
} Command;

typedef struct PCBTable PCBTable;

PCBTable *g_PCBList[MAX_PROCESS] = { NULL };
uint g_PCBListIndex = 0;
pid_t g_fgProcess = -1;

/*******************************************************************************
 * My Helper Functions
 ******************************************************************************/

static pid_t mywaitpid(pid_t pid, int *status, int options) {
    pid_t ret = waitpid(pid, status, options);
    if (ret < 0) {
        fprintf(stderr, "waitpid failed\n");
    }
    return ret;
}

static void redirection(int oldfd, int newfd) {
    if (dup2(oldfd, newfd) == -1) {
        perror("Error redirecting output to file");
        exit(EXIT_FAILURE);
    }
}

static void update_pcb(PCBTable *pcb, int status) {
    if (WIFEXITED(status)) {
        pcb->status = EXITED;
        pcb->exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        pcb->status = EXITED;
        pcb->exitCode = WTERMSIG(status);
    } else if (WIFSTOPPED(status)) {
        pcb->status = STOPPED;
    }
}

static PCBTable *find_pcb(pid_t pid) {
    for (uint i = 0; i < g_PCBListIndex; i++) {
        if (g_PCBList[i]->pid == pid) {
            return g_PCBList[i];
        }
    }
    return NULL;
}

// If err == 1, then the conversion failed
static unsigned long mystrtoul(const char *nptr, bool *err) {
    char *endptr;
    unsigned long res = strtoul(nptr, &endptr, 10);

    if (endptr == nptr || *endptr != '\0' || errno == ERANGE) {
        *err = 1;
        return -1;
    }
    return res;
}

static void waitpid_update_pcb(PCBTable *pcb, int options) {
    int status;
    pid_t ret = mywaitpid(pcb->pid, &status, options);
    if (ret == pcb->pid) {
        update_pcb(pcb, status);
    }
}

/*******************************************************************************
 * Signal handler : ex4
 ******************************************************************************/

static void proc_update_status() {
    int status;
    pid_t ret;
    for (uint i = 0; i < g_PCBListIndex; i++) {
        if (g_PCBList[i]->status != EXITED) {
            ret = mywaitpid(g_PCBList[i]->pid, &status, WNOHANG);
            if (ret == g_PCBList[i]->pid) {
                update_pcb(g_PCBList[i], status);
            }
        }
    }
}

static void signal_handler(int signo) {
    // Use the signo to identy ctrl-Z or ctrl-C and print “[PID] stopped or print “[PID] interrupted accordingly.
    // Update the status of the process in the PCB table
    if (g_fgProcess == -1) {
        return;
    }
    proc_update_status();

    if (signo == SIGINT) {
        PCBTable *pcb = find_pcb(g_fgProcess);
        if (pcb->status == RUNNING) {
            printf("[%d] interrupted\n", pcb->pid);
            kill(pcb->pid, SIGINT);
            g_fgProcess = -1;
        }
    } else if (signo == SIGTSTP) {
        PCBTable *pcb = find_pcb(g_fgProcess);
        if (pcb->status == RUNNING) {
            printf("[%d] stopped\n", pcb->pid);
            kill(pcb->pid, SIGTSTP);
            g_fgProcess = -1;
        }
    }
    proc_update_status();
}


/*******************************************************************************
 * Built-in Commands
 ******************************************************************************/

static void command_info(Command *cmd) {
    if (cmd->argc != 1) {
        ERROR_RETURN("Wrong command");
    }

    bool err = 0;
    unsigned long option = mystrtoul(cmd->argv[0], &err);
    if (err) {
        ERROR_RETURN("Wrong command");
    }

    char *statusCaps[] = { "Exited", "Running", "Terminating", "Stopped" };
    char *statusLower[] = { "exited", "running", "terminating", "stopped" };

    proc_update_status();
    if (option == 0) {
        // print details of all processes
        for (uint i = 0; i < g_PCBListIndex; i++) {
            if (g_PCBList[i]->status == EXITED) {
                printf("[%d] %s %d\n", g_PCBList[i]->pid, statusCaps[g_PCBList[i]->status-1], g_PCBList[i]->exitCode);
            } else {
                printf("[%d] %s\n", g_PCBList[i]->pid, statusCaps[g_PCBList[i]->status-1]);
            }
        }
    } else if (option >= 1 && option <= 4) {
        uint count = 0;
        for (uint i = 0; i < g_PCBListIndex; i++) {
            if (g_PCBList[i]->status == (int) option) {
                count++;
            }
        }
        printf("Total %s process: %d\n", statusLower[option-1], count);
    } else {
        ERROR_RETURN("Wrong command");
    }
}

static void command_wait(Command *cmd) {
    if (cmd->argc != 1) {
        ERROR_RETURN("Wrong command");
    }

    bool err = 0;
    pid_t pid = mystrtoul(cmd->argv[0], &err);
    if (err) {
        ERROR_RETURN("Wrong command");
    }

    PCBTable *pcb = find_pcb(pid);
    if (pcb != NULL && pcb->status == RUNNING) {
        waitpid_update_pcb(pcb,  WUNTRACED);
    }
}


static void command_terminate(Command *cmd) {
    if (cmd->argc != 1) {
        ERROR_RETURN("Wrong command");
    }

    bool err = 0;
    pid_t pid = mystrtoul(cmd->argv[0], &err);
    if (err) {
        ERROR_RETURN("Wrong command");
    }

    PCBTable *pcb = find_pcb(pid);
    if (pcb != NULL && pcb->status == RUNNING) {
        kill(pid, SIGTERM);
        pcb->status = TERMINATING;
    }
}


static void command_fg(Command *cmd) {
    if (cmd->argc != 1) {
        ERROR_RETURN("Wrong command");
    }

    bool err = 0;
    pid_t pid = mystrtoul(cmd->argv[0], &err);
    if (err) {
        ERROR_RETURN("Wrong command");
    }

    PCBTable *pcb = find_pcb(pid);
    if (pcb != NULL && pcb->status == STOPPED) {
        printf("[%d] resumed\n", pid);
        g_fgProcess = pid;
        pcb->status = RUNNING;
        kill(pid, SIGCONT);
        waitpid_update_pcb(pcb,  WUNTRACED);
    }
}


/*******************************************************************************
 * Program Execution
 ******************************************************************************/

static void command_exec(Command *cmd) {
    // check if program exists and is executable : use access()
    if (access(cmd->command, R_OK | X_OK) != 0) {
        // file is not readable or not executable
        fprintf(stderr, "%s not found\n", cmd->command);
        return;
    }

    // fork a subprocess and execute the program
    pid_t pid = fork();
    if (pid < 0) {
        ERROR_RETURN("fork failed");
    } else if (pid == 0) {
        // CHILD PROCESS

        // redirect stdin, stdout, stderr if necessary
        if (cmd->stdinFile != NULL) {
            // check if the file exists
            int fd = open(cmd->stdinFile, O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "%s does not exist\n", cmd->stdinFile);
                exit(EXIT_FAILURE);
            }
            redirection(fd, STDIN_FILENO);
            close(fd);
        }
        if (cmd->stdoutFile != NULL) {
            int fd = open(cmd->stdoutFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            redirection(fd, STDOUT_FILENO);
            close(fd);
        }
        if (cmd->stderrFile != NULL) {
            int fd = open(cmd->stderrFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            redirection(fd, STDERR_FILENO);
            close(fd);
        }

        // decrement argv by 1 to include the command name which is required by execv()
        execv(cmd->command, cmd->argv-1);

        // The exec() functions only return if an error has occurred. The return value is -1, and errno is set to indicate the error.
        fprintf(stderr, "%s failed to executed.\n", cmd->command);
        exit(EXIT_FAILURE);
    } else {
        // PARENT PROCESS
        // register the process in process table
        PCBTable *pcb = malloc(sizeof(PCBTable));
        pcb->pid = pid;
        pcb->status = RUNNING;
        pcb->exitCode = -1;

        // insert into PCB List
        g_PCBList[g_PCBListIndex++] = pcb;

        if (cmd->background) {
            // If  child process need to execute in the background  (if & is present at the end )
            printf("Child [%d] in background\n", pid);
            waitpid_update_pcb(pcb, WNOHANG);
        } else {
            // else wait for the child process to exit
            g_fgProcess = pid;
            waitpid_update_pcb(pcb,  WUNTRACED);
            g_fgProcess = -1;
        }
    }
}



/*******************************************************************************
 * Command Processor
 ******************************************************************************/

static void command(Command *cmd) {
    // printf("Cmd: %s\nArgc: %d\nArgs: ", cmd->command, cmd->argc);
    // for (int i = 0; i < cmd->argc; i++) {
    //     printf("%s ", cmd->argv[i]);
    // }
    // // print cmd files
    // printf("\nStdin: %s, Stdout: %s, Stderr: %s", cmd->stdinFile, cmd->stdoutFile, cmd->stderrFile);
    // printf("\n================\n");

    if (strcmp(cmd->command, "info") == 0) {
        command_info(cmd);
    } else if (strcmp(cmd->command, "wait") == 0) {
        command_wait(cmd);
    } else if (strcmp(cmd->command, "terminate") == 0) {
        command_terminate(cmd);
    } else if (strcmp(cmd->command, "fg") == 0) {
        command_fg(cmd);
    } else {
        command_exec(cmd);
    }
}

/*******************************************************************************
 * High-level Procedure
 ******************************************************************************/

void my_init(void) {
    // ctrl+c
    signal(SIGINT, signal_handler);
    // ctrl+z
    signal(SIGTSTP, signal_handler);
}

void my_process_command(size_t num_tokens, char **tokens) {
    uint command_count = 1;
    for (uint i = 0; i < num_tokens - 1; i++) {
        if (strcmp(tokens[i], ";") == 0) {
            tokens[i] = NULL;
            command_count++;
        }
    }
    for (uint i = 0; i < command_count; i++) {
        int argc = -1;
        char **start = tokens;
        while (*tokens != NULL) {
            tokens++;
            argc++;
        }

        Command *cmd = malloc(sizeof(Command));
        cmd->command = start[0];
        cmd->argc = argc;
        cmd->argv = start+1;

        // check for & at the end and strip
        cmd->background = strcmp(cmd->argv[cmd->argc-1], "&") == 0;
        if (cmd->background) {
            cmd->argv[cmd->argc-1] = NULL;
            cmd->argc--;
        }

        cmd->stdinFile = NULL;
        cmd->stdoutFile = NULL;
        cmd->stderrFile = NULL;

        int decrement = 0;
        for (int i = 0; i < cmd->argc; i++) {
            if (strcmp(cmd->argv[i], ">") == 0) {
                decrement += 2;
                cmd->stdoutFile = cmd->argv[i+1];
            } else if (strcmp(cmd->argv[i], "<") == 0) {
                decrement += 2;
                cmd->stdinFile = cmd->argv[i+1];
            } else if (strcmp(cmd->argv[i], "2>") == 0) {
                decrement += 2;
                cmd->stderrFile = cmd->argv[i+1];
            }
        }
        cmd->argc -= decrement;
        cmd->argv[cmd->argc] = NULL;

        command(cmd);

        // Free the memory
        free(cmd);

        // skip the NULL element
        tokens++;
    }
}

void my_quit(void) {
    // Kill every process in the PCB that is either stopped or running
    proc_update_status();
    for (uint i = 0; i < g_PCBListIndex; i++) {
        if (g_PCBList[i]->status == RUNNING || g_PCBList[i]->status == STOPPED) {
            printf("Killing [%d]\n", g_PCBList[i]->pid);
            kill(g_PCBList[i]->pid, SIGKILL);
        }
        free(g_PCBList[i]);
    }

    printf("\nGoodbye\n");
}
